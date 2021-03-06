#define _POSIX_C_SOURCE 199309L //very obssessed with c99 so have to make timespec usable
#include <stdio.h>  //for printf and primitive stuff
#include <stdlib.h>
#include <getopt.h>  //used for getopt long
#include <string.h>
#include <errno.h>
#include <time.h>  //for the gettime function
#include <pthread.h>  //used for pthread functions
#include <sched.h>   //used for yield

pthread_mutex_t lock;
int spinlock=0;
long long iterations=1;//have to be used by other functions;it might be big
//defualt is 1; otherwise floatpoing exception will be raised
long long ninepower=1000000000;
int opt_yield=0;
int mutexflag=0;
int spinflag=0;
int compareflag=0;
void savetime(char * error, int errornumber){
  fprintf(stderr, "%s: %s",error,strerror(errornumber));
  exit(1);
}
void add(long long *pointer, long long value) {
  long long sum = *pointer + value;
  if (opt_yield)
    sched_yield();
  *pointer = sum;
}

void* addfunc(void * counter){ //return type void * as specified by pthread_create
  counter= (long long *)counter; //now we cast it back
  for (int i=0;i<iterations;i++){
    if(mutexflag){
      pthread_mutex_lock(&lock);
      add(counter,1);
      pthread_mutex_unlock(&lock);
    }
    else if(spinflag){
      while(__sync_lock_test_and_set(&spinlock,1)); //seems like no return val
      add(counter,1);
      __sync_lock_release(&spinlock);
    }
    else if(compareflag){
      long long oldvalue,sum;
      do{ oldvalue=*(long long*)counter;
	sum=oldvalue+1;
	if (opt_yield)
	  sched_yield();
      }while(__sync_val_compare_and_swap((long long *)counter,oldvalue,sum)!=oldvalue);
    }
    else{add(counter,1);}
  }
  for (int i=0;i<iterations;i++){
    if(mutexflag){
      pthread_mutex_lock(&lock);
      add(counter,-1);
      pthread_mutex_unlock(&lock);
    }
    else if(spinflag){
      while(__sync_lock_test_and_set(&spinlock,1)); //seems like no return val
      add(counter,-1);
      __sync_lock_release(&spinlock);
    }
    else if(compareflag){
       long long oldvalue,sum;
      do{oldvalue=*(long long*)counter;
	sum=oldvalue-1;
	if (opt_yield)
	  sched_yield();
      }while(__sync_val_compare_and_swap((long long *)counter,oldvalue,sum)!=oldvalue);
    }
    else{add(counter,-1);}
  }
  return NULL; //just to get rid of the warning
}

int main(int argc, char **argv)
{
  long long counter=0;
  int threads=1;         //default is 1
  
  //int choice=0;
  static struct option long_options[]=
    {
      {"threads",required_argument, 0, 't'},
      {"iterations",required_argument, 0, 'i'},
      {"yield",no_argument, 0,'y'},
      {"sync",required_argument, 0, 's'},
      {0,0,0,0}
    };
  int ret;
  while((ret= getopt_long(argc,argv, "t:i:ys", long_options, NULL))!=-1){
    switch(ret){
    case 't':
      threads=atoi(optarg);
      break;
    case 'i':
      iterations=atoi(optarg);
      break;
    case 'y':
      opt_yield=1;
      break;
    case 's':
      if((optarg[0]=='m')&&(strlen(optarg)==1))
	{mutexflag=1;}
      else if ((optarg[0]=='s')&&(strlen(optarg)==1))
	{spinflag=1;}
      else if ((optarg[0]=='c')&&(strlen(optarg)==1))
	{compareflag=1;}
      else{
	fprintf(stderr, "usage not correct");
	exit(1);}
      break;
    default:
      fprintf(stderr, "usage not correct");
      exit(1);
    }//end of switch
  }//end of while
  
  struct timespec start;
  int test;
  test=clock_gettime(CLOCK_MONOTONIC, &start);
  if(test<0){savetime("gettime error",errno);}

  pthread_t *threadarray= malloc(threads * sizeof(pthread_t));
  for(int i=0;i<threads;i++){
    if(pthread_create(&threadarray[i],NULL,addfunc,&counter)!=0) //counter is the only thing we need to pass down
      {savetime("pthread create error",errno);} 
  }
  
  for(int i=0;i<threads;i++){
    if(pthread_join(threadarray[i],NULL)!=0) 
      {savetime("pthread join error",errno);} 
  }
  
  struct timespec end;
  test=clock_gettime(CLOCK_MONOTONIC, &end);
  if(test<0){savetime("gettime error",errno);}
  free(threadarray);//put it here in case it affects the clock?
  long long innano= (end.tv_sec-start.tv_sec)*ninepower+ (end.tv_nsec-start.tv_nsec); //add bracket just for safety
  long long operations= threads * iterations *2;
  long long average= innano/operations;
  printf("add%s-%s,%d,%lli,%lli,%lli,%lli,%lli\n",opt_yield==0? "":"-yield",mutexflag==1? "m" : spinflag==1? "s" : compareflag==1? "c" : "none",threads,iterations,operations,innano,average,counter);
  exit(0);//FLAG THIS OUTPUT NEEDS TO BE MODIFIED
}
