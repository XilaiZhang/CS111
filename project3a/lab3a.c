
#include <unistd.h> //enough for pread
#include "ext2_fs.h" // all the ext2 values professor gave
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <stdint.h> //uint_32
#include <math.h>

int fd=-1;
struct ext2_super_block info; //this struct is defined in "ext2_fs.h" and matches with the description on http://www.nongnu.org/ext2-doc/ext2.html#S-INODES-COUNT
int blocksize=-1;
int num_groups=-1;
int blocks_per_group = -1;
struct ext2_group_desc * group_desc;
struct ext2_dir_entry directory;
void superblockreport(){
  if( pread(fd,&info,sizeof(struct ext2_super_block),1024) < 0){//http://www.nongnu.org/ext2-doc/ext2.html#SUPERBLOCK
    fprintf(stderr,"pread superblock fails");close(fd);exit(2);
  }  //now stored in info
  if(EXT2_SUPER_MAGIC != info.s_magic){ //compare ext2_fs.h with struct
    fprintf(stderr,"magic number mismatch");close(fd);exit(2);
  }
  blocksize = EXT2_MIN_BLOCK_SIZE << info.s_log_block_size;
  //http://www.nongnu.org/ext2-doc/ext2.html#S-LOG-BLOCK-SIZE
  printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n",info.s_blocks_count,
	 info.s_inodes_count,blocksize,info.s_inode_size,
	 info.s_blocks_per_group,info.s_inodes_per_group,
	 info.s_first_ino);

  num_groups = info.s_blocks_count/info.s_blocks_per_group+1;
  blocks_per_group = info.s_blocks_per_group;
  group_desc = (struct ext2_group_desc*)malloc(blocksize);
}

void desc_group_report(){
	if( pread(fd,group_desc,blocksize,blocksize*2) < 0){//http://www.nongnu.org/ext2-doc/ext2.html#SUPERBLOCK
    	fprintf(stderr,"pread desc_group_report fails");close(fd);exit(2);
	}  //desc_group now stored in info

	//group number is just the index
	for(int i = 0; i < num_groups; i++){
		if(i!=num_groups-1){
			printf("GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", i,blocks_per_group,info.s_inodes_per_group,group_desc[i].bg_free_blocks_count,
				group_desc[i].bg_free_inodes_count, 3, 4, 5);
		} else {
			//last group
			int num_inodes = 0;
			int num_blocks_per_group = 0;
			if(info.s_blocks_count%blocks_per_group==0) num_blocks_per_group = blocks_per_group;
			else num_blocks_per_group = info.s_blocks_count%blocks_per_group;
			if(info.s_inodes_count%info.s_inodes_per_group==0) num_inodes = info.s_inodes_per_group;
			else num_inodes = info.s_inodes_count%info.s_inodes_per_group;

			printf("GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", i,num_blocks_per_group,
				num_inodes,group_desc[i].bg_free_blocks_count,
				group_desc[i].bg_free_inodes_count, 3, 4, 5);


		}
	}

}




void free_block_report(){
	unsigned char *free_block_bitmap = (unsigned char*)group_desc;
	if( pread(fd,free_block_bitmap,blocksize,blocksize*3) < 0){//http://www.nongnu.org/ext2-doc/ext2.html#SUPERBLOCK
    	fprintf(stderr,"pread free_block_report fails");close(fd);exit(2);
	}  //read free block bitmap block
	//hexDump("free_block_bitmap", free_block_bitmap,1024);
	for(int i = 0; i < blocksize; i++){
		for(int j = 0; j < 8; j++)
			if(free_block_bitmap[i]>>j&1){
				//bit is allocated
			} else {
				printf("BFREE,%d\n", i*8+j+1);
			}
	}

}

