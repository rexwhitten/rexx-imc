/* Utility to access the REXX/imc stack server     (C) Ian Collier 1992 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>

void ex1t();

struct {u_short af;char name[100];}  /* a mimic of struct sockaddr, but */
       sock={AF_UNIX};               /* with a longer name field. */
int socklen;                         /* The length of the socket name */
char *buff;                          /* An input buffer */
unsigned bufflen;                    /* The amount of memory allocated */
int s;                               /* The socket fd */

int main(argc,argv)
int argc;
char **argv;
{
   int fifo=1;                       /* fifo or lifo */
   int c;
   int x2d();
   int pop();
   static char *opts[]={"-fifo","-lifo","-string","-pop","-peek","-drop",
                        "-num","-print"};  /* All the allowed options */
   static int optnum=8;              /* The number of allowed options */
   int opt=0;                        /* position of an option in the above */
   char id;                          /* a command character */
   char *string;                     /* The parameter of -string flag */
   int len;
   static char out[10];              /* A buffer for sprintf */
   char *env=getenv("RXSTACK");      /* The socket's file name */
   if(!env)fputs("RX Stack not found\n",stderr),exit(1);
   if(!(buff=malloc(bufflen=256)))perror("Unable to allocate memory"),exit(1);
   strcpy(sock.name,env);
   socklen=sizeof(u_short)+strlen(env);
   for(c=1;c<argc;c++){              /* Find first "action" flag */
      for(opt=0;opt<optnum&&strcmp(opts[opt],argv[c]);opt++);
      if(opt==optnum)fprintf(stderr,"Invalid option: %s\n",argv[c]),exit(1);
      if(opt>1)break;
      if(opt==0)fifo=1;else fifo=0;  /* Deal with -fifo and -lifo and go on */
   }
   if(argc>++c+(opt==2))fputs("Too many options\n",stderr),exit(1);
   if(opt<2)opt=0;                   /* no "action"; stack from stdin */
   s=socket(AF_UNIX,SOCK_STREAM,0);  /* Try to connect... */
   if(s<0)perror("Error in socket call"),exit(1);
   if(connect(s,(struct sockaddr *)&sock,socklen)<0)perror("Error in connect call"),exit(1);
   id=fifo?'Q':'S';                  /* Queue if fifo, Stack if lifo. */
   switch(opt){                      /* Select the appropriate action */
      case 0:while(1){               /* Default: collect lines from stdin */
                len=0;
                while((c=getchar())!=EOF&&c!='\n'){
                   if(len>=bufflen&&!(buff=realloc(buff,bufflen<<=1)))
                      perror("Out of memory"),exit(1);
                   buff[len++]=c;
                }
                if(c==EOF&&!len)break;
                sprintf(out,"%c%06X\n",id,len);
                if(write(s,out,8)<8||write(s,buff,len)<len)
                   perror("Error writing data"),exit(1);
             }
             break;
      case 2:string=(c>=argc?"":argv[c]);/* Stack argument following -string */
             sprintf(out,"%c%06X\n",id,len=strlen(string)); 
             if(write(s,out,8)<8||write(s,string,len)<len)
                perror("Error writing data"),exit(1);
             break;
      case 3:pop('G',1);break;       /* -pop: use the "Get" command */
      case 4:pop('P',1);break;       /* -peek: use the "Peek" command */
      case 5:if(write(s,"D",1)<1)    /* -drop: write a "Drop" command */
                perror("Error writing data"),exit(1);
             break;
      case 6:if(write(s,"N",1)<1)    /* -num: write a "Num" command; get ans */
                perror("Error reading data"),exit(1);
             if(read(s,buff,7)<7)perror("Error reading data"),exit(1);
             len=x2d(buff);
             printf("%d\n",len);
             fflush(stdout);
             break;
      case 7:while(pop('P',0)!=0xffffff) /* -print: continually peek, then */
                if(write(s,"D",1)<1)     /* drop.                          */
		   perror("Error writing data"),exit(1);
   }               /* This way, if the output is being read by a program which
                      terminates, a SIGPIPE arrives and any unread data is
		      still on the stack. */
   if(opt<3||opt==5){ /* wait for the stack process to catch up */
      signal(15,ex1t); /* kludge to make a zero return code */
      sprintf(out,"K%06X\n\017",getpid()); /* kill me with SIGTERM */
      if(write(s,out,9)<9)perror("Error writing data"),exit(1);
      pause();
   }
   close(s);
   return 0;
}

void ex1t() {close(s);exit(0);} /* on SIGTERM, just exit with return code 0 */

int x2d(hex)       /* Convert six hex digits from "hex" into an integer */
char *hex;
{
   int i,len=0;
   char c;
   for(i=0;i<6;i++){
      len<<=4;
      if((c=hex[i])>='0'&&c<='9')len+=c-'0';
      else if(c>='A'&&c<='F')len+=c-'A'+10;
      else if(c>='a'&&c<='f')len+=c-'a'+10;
   }
   return len;
}

int pop(id,nullok)  /* Print the top value from the stack, using "id" as */
char id;            /* the command character.  If the stack is empty and */
int nullok;         /* nullok!=0 then print a newline, else just return. */
{                   /* The return value is the length given by the stack */
   int len;
   int rdlen;
   int i;
   if(write(s,&id,1)<1)    /* write the command character */
      perror("Error writing data"),exit(1);
   if(read(s,buff,7)<7)    /* Expect a length to be returned */
      perror("Error reading data"),exit(1);
   len=x2d(buff);
   if(len!=0xffffff){      /* Stack wasn't empty, so expect some data */
      if(len>bufflen&&!(buff=realloc(buff,(unsigned)len)))
         perror("Out of memory"),exit(1);
      rdlen=0;
      while(rdlen<len)     /* Read the data */
         if((i=read(s,buff+rdlen,len-rdlen))<1)
	    perror("Error reading data"),exit(1);
	 else rdlen+=i;
      fwrite(buff,1,len,stdout); /* Write the data to stdout */
      putchar('\n');
      fflush(stdout);      /* Make sure the application reads this */
   }
   else if(nullok){        /* Stack was empty; write an empty line */
      putchar('\n');
      fflush(stdout);
   }
   return len;
}
