#define _POSIX_C_SOURCE 199309L //very obssessed with c99 so have to make timespec usable
#include <stdio.h>  //for printf and primitive stuff
#include <stdlib.h>
#include <getopt.h>  //used for getopt long
#include <string.h>
#include <errno.h>
#include <time.h>  //for the gettime function
#include <pthread.h>  //used for pthread functions
#include <sched.h>   //used for yield
#include "SortedList.h"
#include <signal.h> //used to detect segmentation fault
SortedList_t * listpointers;
pthread_mutex_t *mutexpointers;
int * spinpointers;
int listsize=1;
pthread_mutex_t lock;
long long totallock=0;
int mutexflag=0;
int spinflag=0;
int spinvalue=0;
long long ninepower=1000000000;
int threads = 1;
long long iterations = 1;
int opt_yield = 0;
SortedList_t *listheader;
SortedListElement_t * listmember;
void savetime(char * error, int errornumber){
  fprintf(stderr, "%s: %s",error,strerror(errornumber));
  exit(1);
}
void handler(int signum)
{
  if(signum==SIGSEGV){
    fprintf(stderr,"segmentation falut: %s", strerror(errno));
    exit(2);
  }
}

void * addfunc (void* startpos){
  SortedListElement_t *start=(SortedListElement_t *)startpos;int startposition=*(int *)startpos;
  struct timespec lockstart;
  struct timespec lockend;
  int length=0;
  for(int i=0;i<iterations;i++){
    if(mutexflag){
      if(clock_gettime(CLOCK_MONOTONIC,&lockstart)==-1){savetime("gettime error:",errno);}
      pthread_mutex_lock(&mutexpointers[(startposition+i)%listsize]);
      if(clock_gettime(CLOCK_MONOTONIC,&lockend)==-1){savetime("gettime error:",errno);}
      totallock+=ninepower*(lockend.tv_sec-lockstart.tv_sec)+(lockend.tv_nsec-lockstart.tv_nsec);
      SortedList_insert(&listpointers[(startposition+i)%listsize],start+i);
      pthread_mutex_unlock(&mutexpointers[(startposition+i)%listsize]);
    }
    else if(spinflag){
      if(clock_gettime(CLOCK_MONOTONIC,&lockstart)==-1){savetime("gettime error:",errno);}
      while(__sync_lock_test_and_set(&spinpointers[(startposition+i)%listsize],1));
      if(clock_gettime(CLOCK_MONOTONIC,&lockend)==-1){savetime("gettime error:",errno);}
      totallock+=ninepower*(lockend.tv_sec-lockstart.tv_sec)+(lockend.tv_nsec-lockstart.tv_nsec);
      SortedList_insert(&listpointers[(startposition+i)%listsize],start+i);
      __sync_lock_release(&spinpointers[(startposition+i)%listsize]);
    }
    else{SortedList_insert(&listpointers[(startposition+i)%listsize],start+i);}
  }
  //block for checking the length
  
  if(mutexflag){
    for(int j=0;j<listsize;j++){ //this needs a back bracket    
      if(clock_gettime(CLOCK_MONOTONIC,&lockstart)==-1){savetime("gettime error:",errno);}
      pthread_mutex_lock(&mutexpointers[j]);
      if(clock_gettime(CLOCK_MONOTONIC,&lockend)==-1){savetime("gettime error:",errno);}
      totallock+=ninepower*(lockend.tv_sec-lockstart.tv_sec)+(lockend.tv_nsec-lockstart.tv_nsec);
      int temp=SortedList_length(&listpointers[j]);
      if(temp==-1){fprintf(stderr,"corruption");exit(2);}
      else{length+=temp;}
      pthread_mutex_unlock(&mutexpointers[j]);
    }
  } //bracket for the loop
    else if(spinflag){
      for(int j=0;j<listsize;j++){ //this needs a back bracket  
      if(clock_gettime(CLOCK_MONOTONIC,&lockstart)==-1){savetime("gettime error:",errno);}
      while(__sync_lock_test_and_set(&spinpointers[j],1));
      if(clock_gettime(CLOCK_MONOTONIC,&lockend)==-1){savetime("gettime error:",errno);}
      totallock+=ninepower*(lockend.tv_sec-lockstart.tv_sec)+(lockend.tv_nsec-lockstart.tv_nsec);
      int temp=SortedList_length(&listpointers[j]);
      if(temp==-1){fprintf(stderr,"corruption");exit(2);}
      else{length+=temp;}
      __sync_lock_release(&spinpointers[j]);
    }
    }
    else{
      for(int j=0;j<listsize;j++){ //this needs a back bracket  
	int temp=SortedList_length(&listpointers[j]);
	if(temp==-1){fprintf(stderr,"corruption");exit(2);}
	else{length+=temp;}
      }
    }
    //end of for loop
    //block for lookup and delete
    for(int i=0; i<iterations;i++){
    if(mutexflag){
      if(clock_gettime(CLOCK_MONOTONIC,&lockstart)==-1){savetime("gettime error:",errno);}
      pthread_mutex_lock(&mutexpointers[(startposition+i)%listsize]);
      if(clock_gettime(CLOCK_MONOTONIC,&lockend)==-1){savetime("gettime error:",errno);}
      totallock+=ninepower*(lockend.tv_sec-lockstart.tv_sec)+(lockend.tv_nsec-lockstart.tv_nsec);
      SortedListElement_t *mark=SortedList_lookup(&listpointers[(startposition+i)%listsize],start[i].key);
     if(mark==NULL){
      fprintf(stderr,"lookup corruption");exit(2);
     }
     if(SortedList_delete(mark)!=0){
      fprintf(stderr,"delete corruption");exit(2);
     }
      pthread_mutex_unlock(&mutexpointers[(startposition+i)%listsize]);
    }
    else if(spinflag){
      if(clock_gettime(CLOCK_MONOTONIC,&lockstart)==-1){savetime("gettime error:",errno);}
      while(__sync_lock_test_and_set(&spinpointers[(startposition+i)%listsize],1));
      if(clock_gettime(CLOCK_MONOTONIC,&lockend)==-1){savetime("gettime error:",errno);}
      totallock+=ninepower*(lockend.tv_sec-lockstart.tv_sec)+(lockend.tv_nsec-lockstart.tv_nsec);
      SortedListElement_t *mark=SortedList_lookup(&listpointers[(startposition+i)%listsize],start[i].key);
     if(mark==NULL){
      fprintf(stderr,"lookup corruption");exit(2);
     }
     if(SortedList_delete(mark)!=0){
      fprintf(stderr,"delete corruption");exit(2);
     }
      __sync_lock_release(&spinpointers[(startposition+i)%listsize]);
    }
    else{
     SortedListElement_t *mark=SortedList_lookup(&listpointers[(startposition+i)%listsize],start[i].key);
     if(mark==NULL){
      fprintf(stderr,"lookup corruption");exit(2);
     }
     if(SortedList_delete(mark)!=0){
      fprintf(stderr,"delete corruption");exit(2);
     } 
    }
    
  }
  return NULL;

}

