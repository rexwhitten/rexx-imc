/* The stack server of REXX/imc                    (C) Ian Collier 1992 */

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<signal.h>
#include<fcntl.h>
#include<sys/time.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<string.h>

int io();                            /* function which does the work */
void term();                         /* cleanup function */

struct {u_short af;char name[100];}  /* a mimic of struct sockaddr, but */
       sock={AF_UNIX};               /* with a longer name field. */
int socklen;                         /* The length of the socket name */
fd_set sockfds;                      /* Collection of fds in use */
int maxfd;                           /* maximum fd in use */
int s;                               /* Socket fd */
char inbuf[20];                      /* for reading into */
char *sname;                         /* Socket file name */
static char uname[100];              /* for storing the socket file name */
char dname[100];                     /* for storing the socket directory */
char **stack;                        /* Pointer to the stack structure */
int first,last;                      /* First and last elements in circle */
int empty;                           /* Is stack empty when first==last? */
unsigned max;                        /* Number of stack elements allocated */

main(argc,argv)           /* Specify "-csh" and/or a filename */
int argc;
char **argv;
{
   int t;
   int csh=0;
   int parent=getppid();
   if(argc>1&&!strcmp(argv[1],"-csh"))argv++,argc--,csh++;
   if(argc>2)fputs("Usage: rxque [-csh] [socketname]\n",stderr),exit(1);
   umask(077);
   if(argc==2)sname=argv[1];  /* must be the supplied file name */
   else{                      /* make one up */
      sprintf(dname,"/tmp/rxsock%d",getuid()),
      sprintf(uname,"%s/rxstack.%d",dname,getpid());
      if(mkdir(dname,0700)&&errno!=EEXIST)
         perror("Cannot create socket directory"),
         exit(1);
      sname=uname;
   }
   unlink(sname);             /* just in case it was there already */
   strcpy(sock.name,sname);
   socklen=sizeof(u_short)+strlen(sname);
   s=socket(AF_UNIX,SOCK_STREAM,0);
   if(s<0)perror("Cannot create socket"),exit(1);
   if(s==2){ /* 2 is needed for error output */
      t=dup(s);
      if(t<0)perror("Cannot dup socket descriptor"),exit(1);
      close(s);
      s=t;
   }
   if(bind(s,(struct sockaddr *)&sock,socklen)<0)perror("Error in bind call"),exit(1);
   if(listen(s,1)<0)perror("Error in listen call"),exit(1);
   setpgid(0,getpid());       /* Prevent signals from keyboard */
   if((t=fork())<0)perror("fork"),exit(1);
   if(t){                     /* Parent: output data and exit */
      if(argc==2)printf("%d\n",t);
      else
         if(csh)printf("setenv RXSTACK %s;setenv RXSTACKPROC %d",sname,t);
         else printf("RXSTACK=%s RXSTACKPROC=%d",sname,t);
      fflush(stdout),
      exit(0);
   }
   /* Child: now in background, so start serving */
   if(!(stack=(char**)malloc((max=128)*sizeof(char*))))
      perror("malloc"),exit(1);
   if (s!=1) fclose(stdout);
   if (s!=0) fclose(stdin),
   freopen("/dev/tty","w",stderr); /* In case of error messages */
   signal(SIGTERM,term);           /* enable cleanup when killed */
   first=0;last=0;empty=1;
   FD_ZERO(&sockfds);
   maxfd=s;
   while(1)
      if(!io()){                   /* returns 0 if inactive for 5 minutes */
         if(kill(parent,0)&&errno==ESRCH)
            fputs("RX Stack: Exiting since parent no longer exists\n",stderr),
            fflush(stderr),
            term();
      }
}

