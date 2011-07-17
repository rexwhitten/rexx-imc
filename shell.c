/* A Command Shell for REXX/imc                 (C) Ian Collier 1992 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<setjmp.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/param.h>
#include"const.h"
#include"functions.h"
#include<string.h>

typedef struct _hashitem {  /* An item in the hash table of path names */
   struct _hashitem *next;  /* The next item in the bucket */
   int hits;                /* Number of times this has been found */
   int expense;             /* Position within $PATH */
   int dot;                 /* Whether dot occurred in the path before this */
   int data;                /* Offset from end of header to data */
} hashitem;

static unsigned hashfn();       /* the hash function to make int from string */
static hashitem **hashcreate(); /* Create a hash table */
static void *search();          /* search in the hash table */
static char *locate();          /* find out the path for a command */
static void hashcmd();          /* Execute the "hash" builtin command */
static void hashdel();          /* Delete an element from the hash table */

char **arguments=0; /* An array to hold the argument list */
unsigned argnum=0;  /* The number of elements allocated so far */
hashitem **hashtable;
int hashbuckets;

int shell(command) /* Execute a UNIX command.  The command must be writable */
char *command;     /* and zero-terminated */
{
   int argc=0;
   char quote=0;
   char c;
   int i,j;
   int pid;
   char *exec;

   if(!arguments)                   /* Allocate some initial memory */
      arguments=(char**)allocm((argnum=20)*sizeof(char*)),
      hashtable=hashcreate(hashbuckets=251);
   while(command[0]==' ')command++; /* Ignore leading spaces */
   arguments[argc++]=command;       /* Store the start of arg[0] */
   for(i=j=0;c=command[j];j++){     /* Start tokenising... */
      if(c==quote){quote=0;continue;}
      if(quote){command[i++]=c;continue;}
      if(c=='\''||c=='\"'){quote=c;continue;}
      if(c==' '){
         command[i++]=0;
	 while(command[++j]==' ');
	 j--;
	 if(argc+1>=argnum){
	    arguments=(char**)realloc((char*)arguments,
	       sizeof(char*)*(argnum+=10));
	    if(!arguments)die(Emem);
	 }
	 arguments[argc++]=command+i;
	 continue;
      }
      command[i++]=c;
   }
   command[i++]=0;                 /* 0-terminate the last argument */
   if(!arguments[argc-1][0])argc--;/* In case there were trailing spaces */
   if(!argc)return 0;              /* Null string: just return */
   arguments[argc++]=0;            /* Add the terminating NULL */
   if(!strcmp(arguments[0],"hash"))/* "hash" is built in */
      return hashcmd(arguments),0;
   exec=locate(arguments[0]);      /* Locate the command */
   if(!(pid=vfork())){
      execv(exec,arguments);       /* Execute command */
      if(errno==ENOENT)            /* did not exist */
         fprintf(stderr,"%s: Command not found.\n",arguments[0]);
      else perror(exec);           /* some other error */
      _exit(-3);
   }
   if(pid==-1){
      perror("vfork");
      return -3;
   }
   i=0;
   waitpid(pid,&i,0);             /* Wait for command */
   return (int)(char)(i/256);
}

static unsigned hashfn(string,buckets)  /* A hash function */
char *string;         /* the string to hash */
int buckets;          /* the number of hash buckets */
{
   register unsigned i=0;
   while(*string)i+=(i<<3)+*string++;
/* return (((i*40503L)&65535)*buckets)/65536;*/ /* multiplicative hashing: when
                                                 buckets is a power of 2 */
   return i%buckets;        /* division method: when buckets is a prime such
                            that 16^k=a(mod buckets) for small k and a */
}

static hashitem **hashcreate(buckets)
                          /* Create hash table as array of null pointers */
int buckets;              /* Number of buckets in hash table */
{
   hashitem **table=(hashitem**)allocm(buckets*sizeof(char *));
   int i;
   for(i=0;i<buckets;table[i++]=0);
   return table;
}