int main(int argc, char **argv)
{
  static struct option long_options[]=
    {
      {"threads",required_argument, 0, 't'},
      {"iterations",required_argument, 0, 'i'},
      {"yield",required_argument, 0,'y'},
      {"sync",required_argument,0,'s'},
      {"lists",required_argument,0,'l'},
      {0,0,0,0}
    };
  int ret;
  while((ret= getopt_long(argc,argv, "t:i:y:s:l:", long_options, NULL))!=-1){
    switch(ret){
    case 't':
      threads=atoi(optarg);
      break;
    case 'l':
      listsize=atoi(optarg);
      break;
    case 'i':
      iterations=atoi(optarg);
      break;
    case 'y':
      for(size_t i=0;i<strlen(optarg);i++){
	if(optarg[i]=='i'){
	  opt_yield |= INSERT_YIELD;}
	else if(optarg[i]=='d'){
	    opt_yield |= DELETE_YIELD;}
	else if(optarg[i]=='l'){
	  opt_yield |= LOOKUP_YIELD;}
	else{fprintf(stderr, "incorrect usage");
	  exit(1);
	}
      }//end of string loop for
      //to be filled later
      break;
    case 's':
      if((optarg[0]=='m')&&strlen(optarg)==1)
	{mutexflag=1;}
      else if((optarg[0]=='s')&&strlen(optarg)==1)
	{spinflag=1;}
      else{fprintf(stderr, "incorrect usage");
	  exit(1);
	}
      //to be filled later
      break;
    default:
      fprintf(stderr,"unknown option");
      exit(1);
    }//end switch
  }//end while loop
  listpointers=malloc(listsize*sizeof(SortedList_t));
  for(int i=0;i<listsize;i++){
    listpointers[i].key=NULL;listpointers[i].next=&listpointers[i];
    listpointers[i].prev= &listpointers[i];
  } //assigning some random values for initialization
  //get locks prepared
  if(mutexflag){
    mutexpointers=malloc(listsize * sizeof(pthread_mutex_t));
    for(int i=0;i<listsize;i++){
      pthread_mutex_init(&mutexpointers[i],NULL);
    }
  }
  else if(spinflag){ //remember to free all the malloc
    spinpointers=malloc(listsize*sizeof(int));
    for(int i=0;i<listsize;i++){
      spinpointers[i]=0;
    }
  }
  if(signal(SIGSEGV,handler)==SIG_ERR){
    savetime("signal not sent:",errno);}
  listheader=malloc(sizeof(SortedList_t));
  listheader->key=NULL;
  listheader->prev=listheader;
  listheader->next=listheader;
  long long number= threads * iterations;
  listmember=malloc(number*sizeof(SortedListElement_t));
  srand(time(NULL));//randomize seed https://stackoverflow.com/questions/16569239/how-to-use-function-srand-with-time-h
  for(int i=0;i<number;i++){ 
     char *keystring = malloc((4+1) * sizeof(char)); //choose 3 to be the length of our key. I am assuming that freeing the whole list will free each node
     for(int k=0;k<4;k++){
       keystring[k] = 'a' + (char)rand()%26;}
     keystring[4]='\0';
     listmember[i].key = (const char *) keystring;
  }
  pthread_t * threadarray=malloc(threads*sizeof(pthread_t));
  struct timespec start;
  if (clock_gettime(CLOCK_MONOTONIC, &start)<0) { 
    savetime("gettime error:",errno);}
  
  for(int i=0;i<threads;i++){
    if(pthread_create(&threadarray[i],NULL,addfunc,listmember+i*iterations)!=0) 
      {savetime("pthread create error",errno);}
  }
  
  for(int i=0;i<threads;i++){
    if(pthread_join(threadarray[i],NULL)!=0)
      {savetime("pthread join error",errno);}
  }
  

  struct timespec end;
  if (clock_gettime(CLOCK_MONOTONIC, &end)<0) { 
    savetime("gettime error:",errno);}
  
  if(SortedList_length(listheader)!=0){
    fprintf(stderr, "list length is not 0,probs error\n");
    exit(2);
  }
  free(threadarray);
  
  free(listmember);
  
  int sum=0;
  for(int i=0;i<listsize;i++){sum+=SortedList_length(&listpointers[i]);}
  if(sum!=0){fprintf(stderr,"multilist length check fail");exit(2);}
  //free block
  if(mutexflag){free(mutexpointers);}
  else if(spinflag){free(spinpointers);}
  free(listpointers);
  long long innano= (end.tv_sec-start.tv_sec)*ninepower+ (end.tv_nsec-start.tv_nsec); //add bracket just for safety
  long long operations= threads * iterations *3;
  long long average= innano/operations;
  long long averagelock=totallock/operations;
  char *output;
  switch(opt_yield){
  case 0:
    output="none";
    break;
  case 0x01:
    output="i";
    break;
  case 0x02: output="d";break;
  case 0x04: output="l";break;
  case 0x03: output="id";break;
  case 0x05: output="il";break;
  case 0x06: output="dl";break;
  case 0x07: output="idl";break;
  default: output="switch case not correct" ;
  }
  printf("list-%s-%s,%d,%lli,%d,%lli,%lli,%lli,%lli\n",output,mutexflag==1?"m":spinflag==1?"s":"none",threads,iterations,listsize,operations,innano,average,averagelock);
  

  exit(0);
} //end main