int io() /* Wait for connections; return 0 if timed out, 1 otherwise */
{
   fd_set readfds, exceptfds; /* for select call */
   int nfds;
   int t;
   int rc;
   int len;
   int rdlen;
   int i;
   char c;
   char *mem;
   struct timeval timeout;
   timeout.tv_sec=300;timeout.tv_usec=0;  /* 5 minute timeout */
   readfds=sockfds;     /* for select(): */
   FD_SET(s,&readfds);  /* read fds: all fds plus the socket */
   FD_ZERO(&exceptfds); /* except fds: just the socket */
   FD_SET(s,&exceptfds);
   if(!(nfds=select(maxfd+1,&readfds,NULL,&exceptfds,&timeout))) return 0;
   if (nfds<0) return 1; /* Error (what to do?) */
   if(FD_ISSET(s,&readfds) || FD_ISSET(s,&exceptfds)){ /* The socket is ready */
      if((t=accept(s,(struct sockaddr *)&sock,&socklen))<0); /* Error (what to do?) */
      else {
         if (t>maxfd) maxfd=t;
         FD_SET(t,&sockfds);
      }
   }
   for (t=0; t<=maxfd; t++) { /* Process all fds which are available for read */
      if(FD_ISSET(t,&readfds) && t!=s){ /* Test each one in turn */
         rc=read(t,inbuf,1);            /* Get the command character */
         if(rc>0)switch(inbuf[0]){      /* and select appropriate action */
            case 'N':len=last-first;    /* Num: output last-first (mod max) */
               if(len<=0&&!empty)len+=max;
               sprintf(inbuf,"%06X\n",len);
               if(write(t,inbuf,7)<7)rc=-1;
               break;
            case 'S':                   /* Stack */
            case 'Q':                   /* Queue */
                  if(read(t,inbuf+1,7)<7){ /* Get the data length */
                  rc=-1;
                  break;
               }
               for(len=0,i=1;i<7;i++){       /* Interpret the hex value */
                  len<<=4;
                  if((c=inbuf[i])>='0'&&c<='9')len+=c-'0';
                  else if(c>='A'&&c<='F')len+=c-'A'+10;
                  else if(c>='a'&&c<='f')len+=c-'a'+10;
                  else rc=-1;
               }
               if(rc<0)break;
               if(first==last&&!empty){     /* Extend the full stack */
                  if(!(stack=(char**)realloc((char *)stack,(max<<=1)*sizeof(char *))))
                     perror("RX Stack server: Unable to allocate memory"),
                     exit(1);
                  memcpy((char *)(stack+first+max/2),(char *)(stack+first),(max/2-first)*sizeof(char*));
                  first+=max/2;
               }              /* next allocate memory for a stack entry */
               if(!(mem=malloc((unsigned)len+sizeof(int)))){
                  rc=-1;
                  perror("RX Stack server: Unable to stack data");
                  break;
               }
               rdlen=0;
               while(rdlen<len&&rc>0)       /* Read the data to be stacked */
                  if((i=read(t,mem+sizeof(int)+rdlen,len-rdlen))<1)rc=-1;
                  else rdlen+=i;
               if(rc>0) {
                  *(int *)mem=len;          /* Store its length */
                  empty=0;
                  if(inbuf[0]=='Q'){        /* Put its address in the array */
                     stack[last++]=mem;
                     if(last==max)last=0;
                  }
                  else{
                     if(--first<0)first+=max;
                     stack[first]=mem;
                  }
               }
               break;
            case 'G':                   /* Get */
            case 'P':if(empty){         /* Peek */
                  if(write(t,"FFFFFF\n",7)<7)rc=-1;
                  break;
               }
               sprintf(inbuf+1,"%06X\n",len=*(int *)stack[first]);
               if(write(t,inbuf+1,7)<7)rc=-1;  /* Write the data */
               else if(write(t,stack[first]+sizeof(int),len)<len)rc=-1;
               if(inbuf[0]=='P')break;           /* Continue if it was "Get" */
            case 'D':if(empty)break;    /* Drop */
               free(stack[first]);
               if(++first==max)first=0;
               if(first==last)empty=1;
            break;
            case 'K':                   /* Kill me */
                  if(read(t,inbuf+1,8)<8){ /* Get the pid and signal */
                  rc=-1;
                  break;
               }
               for(len=0,i=1;i<7;i++){       /* Interpret the hex value */
                  len<<=4;
                  if((c=inbuf[i])>='0'&&c<='9')len+=c-'0';
                  else if(c>='A'&&c<='F')len+=c-'A'+10;
                  else if(c>='a'&&c<='f')len+=c-'a'+10;
                  else rc=-1;
               }
               if(rc<0)break;
               kill(len,inbuf[8]);
               break;
            default:rc=-1;             /* Unrecognised */
         }
         if(rc<=0){ /* Error: close connection */
            close(t);
            FD_CLR(t,&sockfds); 
         }
      }
   }
   return 1;
}

void term(){
   int n;
   int l;
   char *p,*q;
   close(s);      /* Close and remove the socket */
   unlink(sname);
   if(!empty){    /* Print and free all stacked items */
      n=last-first;
      if(n<0)n+=max;
      fprintf(stderr,"RX Stack: %d item%s still stacked\n",n,n==1?"":"s");
      while(n-->0){
         fputc('>',stderr);
         q=p=stack[first++];
         if(first==max)first=0;
         l=*(int *)p;
         p+=sizeof(int);
         while(l--)fputc(*p>=' '&&*p<=0x7e?*p:'?',stderr),p++;
         fputc('\n',stderr);
         free(q);
      }
      fflush(stderr);
   }
   free((char *)stack);
   exit(0);
}