static void *search(name,exist) /* Search for a name in the hash table    */
char *name;                     /* if exist=1, the result is a pointer to */
int *exist;                     /* the item; if exist=0 the result is a   */
                                /* "next" field where the new item would  */
{                               /* be inserted                            */
   int h=hashfn(name,hashbuckets);
   hashitem **i=&hashtable[h];
   hashitem *j;
   int c;
   if(!(j=*i)) return *exist=0,(void *)i; /* No elements in this bucket */
   while(c=strcmp(name,(char *)(j+1))){   /* stop when correct element found */
      if(c<0) return *exist=0,(void *)i;  /* gone too far down the list */
      i=&(j->next);
      if(!(j=*i)) return *exist=0,(void *)i; /* no next element in list */
   }
   return *exist=1,(void *)j;
}

static char *locate(name)  /* Locate the executable file "name" */
char *name;
{
   char *path=getenv("PATH");
   void *hash;
   int exist;
   int dot=0;
   int i;
   int dirs=0;
   hashitem *new,**old;
   char *ans;
   static char test[MAXPATHLEN+1];
   if(!strchr(name,'/')){             /* only search if the name has no '/'s */
      hash=search(name,&exist);       /* first search the hash table */
      if(!exist&&path) while(path[0]){/* then search the path */
         dirs++;
         for(i=0;(test[i]=path[0])&&path++[0]!=':';i++); /* Copy next dir */
	 if(i==1&&test[0]=='.'){dot=1;continue;} /* Test for "." */
	 test[i]='/';
	 strcpy(test+i+1,name);                  /* add slash and name */
	 if(!access(test,X_OK)){                 /* if it is executable... */
	    new=(hashitem *)                     /* make a new hash item */
	       allocm(sizeof(hashitem)+strlen(name)+strlen(test)+2);
	    old=(hashitem **)hash; /* this points to the previous link field */
            new->next=*old;
	    *old=new;
	    new->dot=dot;
	    new->hits=0;
	    new->expense=dirs;
	    new->data=strlen(name)+1;
	    strcpy((char *)(new+1),name);
	    strcpy((char *)(new+1)+new->data,test);
	    exist=1;
	    hash=(void*)new;
	    break;
	 }
      }
      if(exist){ /* Now, if the hash item was found or newly created, use it */
         new=(hashitem *)hash;
	 new->hits++;
	 ans=(char *)(new+1)+new->data;
	 if(new->dot&&!access(name,X_OK)) /* If "." came in the path before */
	    return name;                  /* the named directory, check "." */
	 return ans;                      /* first, then return the stored  */
      }                                   /* name.                          */
   }
   return name; /* if the name contains '/' or wasn't found in the path,
                   return it unchanged. */
}

static void hashdel(name)  /* delete name from hash table, if present */
char *name;
{
   int h=hashfn(name,hashbuckets);
   hashitem **i=&hashtable[h];
   hashitem *j;
   int c;
   if(!(j=*i)) return;                   /* No items in this bucket */
   while(c=strcmp(name,(char *)(j+1))){  /* search for the given name */
      if(c<0) return;
      i=&(j->next);
      if(!(j=*i)) return;
   }
   *i=j->next;                        /* link the next item to the previous  */
   free(j);                           /* so deleting this one from the chain */
}

void hashclear()          /* Clear the hash table (eg when PATH changes) */
{
   int j;
   hashitem *h,*k;
   for(j=0;j<hashbuckets;j++)
      for(h=hashtable[j],hashtable[j]=0;h;h=k){
         k=h->next;        /* find the address of the next item before */
         free((char *)h);  /* freeing this one (obviously).            */
      }
}

static void hashcmd(args) /* Implement the "hash" builtin command */
char *args[];
{
   int i;
   int j;
   int hits;
   hashitem *h;
   if(args[i=1])          /* some arguments exist */
      while(args[i]){
         if(!strcmp(args[i],"-r")) hashclear();  /* Clear table */
	 else locate(args[i]);      /* add argument to table */
	 i++;
      }
   else{                  /* no arguments: print table */
      hits=0;
      for(i=0;i<hashbuckets;i++)
         for(j=0,h=hashtable[i];h;h=h->next){
	    if(!hits++) puts(" hits    cost    command");
	    putchar(j++?'+':' ');
            printf("%-7d %-7d %s\n",h->hits,h->expense,(char*)(h+1)+h->data);
	 }
      if(!hits) puts("No commands in hash table.");
   }
}
