#define _POSIX_SOURCE //stop the warning from kill
#include <stdio.h>
#include <stdlib.h>   //to use atexit
#include <getopt.h>      //for get opt
#include <errno.h>
#include <termios.h>     //to use termios
#include <unistd.h>
#include <string.h>    // to use strerror
#include <signal.h>  // to use signal
#include <fcntl.h>   //used by pipe
#include <sys/types.h> //used by fork
#include <poll.h>  //used by poll
#include <sys/wait.h> //used by waitpid
#include <sys/socket.h>
#include <netinet/in.h>
#include "zlib.h"
//#include <sys/ioctl.h> //sha
//#include <sys/stat.h>
z_stream sendout;
z_stream receive;

int port;
int compressflag=0;
char smallbuffer[1]={'\n'};
int terminaltoshell[2];
int shelltoterminal[2];  //don't understand why can't declare right before it

//int ret=0;
pid_t catch=-1; //is it the problem of initializaion?
int shellflag=0;
char minibuffer[2]={'\r','\n'};  //damn C declaration syntax
void endflate() {

  deflateEnd(&sendout);

  inflateEnd(&receive);

}


void restore_1()
{   
    int status;//to be filled out later..waitpid and stuff
    pid_t noidea=waitpid(catch,&status,0);
    if(noidea==-1) {fprintf(stderr,"error is executing this part, waitpid: %s", strerror(errno)); exit(EXIT_FAILURE);} //to avoid a warning of type conflict
    //waitpid(catch,&status,0);
    //if(WIFEXITED(status)) { fprintf(stderr,"SHELL EXIT SIGNAL=%d STATUS=%d\n", 0,WEXITSTATUS(status)); }
    if ( WIFSTOPPED(status) ) {
      int signum1 = WSTOPSIG(status);
      fprintf(stderr,"Stopped due to receiving signal %d\n", signum1);
    }

    //else if(WIFSIGNALED(status)) {
    fprintf(stderr,"SHELL EXIT SIGNAL=%d STATUS=%d\n", (0x007f&status),(status&0xff00)>>8);// }

    //exit(0);//redundant
      
}
void handler(int signum)
{
  if(signum==SIGPIPE)
    {exit(0);}
  /*if(signum==SIGINT)
    {exit(0);}
  */
  if(signum==SIGTERM)
    {exit(0);} //only change
  if(signum==SIGINT)
    {exit(0);}
}

