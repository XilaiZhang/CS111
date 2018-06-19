#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>      //where close is defined
#include <fcntl.h>          //where open and creat is defined

void signalFunction(){      //checking functions does not allow parameter here,especially warning output
  fprintf(stderr,"segementation fault caught, the description: %s \n", strerror(errno));
  exit(4);
}

int main(int argc, char** argv)
{
  static struct option long_options[]=
    {
      {"input",required_argument, 0, 'i'},
      {"output",required_argument,0,'o'},
      {"segfault",no_argument,0,'s'},
      {"catch",no_argument,0,'c'},
      {0,0,0,0}   //mark the end of the array, no practical use
    };
  int ret;
  int option_index=0;
  int storeseg=0;
  int signalFlag=0;
  char * input= NULL;
  char * output= NULL; 
  while((ret=getopt_long(argc,argv,"i:o:sc",long_options,&option_index))!=-1)  //:indicate required argument; returns -1 when no more argument
    {
      switch(ret)
	{
	case 'i':
	  input=optarg;
	  break; //optarg stores the argument 
	case 'o':
	  output=optarg;
	  break;
	case 's':
	  storeseg=1;
	  break;
	case 'c':
	  signalFlag=1;
	  //signal(SIGSEGV,signalFunction);
	  break;
	default:
	  printf("correct usage: lab0 --input filename --output filename [sc]");
	  exit(1);
	}

    }
  if(input)
    {
      int ifd=open(input,O_RDWR); //let me give it both read and write permission
      if(ifd!=-1)
	{ close(0);   //close 0 to get space
	  dup(ifd);  //create a duplicate to 0
	  close(ifd); //now the original onw is redundant
	}
      else
	{
	  fprintf(stderr,"%s the input file can not be opened, description: %s", input, strerror(errno)); 
	  exit(2);
	}
    }
  if(output)
    {
      int ofd=creat(output,0777); //let me give it read,write and execution permission
      if(ofd!=-1)
	{ close(1);   //close 1 to get space
	  dup(ofd);  //create a duplicate to 1
	  close(ofd); //now the original onw is redundant
	}
      else
	{
	  fprintf(stderr,"%s the output file can not be opened, description: %s", output, strerror(errno)); 
	  exit(3);
	}
    }
  if(signalFlag)
    { signal(SIGSEGV,signalFunction);
    }
  if(storeseg)
    { char * setNull= NULL;
      *setNull=47;
    }

  char buffer[2];
  ssize_t count;
  while((count=read(0,buffer,1))!=0)
    {write(1,buffer,count);} //buffer is of type char*
  exit(0);
}