void free_inode_report(){
	unsigned char *free_inode_bitmap = (unsigned char*)group_desc;
	if( pread(fd,free_inode_bitmap,blocksize,blocksize*4) < 0){//http://www.nongnu.org/ext2-doc/ext2.html#SUPERBLOCK
    	fprintf(stderr,"pread free_inode_report fails");close(fd);exit(2);
	}  //read free block bitmap block
	//hexDump("free_inode_bitmap", free_inode_bitmap,1024);
	for(int i = 0; i < blocksize; i++){
		for(int j = 0; j < 8; j++)
			if(free_inode_bitmap[i]>>j&1){
				//bit is allocated
			} else {
				printf("IFREE,%d\n", i*8+j+1);
			}
	}
}




#define EXT2_S_IFREG	0x8000
#define EXT2_S_IFDIR	0x4000
#define EXT2_S_IFLNK	0xA000

void print_time(int timestamp){
	 char buffer[26] = {0};
    time_t time = timestamp;
    struct tm* tm_info;
    tm_info = gmtime(&time);
    strftime(buffer, 26, "%m/%d/%y %H:%M:%S", tm_info);
    printf("%s",buffer); 
}

void indirect_recursive(int inode_no, uint32_t block_no, int level, uint32_t oldoffset){
  	if(level==0){return;} //end of recursion 
  	uint32_t pointerarray[1024];
  	memset(pointerarray,0,1024*4);
   	if (pread(fd, pointerarray, blocksize, block_no*blocksize) < 0) {
    	fprintf(stderr, "Error with pread");exit(1);
   	}
   	for(int i=0;i<blocksize/4;i++){
     	if(pointerarray[i]==0) continue; //continue instead of break
     	uint32_t offset = oldoffset+(uint32_t)(i*pow(256, level-1));
     	printf("INDIRECT,%u,%u,%u,%u,%u\n",inode_no,level,offset,block_no,pointerarray[i]);

     	if(level>1)
     		indirect_recursive(inode_no,pointerarray[i],level-1,offset);
   }//for loop ends
   
}


void indirectdir(unsigned int inode_no, unsigned int level, uint32_t block_no){
  if(level==0){return;} //end of recursion 
  uint32_t pointerarray[1024];
   memset(pointerarray,0,1024*4);
   if (pread(fd, pointerarray, blocksize, block_no*blocksize) < 0) {
     fprintf(stderr, "Error with pread"); exit(1);
   }
   for(int i=0;i<blocksize/4;i++){
     if(pointerarray[i]==0){continue;}//continue instead of break
     if(level==2 || level==3){indirectdir(inode_no,level-1,pointerarray[i]);}
    
     //same thing as directory begin
     int externaloffset=pointerarray[i]*blocksize;
     int internaloffset=0;
     while(internaloffset<blocksize){
       if( pread(fd,&directory,sizeof(struct ext2_dir_entry),externaloffset+internaloffset)<0 ){
	 fprintf(stderr,"pread free_inode_report fails");close(fd);exit(2);
       }
       if(directory.inode==0){internaloffset+=directory.rec_len; continue;}
       printf("DIRENT,%d,%d,%d,%d,%d,'%s'\n",inode_no,internaloffset, directory.inode,directory.rec_len,directory.name_len,directory.name);
       internaloffset+=directory.rec_len;
     }
          
     //same thing as directory end
   }

}