int main(int argc, char **argv)
{  
  int ret=0;
  
  static struct option long_options[]=
  {
    {"port", required_argument, 0, 'p'},
    {"compress",optional_argument,0,'c'},
    {0,0,0,0}
  };
  
  while((ret=getopt_long(argc,argv,"p:c",long_options,NULL))!=-1)
    {
      switch(ret){
        case 'p':
	  port=atoi(optarg);
	  break;
        case 'c':
	  compressflag=1;//to be filled in later
	  break;
        default:
	  fprintf(stderr,"unrecognized option, use: server --port= port number [--compress] \n");
	  exit(EXIT_FAILURE);
      }
    }
  if(signal(SIGPIPE,handler)==SIG_ERR){exit(EXIT_FAILURE);}
  if(signal(SIGTERM,handler)==SIG_ERR){exit(EXIT_FAILURE);}
  if(signal(SIGINT,handler)==SIG_ERR){exit(EXIT_FAILURE);}
  if(compressflag){
    atexit(endflate);
    sendout.zalloc=Z_NULL;
    sendout.zfree=Z_NULL;
    sendout.opaque=Z_NULL;//to be filled out later
    ret=deflateInit(&sendout,Z_DEFAULT_COMPRESSION);
    if(ret!=Z_OK){exit(EXIT_FAILURE);}
    receive.zalloc=Z_NULL;
    receive.zfree=Z_NULL;
    receive.opaque=Z_NULL;//to be filled out later
    ret=inflateInit(&receive);
    if(ret!=Z_OK){exit(EXIT_FAILURE);}

  }

  //atexit(restore_1);
  int sockfd, newsockfd;unsigned int clilen; //to avoid warning
  struct sockaddr_in serv_addr,cli_addr;
  sockfd=socket(AF_INET,SOCK_STREAM,0);
  if(sockfd<0){
    fprintf(stderr,"error open socket:%s",strerror(errno));
    
    close(sockfd);
    exit(EXIT_FAILURE);
  }
  memset((char*)&serv_addr,0,sizeof(serv_addr));
  serv_addr.sin_family=AF_INET;
  serv_addr.sin_port=htons(port);
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  if(bind(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
      fprintf(stderr,"bind error: %s",strerror(errno));
      
      //close(sockfd);
      exit(EXIT_FAILURE);
    }
  if(listen(sockfd,5)<0){exit(EXIT_FAILURE);}
  clilen=sizeof(cli_addr);
  newsockfd=accept(sockfd,(struct sockaddr *)&cli_addr,&clilen);
  if(newsockfd<0){
    fprintf(stderr,"socket accept error: %s",strerror(errno));
    //close(newsockfd);
    //close(sockfd);
    exit(EXIT_FAILURE);
  }
  
  
  
    
    ret=pipe(terminaltoshell);
    if(ret<0){
      fprintf(stderr,"error set by pipe: %s",strerror(errno));
      //close(newsockfd);
      //close(sockfd);
      exit(EXIT_FAILURE);	      }
    ret=pipe(shelltoterminal);
    if(ret<0){
      fprintf(stderr,"error set by pipe: %s",strerror(errno));
      //close(newsockfd);
      //close(sockfd);
      exit(EXIT_FAILURE);	      }
    
    catch=fork();
    if(catch<0){
      fprintf(stderr,"errno is set by fork: %s",strerror(errno));
      
      //close(newsockfd);
      //close(sockfd);
      exit(EXIT_FAILURE);
    }
    else if(catch==0){
      //in child process
     ret=close(terminaltoshell[1]);
     if(ret<0){
       fprintf(stderr,"close pipe is not successful: %s",strerror(errno));
       exit(EXIT_FAILURE);
      }
     ret=close(shelltoterminal[0]);
     if(ret<0){
       fprintf(stderr,"close pipe is not successful: %s",strerror(errno));
       exit(EXIT_FAILURE);
      }
     ret=dup2(terminaltoshell[0],0);//should not matter
     if(ret<0){
       fprintf(stderr,"dup is not successful: %s",strerror(errno));
       exit(EXIT_FAILURE);
      }
     ret=dup2(shelltoterminal[1],1);
     if(ret<0){
       fprintf(stderr,"dup is not successful: %s",strerror(errno));
       exit(EXIT_FAILURE);
      }
     ret=dup2(shelltoterminal[1],2);
     if(ret<0){
       fprintf(stderr,"dup is not successful: %s",strerror(errno));
       exit(EXIT_FAILURE);
      }
     ret=close(terminaltoshell[0]);
     if(ret<0){
       fprintf(stderr,"close pipe is not successful: %s",strerror(errno));
       exit(EXIT_FAILURE);
      }
     ret=close(shelltoterminal[1]);
     if(ret<0){
       fprintf(stderr,"close pipe is not successful: %s",strerror(errno));
       exit(EXIT_FAILURE);
      }
     char chararray[]="/bin/bash";
     char* pointerarray[2];
     pointerarray[0]=chararray;
     pointerarray[1]=NULL;
     ret=execvp(chararray,pointerarray);
     if(ret<0){
       fprintf(stderr,"execvp is not successful: %s",strerror(errno));
       
       //close(newsockfd);
       //close(sockfd);
       exit(EXIT_FAILURE);
      }
    }
    else { //when in parent, child pid is returned
       ret=close(terminaltoshell[0]);
       if(ret<0){
       fprintf(stderr,"close pipe is not successful: %s",strerror(errno));
       exit(EXIT_FAILURE);
       } 
       ret=close(shelltoterminal[1]);
       if(ret<0){
       fprintf(stderr,"close pipe is not successful: %s",strerror(errno));
       exit(EXIT_FAILURE);
       }
       struct pollfd listen1[2];
       listen1[0].fd=newsockfd;
       listen1[0].events= POLLIN | POLLHUP |POLLERR;
       listen1[1].fd=shelltoterminal[0];
       listen1[1].events= POLLIN | POLLHUP |POLLERR;
       while(1){
	 ret=poll(listen1,2,0);
	 if(ret<0){
	   fprintf(stderr,"poll is not successful: %s",strerror(errno));
	   
	   //close(newsockfd);
	   //close(sockfd);
	   exit(EXIT_FAILURE);
	 }
	 if(listen1[0].revents & POLLIN){
	   
	   ssize_t res=0;
	   char buffer1[256];
	   char compressionbuffer[256];
	   int num_bytes;
	   res=read(newsockfd,buffer1,256);
	   if(res<=0){
	     if(kill(catch,SIGTERM)<0){exit(EXIT_FAILURE);}
	     restore_1();break; //only change?
	   }
	       
	   if(compressflag){
	     receive.avail_in=res;
	     receive.next_in=(unsigned char *)buffer1; //might produce
	     receive.avail_out=256;
	     receive.next_out=(unsigned char *)compressionbuffer;
	     do{
	       inflate(&receive,Z_SYNC_FLUSH);
	     }while(receive.avail_in>0);
	     num_bytes=256-receive.avail_out;
           }else{memcpy(compressionbuffer,buffer1,256);}
	   
	   //fprintf(stderr,"debug purpose: %s, and the valid size:%zd\n",compressionbuffer,res);
	   if(compressflag){res=num_bytes;}
	   for(int i=0;i<res;i++)
		 {
		   char selected=compressionbuffer[i];
		   //fprintf(stderr,"debug purpose: %c", selected);
		   switch(selected){
		   case '\r':
		   case '\n':
		     
		     ret=write(terminaltoshell[1],smallbuffer,1);  //2 instead of 1
		     if(ret<0)
		       { fprintf(stderr, "write error: %s", strerror(errno));
			 exit(EXIT_FAILURE);
		       }
		     break;//this fucking break, oh my lord, two days of debugging
		   case 0x04:
		     ret=close(terminaltoshell[1]);
		     if(ret<0)
		       { fprintf(stderr, "close error: %s", strerror(errno));
			 exit(EXIT_FAILURE);
		       }
		     break;
		   case 0x03:
		     kill(catch,SIGINT);  //hope catch is defined in this scope
		     break;
		   default:
		     
		     ret=write(terminaltoshell[1],compressionbuffer+i,1);
		     if(ret<0)
		       { fprintf(stderr, "write error: %s", strerror(errno));
			 exit(EXIT_FAILURE);
		       }
		   }//switch
		 }//for
	     //end of the read and write
	 }//listen1[0]
	 if(listen1[1].revents & POLLIN){
	               ssize_t res=0;
	   char buffer2[256];
	   char compressionbuffer[256];
	   res=read(shelltoterminal[0],buffer2,256);//read from shell and sendout out
	   //fprintf(stderr,"debug purpose: %s, and the valid size:%zd\n",buffer2,res);
	   if(res==0){ //might need furthre improvement
	     restore_1();break;//is break really needed?
	   }
	         if(compressflag){
                     sendout.avail_in=res;
                     sendout.next_in=(unsigned char*)buffer2; //might produce
                     sendout.avail_out=256;
                     sendout.next_out=(unsigned char*)compressionbuffer;
                     do{
                       deflate(&sendout,Z_SYNC_FLUSH);
                     }while(sendout.avail_in>0);
                     write(newsockfd,compressionbuffer,256-sendout.avail_out); //256-
		     
		 }
	         
		   
                   else{ 
		     ret=write(newsockfd,buffer2,res);
		     if(ret<0)
		       { fprintf(stderr, "write error: %s", strerror(errno));
			 exit(EXIT_FAILURE);
		       }
		   }
		 //}//for
		 
	     //end of the read and write
	   
	 }//listen1[1]
	 if((listen1[1].revents &POLLHUP) ==POLLHUP|| ((listen1[1].revents &POLLERR)==POLLERR))
	   { if(close(listen1[1].fd)<0){exit(EXIT_FAILURE);}
	     restore_1();//
	     //close(newsockfd);
	     //close(sockfd);
	     //exit(0);
	     break;//why?
	   }
       }//while poll loop ends at here






    } //baskstick for parent
 
    //if necessary,manully exit 0 here
}
