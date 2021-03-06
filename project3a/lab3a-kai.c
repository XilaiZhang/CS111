//header files not fully added right now
#include <unistd.h> //enough for pread
#include "ext2_fs.h" // all the ext2 values professor gave
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>




int fd=-1;
struct ext2_super_block info; //this struct is defined in "ext2_fs.h" and matches with the description on http://www.nongnu.org/ext2-doc/ext2.html#S-INODES-COUNT
int blocksize=-1;
int num_groups=-1;
int blocks_per_group = -1;
struct ext2_group_desc * group_desc;

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


int main(int argc, char **argv){
  if(argc !=2)
    {fprintf(stderr,"bad arguments passed to the program\n"); exit(1);}
  const char *imagename = argv[1];
  fd = open(imagename,O_RDONLY);
  if(fd<0)
    {fprintf(stderr,"can't open file system\n");close(fd);exit(2);}
  
  superblockreport();
  desc_group_report();
  //free any dynamically allocated structures
  exit(0);
}