void inode_summary_report(){
	int num_bytes =  info.s_inodes_per_group*info.s_inode_size;
	struct ext2_inode *inode_itself = (struct ext2_inode *)malloc( num_bytes);
	if( pread(fd,inode_itself, num_bytes, blocksize*5) < 0){//http://www.nongnu.org/ext2-doc/ext2.html#SUPERBLOCK
    	fprintf(stderr,"pread free_inode_report fails");close(fd);exit(2);
	}  //read free block bitmap block
	for(unsigned int i = 0; i < info.s_inodes_per_group; i++){
		char file_type = '?';
		if(inode_itself[i].i_mode == 0 || inode_itself[i].i_links_count == 0) continue;
		if ((inode_itself[i].i_mode&EXT2_S_IFREG) == EXT2_S_IFREG) file_type = 'f';
		if ((inode_itself[i].i_mode&EXT2_S_IFDIR) == EXT2_S_IFDIR) file_type = 'd';
		if ((inode_itself[i].i_mode&EXT2_S_IFLNK) == EXT2_S_IFLNK) file_type = 's';

		printf("INODE,%d,%c,%o,%d,%d,%d,",i+1,
			file_type,(inode_itself[i].i_mode&4095),(inode_itself[i].i_uid),
			(inode_itself[i].i_gid), (inode_itself[i].i_links_count) );

		print_time(inode_itself[i].i_ctime);
		printf(",");
		print_time(inode_itself[i].i_mtime);
		printf(",");
		print_time(inode_itself[i].i_atime);
		printf(",%d,%d",inode_itself[i].i_size,inode_itself[i].i_blocks );
	    for(int k = 0; k < EXT2_N_BLOCKS; k++){
	    	if(file_type!='s')
	    		printf(",%d", inode_itself[i].i_block[k]);
	    	else {
	    		printf(",%d", inode_itself[i].i_block[k]); break;
	    	}
	    }
	    printf("\n");
	    //if(file_type=='d'||file_type=='f'){ //note that indirect blocks can exist in non directory blocks
	     if(file_type=='d'){
	       for(int j=0;j<12;j++){
				if(inode_itself[i].i_block[j]==0){continue;}
				int externaloffset=inode_itself[i].i_block[j]*blocksize;
				int internaloffset=0;
				while(internaloffset<blocksize){
				  if( pread(fd,&directory,sizeof(struct ext2_dir_entry),externaloffset+internaloffset)<0 ){
				    fprintf(stderr,"pread free_inode_report fails");close(fd);exit(2);
				  }
				  if(directory.inode==0){internaloffset+=directory.rec_len;continue;}
				  printf("DIRENT,%d,%d,%d,%d,%d,'%s'\n",i+1,internaloffset, directory.inode,directory.rec_len,directory.name_len,directory.name);
				  internaloffset+=directory.rec_len;
				}
	       	}//direct blocks in a directory
	       //unsigned int byte_offset=0;
	      	for(int j=12;j<15;j++){
		 		if(inode_itself[i].i_block[j]!=0)
		   			indirectdir(i+1,j-11,inode_itself[i].i_block[j]);
	       	}//indirect blocks in a directory
	      }//process of directory blocks
	      //printf("I hate debug2: %d %d",inode_itself[i].i_block[12],i+1);
	      
	      //indirect blocks
	      if (inode_itself[i].i_block[EXT2_IND_BLOCK] != 0) {
			indirect_recursive(i+1, inode_itself[i].i_block[EXT2_IND_BLOCK], 1, 12);
	      }
	      if (inode_itself[i].i_block[EXT2_DIND_BLOCK] != 0) {
			indirect_recursive(i+1, inode_itself[i].i_block[EXT2_DIND_BLOCK], 2, 12+1024/4);
	      }
	      if (inode_itself[i].i_block[EXT2_TIND_BLOCK] != 0) {
			indirect_recursive(i+1, inode_itself[i].i_block[EXT2_TIND_BLOCK], 3, 12+256+256*256);
	      }
	   
	    
	}


	free(inode_itself);
}



int main(int argc, char **argv){
  if(argc !=2)
    {fprintf(stderr,"bad arguments passed to the program\n"); exit(1);}
  const char *imagename = argv[1];
  fd = open(imagename,O_RDONLY);
  if(fd<0)
    {fprintf(stderr,"can't open file, gg \n");close(fd);exit(2);}
  
  superblockreport();
  desc_group_report();
  free_block_report();
  free_inode_report();
  inode_summary_report();
  free(group_desc);
  exit(0);
}
