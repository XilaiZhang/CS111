#define _POSIX_SOURCE //stop the warning from kill
#define _POSIX_C_SOURCE 200809L
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
//#include <sys/wait.h> //used by waitpid
#include <sys/socket.h> //used by socket
#include <netinet/in.h> //for internet domain addresses
#include <netdb.h>  //defines the struct hostent
#include "zlib.h"
z_stream client_to_server;
z_stream server_to_client;

int port;
char * filename;
int filepointer;
char smallbuffer[1]={'\n'};


struct termios dearpointer; //global so that restore can see
struct termios modified;

int shellflag=0;
int fileflag=0;
int compressflag=0;
char minibuffer[2]={'\r','\n'};  //damn C declaration syntax
void endflate() {

  deflateEnd(&client_to_server);

  inflateEnd(&server_to_client);

}

void restore()
{ int ret=tcsetattr(STDIN_FILENO,TCSANOW,&dearpointer);
  if(ret!=0)
    { fprintf(stderr, "tcsetattr error: %s", strerror(errno));
      exit(EXIT_FAILURE);
    }
  
}


int main(int argc, char **argv)
{  
  int ret=0;
  
  static struct option long_options[]=
  {
    
    {"port",required_argument,0,'p'},
    {"log",optional_argument,0,'l'},
    {"compress",optional_argument,0,'c'},
    {0,0,0,0}
  };
  
  while((ret=getopt_long(argc,argv,"p:l:c",long_options,NULL))!=-1)
    {
      switch(ret){
        case 's':
	  shellflag=1;//to be filled
	  break;
	case 'p':
	  port=atoi(optarg);
	  break;
	case 'l':
	  fileflag=1;
	  filename=optarg;
	  break;
	case 'c':
	  compressflag=1;
	  
	  break;
        default:
	  fprintf(stderr,"unrecognized option, use: client --port=port number [--log=filename] [--compress]\n");
	  exit(EXIT_FAILURE);
      }
    }
  if(fileflag){
    filepointer=creat(filename,0777);
    if(filepointer<0)//0 on success, -1 on failure
      {fprintf(stderr,"log can't be created, error msg: %s",strerror(errno));exit(EXIT_FAILURE);}
  }
  if(compressflag){
    atexit(endflate);
    client_to_server.zalloc=Z_NULL;
    client_to_server.zfree=Z_NULL;
    client_to_server.opaque=Z_NULL;//to be filled out later
    ret=deflateInit(&client_to_server,Z_DEFAULT_COMPRESSION);
    if(ret!=Z_OK){exit(EXIT_FAILURE);}
    server_to_client.zalloc=Z_NULL;
    server_to_client.zfree=Z_NULL;
    server_to_client.opaque=Z_NULL;//to be filled out later
    ret=inflateInit(&server_to_client);
    if(ret!=Z_OK){exit(EXIT_FAILURE);}
    
  }
  ret=tcgetattr(STDIN_FILENO, &dearpointer);
   
  if(ret<0)//0 on success, -1 on failure
    { 
      fprintf(stderr,"tcgetattr is not successful, error msg: %s",strerror(errno)); //errno will be set by tcgetattr
      exit(EXIT_FAILURE);
    }
  atexit(restore); //restore is a function pointer
  
  ret=tcgetattr(STDIN_FILENO, &modified); 
  if(ret<0)//0 on success, -1 on failure
    {
      fprintf(stderr,"tcgetattr is not successful, error msg: %s",strerror(errno)); //errno will be set by tcgetattr
      exit(EXIT_FAILURE);
    }
  modified.c_iflag=ISTRIP;
  modified.c_oflag=0;
  modified.c_lflag=0;
  ret=tcsetattr(STDIN_FILENO,TCSANOW,&modified);
  if(ret!=0)
    { fprintf(stderr, "tcsetattr error: %s", strerror(errno));
      exit(EXIT_FAILURE);
    }
  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  sockfd=socket(AF_INET,SOCK_STREAM,0);
  if (sockfd<0){fprintf(stderr,"error opening socket");exit(EXIT_FAILURE);}
  server=gethostbyname("localhost");
  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(EXIT_FAILURE);
  }
  memset((char*)&serv_addr,0, sizeof(serv_addr)); //add a type cast since it is in the tutorial
  serv_addr.sin_family=AF_INET;
  memcpy((char*)&serv_addr.sin_addr.s_addr,(char*)server->h_addr_list[0],server->h_length);
  serv_addr.sin_port=htons(port);//network byte order
  if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
    {fprintf(stderr,"error connect socket: %s",strerror(errno));exit(EXIT_FAILURE);}
  
       struct pollfd listen1[2];
       listen1[0].fd=STDIN_FILENO;
       listen1[0].events= POLLIN | POLLHUP |POLLERR;
       listen1[1].fd=sockfd;
       listen1[1].events= POLLIN | POLLHUP |POLLERR;
       while(1){
	 ret=poll(listen1,2,0);
	 if(ret<0){
	   fprintf(stderr,"poll is not successful: %s",strerror(errno));
	   exit(EXIT_FAILURE);
	 }
	 if(listen1[0].revents & POLLIN){
	   //printf("it inside this state1\n\n\n\n");
	   //read and write from terminal
	   ssize_t res=0;
	   unsigned char buffer1[256];
	   unsigned char buffer1copy[256];
	   
	   res=read(STDIN_FILENO,buffer1,256);
	   memcpy(buffer1copy,buffer1,256);
	   if(res<0){exit(EXIT_FAILURE);}
	   char compressionbuffer[256];  
	         
	         if(compressflag){
		     client_to_server.avail_in=res;
		     client_to_server.next_in=(unsigned char*)buffer1; //might produce 
		     client_to_server.avail_out=256;
		     client_to_server.next_out=(unsigned char*)compressionbuffer;
		     do{
		       deflate(&client_to_server,Z_SYNC_FLUSH);
		     }while(client_to_server.avail_in>0);
		     write(sockfd,compressionbuffer,256-client_to_server.avail_out); //256-
		     if(fileflag){
		       dprintf(filepointer,"SENT %d bytes: ",(int)res);
		       write(filepointer,compressionbuffer,256-client_to_server.avail_out);
		       write(filepointer,"\n",1);
		     }
		 }
		 else{ret=write(sockfd,buffer1,res);
		   if(ret<0)
		       { fprintf(stderr, "write error: %s", strerror(errno));
			 exit(EXIT_FAILURE);
		       }
		 }
	         for(int i=0;i<res;i++)
		 {
		   char selected=buffer1[i];
		   if(compressflag)
		     {selected=buffer1copy[i];}
		   
		   
		   
		     
		   switch(selected){
		   case '\r':
		   case '\n':
		     ret=write(STDOUT_FILENO,minibuffer,2);  //2 instead of 1
		     if(ret<0)
		       { fprintf(stderr, "write error: %s", strerror(errno));
			 exit(EXIT_FAILURE);
		       }
		     break;
		   
		   default:
		     ret=write(STDOUT_FILENO,buffer1+i,1);
		     if(ret<0)
		       { fprintf(stderr, "write error: %s", strerror(errno));
			 exit(EXIT_FAILURE);
		       }
		     
		     
		   }//switch
		 }//for
	     //end of the read and write
		 if(!compressflag && fileflag){
		   //fprintf(stderr,"debug,entered this block");
		   dprintf(filepointer,"SENT %d bytes: ",(int)res);
		   //write(filepointer, tempbuffer, 14);
		       write(filepointer,buffer1copy,res);
		       //else{write(filepointer,buffer1+i,1);}
		       write(filepointer,"\n",1);
		     }
	 }//listen1[0]
	 if(listen1[0].revents & (POLLHUP+POLLERR)){
	   break;
	 }
	 if(listen1[1].revents & POLLIN){
	               ssize_t res=0;
	   char buffer2[256];
	   char compressionbuffer[256];
	   int num_bytes;
	   res=read(sockfd,buffer2,256);
	   if(res==0){
	     if(close(sockfd)<0){exit(EXIT_FAILURE);}
	     exit(0);
	   }
	   if(res<0){exit(EXIT_FAILURE);}
	   if(fileflag){
	     dprintf(filepointer,"RECEIVED %d bytes: ",(int)res);
		       //write(filepointer, tempbuffer, 18);
		       write(filepointer,buffer2,res);
		       write(filepointer,"\n",1);
		     }
	   if(compressflag){
	             server_to_client.avail_in=res;
		     server_to_client.next_in=(unsigned char *)buffer2; //might produce 
		     server_to_client.avail_out=256;
		     server_to_client.next_out=(unsigned char *)compressionbuffer;
		     do{
		       inflate(&server_to_client,Z_SYNC_FLUSH);
		     }while(server_to_client.avail_in>0);
		     num_bytes=256-server_to_client.avail_out;
		     /*if(fileflag){
		       dprintf(filepointer,"RECEIVED %d bytes: ",(int)res);
		       
		       write(filepointer,compressionbuffer,num_bytes);
		       write(filepointer,"\n",1);
		       }*/
	   }else{memcpy(compressionbuffer,buffer2,256);}
	   
	   //printf("debugging purpose on the client side: %s, and valid size:%zd\n",compressionbuffer,res);
	   if(compressflag){res=num_bytes;}
	       for(int i=0;i<res;i++)
		 {
		   
		   char selected=compressionbuffer[i];
		   switch(selected){
		   
		   case 0x0A:
		    
		     ret=write(STDOUT_FILENO,minibuffer,2);  //2 instead of 1
		     if(ret<0)
		       { fprintf(stderr, "write error: %s", strerror(errno));
			 exit(EXIT_FAILURE);
		       }
		     break;
		   default:
		     ret=write(STDOUT_FILENO,compressionbuffer+i,1);
		     if(ret<0)
		       { fprintf(stderr, "write error: %s", strerror(errno));
			 exit(EXIT_FAILURE);
		       }
		     
		   }//switch
		 }//for
	     //end of the read and write
	   
	 }//listen1[1]
	 if(listen1[1].revents & (POLLHUP | POLLERR))
	   { int info=close(sockfd);
	     if(info<0){fprintf(stderr,"close on client fails: %s",strerror(errno));}
	     exit(0);
	   }
       }//while poll loop ends at here
}//int main
