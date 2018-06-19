#include <stdio.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <mraa.h>
#include <mraa/aio.h>
#include <signal.h>
#include <math.h>  //needed for temperature convertion
//sig_atomic_t volatile run_flag=1;
mraa_aio_context temperature;
mraa_gpio_context button;
int manualstop=1;
int period=1;
char scale='C';
FILE *filepointer=NULL;
int goodtemperature=0;
int checklog=0;
void printtime(){
  time_t current;
  char formatstring[15];
  struct tm* timestruct;
  
  time(&current);
  timestruct=localtime(&current);
  strftime(formatstring,15, "%H:%M:%S",timestruct);
  fprintf(stdout,"%s ",formatstring);
  if(checklog){
    fprintf(filepointer,"%s ",formatstring);
    fflush(filepointer);
  }
}


void do_when_interrupted(){
  //run_flag=0;
  printtime();
  fprintf(stdout,"SHUTDOWN\n");
  //fprintf(stderr,"debug purpose entered here2");
  if(checklog){
    //fprintf(stderr,"debug purpose entered here3");
    fprintf(filepointer,"%s","SHUTDOWN\n");
    fflush(filepointer);
  }
  mraa_aio_close(temperature);
  mraa_gpio_close(button);
  exit(0);
}

void domany(const char * big){
  
  if(strcmp(big,"OFF")==0){
    //fprintf(stderr,"debug purpose entered here6");
    if(checklog){fprintf(filepointer,"%s\n","OFF");
      //fprintf(stderr,"debug purpose entered here4");
      fflush(filepointer);}
    
    do_when_interrupted();
  }
  else if (strcmp(big,"SCALE=C")==0){
    scale='C';
    if(checklog){fprintf(filepointer,"%s\n","SCALE=C");fflush(filepointer);}
  }
  else if (strcmp(big,"SCALE=F")==0){
    scale='F';
    if(checklog){fprintf(filepointer,"%s\n","SCALE=F");fflush(filepointer);}
  }
  else if (strcmp(big,"STOP")==0){
    
    manualstop=0;
    if(checklog){fprintf(filepointer,"%s\n","STOP");fflush(filepointer);}
  }
  else if (strcmp(big,"START")==0){
    //fprintf(stderr,"debug purpose entered here99");
    manualstop=1;
    if(checklog){fprintf(filepointer,"%s\n","START");fflush(filepointer);}
  }
  else if(strncmp(big,"PERIOD=",7*sizeof(char))==0){
    period=atoi(big+7);
    if(checklog){fprintf(filepointer,"%s\n",big);fflush(filepointer);}
  }
  else if(strncmp(big,"LOG ",4*sizeof(char))==0){
    if(checklog){fprintf(filepointer,"%s\n",big);fflush(filepointer);}
  }
  else{
    fprintf(stderr,"bad argument from stdin");
    fprintf(stderr,"%s",big);
    exit(1);
  }
}

int main(int argc, char **argv){
  static struct option long_options[]= {
    {"period",required_argument,0,'p'},
    {"scale",required_argument, 0, 's'},
    {"log",required_argument,0,'l'},
    {0,0,0,0}
  };
  int ret=0;
  while((ret=getopt_long(argc,argv,"p:s:l",long_options,NULL))!=-1){
    switch(ret){
    case 'p':
      period=atoi(optarg);
      break;
    case 's':
      if((optarg[0]=='C' || optarg[0]== 'F')&&strlen(optarg)==1){
	scale=optarg[0];}
      else {fprintf(stderr,"incorrect argument for scale\n");exit(1);}
      break;
    case 'l':
      checklog=1;
      filepointer=fopen(optarg,"w");
      if(filepointer==NULL){fprintf(stderr,"fail to open file\n");exit(1);}
      break;
    default:
      fprintf(stderr,"bad argument at start time");exit(1);
    }//end of switch
  }//end of while
  
  //mraa_aio_context temperature; initialize to the beginning so can close
  temperature=mraa_aio_init(1);

  //mraa_gpio_context button;
  button=mraa_gpio_init(60);
  mraa_gpio_dir(button,MRAA_GPIO_IN);
  mraa_gpio_isr(button,MRAA_GPIO_EDGE_RISING,&do_when_interrupted,NULL);

  time_t current,begin,end;
  char formatstring[15];
  struct tm* timestruct;
  
  struct pollfd pool[1];
  pool[0].fd=STDIN_FILENO;
  pool[0].events= POLLIN | POLLHUP | POLLERR;
  
  
  while(1){
    goodtemperature=mraa_aio_read(temperature);
    double temp = 1023.0 / ((double)goodtemperature)-1.0;
    temp=100000.0 * temp;
    double ctemperature=1.0 /(log(temp/100000.0)/4275+1/298.15)-273.15;
    if(scale=='F'){ctemperature=ctemperature *9/5 +32;}
    //fprintf(stdout,"no seg fault up to here1");
    time(&current);
    timestruct=localtime(&current);
    strftime(formatstring,15, "%H:%M:%S",timestruct);
    //fprintf(stdout,"no seg fault up to here2");
    if(manualstop){
      fprintf(stdout,"%s %.1f\n",formatstring, ctemperature);
      if(checklog){
      fprintf(filepointer,"%s %.1f\n",formatstring, ctemperature);
      fflush(filepointer);
      }
    }
    time(&begin);
    time(&end);
    //fprintf(stdout,"no seg fault up to here3");
    while(difftime(end,begin)<period){
      int result=poll(pool,1,0);
      if(result<0){fprintf(stderr,"poll pool error");exit(1);}
      if(pool[0].revents & POLLIN){
	char big[500];char small[20];int j=0;
	memset(big,0,500);
	memset(small,0,20);
	int num_read=read(STDIN_FILENO,big,500);
	//fprintf(stderr,"%s\n",big);
	if(num_read<0){exit(1);}
	for(int i=0;i<num_read;i++){
	  if(big[i]=='\n'){
	    domany(small);
	    j=0;
	    memset(small,0,20);
	  }
	  else{
	    small[j]=big[i];
	    j++;
	  }
	}//end of read loop

      }
      time(&end);
    }//small while loop ends
  }//end of while loop
  exit(0);
}
