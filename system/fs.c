#include <xinu.h>
#include <kernel.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>


#ifdef FS
#include <fs.h>

static struct fsystem fsd;
int dev0_numblocks;
int dev0_blocksize;
char *dev0_blocks;

extern int dev0;

char block_cache[512];

#define SB_BLK 0
#define BM_BLK 1
#define RT_BLK 2

#define NUM_FD 16
struct filetable oft[NUM_FD];
int next_open_fd = 0;


#define INODES_PER_BLOCK (fsd.blocksz / sizeof(struct inode))
#define NUM_INODE_BLOCKS (( (fsd.ninodes % INODES_PER_BLOCK) == 0) ? fsd.ninodes / INODES_PER_BLOCK : (fsd.ninodes / INODES_PER_BLOCK) + 1)
#define FIRST_INODE_BLOCK 2

int fs_fileblock_to_diskblock(int dev, int fd, int fileblock);

/* YOUR CODE GOES HERE */

int fs_fileblock_to_diskblock(int dev, int fd, int fileblock) {
  int diskblock;

  if (fileblock >= INODEBLOCKS - 2) {
    printf("No indirect block support\n");
    return SYSERR;
  }

  diskblock = oft[fd].in.blocks[fileblock]; //get the logical block address

  return diskblock;
}

/* read in an inode and fill in the pointer */
int
fs_get_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;
  int inode_off;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    printf("fs_get_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  inode_off = inn * sizeof(struct inode);

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  printf("inn*sizeof(struct inode): %d\n", inode_off);
  */

  bs_bread(dev0, bl, 0, &block_cache[0], fsd.blocksz);
  memcpy(in, &block_cache[inode_off], sizeof(struct inode));

  return OK;

}

int
fs_put_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    printf("fs_put_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  */

  bs_bread(dev0, bl, 0, block_cache, fsd.blocksz);
  memcpy(&block_cache[(inn*sizeof(struct inode))], in, sizeof(struct inode));
  bs_bwrite(dev0, bl, 0, block_cache, fsd.blocksz);

  return OK;
}
     
int fs_mkfs(int dev, int num_inodes) {
  int i;
  
  if (dev == 0) {
    fsd.nblocks = dev0_numblocks;
    fsd.blocksz = dev0_blocksize;
  }
  else {
    printf("Unsupported device\n");
    return SYSERR;
  }

  if (num_inodes < 1) {
    fsd.ninodes = DEFAULT_NUM_INODES;
  }
  else {
    fsd.ninodes = num_inodes;
  }

  i = fsd.nblocks;
  while ( (i % 8) != 0) {i++;}
  fsd.freemaskbytes = i / 8; 
  
  if ((fsd.freemask = getmem(fsd.freemaskbytes)) == (void *)SYSERR) {
    printf("fs_mkfs memget failed.\n");
    return SYSERR;
  }
  
  /* zero the free mask */
  for(i=0;i<fsd.freemaskbytes;i++) {
    fsd.freemask[i] = '\0';
  }
  
  fsd.inodes_used = 0;
  
  /* write the fsystem block to SB_BLK, mark block used */
  fs_setmaskbit(SB_BLK);
  bs_bwrite(dev0, SB_BLK, 0, &fsd, sizeof(struct fsystem));
  
  /* write the free block bitmask in BM_BLK, mark block used */
  fs_setmaskbit(BM_BLK);
  bs_bwrite(dev0, BM_BLK, 0, fsd.freemask, fsd.freemaskbytes);

  return 1;
}

void
fs_print_fsd(void) {

  printf("fsd.ninodes: %d\n", fsd.ninodes);
  printf("sizeof(struct inode): %d\n", sizeof(struct inode));
  printf("INODES_PER_BLOCK: %d\n", INODES_PER_BLOCK);
  printf("NUM_INODE_BLOCKS: %d\n", NUM_INODE_BLOCKS);
}

/* specify the block number to be set in the mask */
int fs_setmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  fsd.freemask[mbyte] |= (0x80 >> mbit);
  return OK;
}

/* specify the block number to be read in the mask */
int fs_getmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  return( ( (fsd.freemask[mbyte] << mbit) & 0x80 ) >> 7);
  return OK;

}

/* specify the block number to be unset in the mask */
int fs_clearmaskbit(int b) {
  int mbyte, mbit, invb;
  mbyte = b / 8;
  mbit = b % 8;

  invb = ~(0x80 >> mbit);
  invb &= 0xFF;

  fsd.freemask[mbyte] &= invb;
  return OK;
}

/* This is maybe a little overcomplicated since the lowest-numbered
   block is indicated in the high-order bit.  Shift the byte by j
   positions to make the match in bit7 (the 8th bit) and then shift
   that value 7 times to the low-order bit to print.  Yes, it could be
   the other way...  */
void fs_printfreemask(void) {
  int i,j;

  for (i=0; i < fsd.freemaskbytes; i++) {
    for (j=0; j < 8; j++) {
      printf("%d", ((fsd.freemask[i] << j) & 0x80) >> 7);
    }
    if ( (i % 8) == 7) {
      printf("\n");
    }
  }
  printf("\n");
}

//FS_OPEN
int fs_open(char *filename, int flags) {
	struct inode inDetails;		//Initiate Inode
	struct fsystem fsDetails;	//Initiate File System
	int fExist=0;				//Set a flag to check if the file exists
	int fd=next_open_fd;
	int i;

	bs_bread(dev0, SB_BLK, 0, &fsDetails, sizeof(struct fsystem));	//Read the fsystem struct data from block 0
	//Search through the directories
	for (i=0; i < fsDetails.root_dir.numentries; i++){
		if (strcmp(filename,fsDetails.root_dir.entry[i].name)==0){
			oft[fd].state=FSTATE_OPEN;  //Assign the state to open in file table
			oft[fd].fileptr=0;
			oft[fd].de=&fsDetails.root_dir.entry[i];
			fs_get_inode_by_num(0,fsDetails.root_dir.entry[i].inode_num,&inDetails);	//gets inode by inode number
			oft[fd].in=inDetails;
			inDetails.type=flags;
			fs_put_inode_by_num(0,fsDetails.root_dir.entry[i].inode_num,&inDetails);	//puts updated inode by inode number
			fExist=1;		//set the flag to 1 since we are in the loop
		}
	}
	if(fExist == 0){				//Enters the loop if there are no flags
		printf("No file exists\n");
		return SYSERR;	
	}
	next_open_fd++;					
	return fd;
}

//FS_CLOSE
int fs_close(int fd) {
	oft[fd].state=FSTATE_CLOSED;	//Assign the state to close in filetable
	oft[fd].fileptr=0;
	next_open_fd--;
	return OK;
}

//FS_CREATE
int fs_create(char *filename, int mode) {
	int i;
	int currentInode, currentFile;
	struct inode inDetails;
	struct fsystem fsDetails;
	struct dirent dEntry;
	
	bs_bread(dev0, SB_BLK, 0, &fsDetails, sizeof(struct fsystem));	//Read the fsystem struct data from block 0
	
	//If file created with a same filename
	if(mode == O_CREAT){
		for (i=0;i<fsDetails.root_dir.numentries;i++){
			if(strcmp(filename,fsDetails.root_dir.entry[i])==0){
				printf("Filename already exists\n");
				return SYSERR;
			}
		}
	}
	if (strlen(filename) > FILENAMELEN){
		printf("Filename length is greater than %d characters", FILENAMELEN);
		return SYSERR;
	}
	
	currentInode=fsDetails.inodes_used;		//Assigning to currentInode
	fsDetails.inodes_used++;
	currentFile=fsDetails.root_dir.numentries;	//Assigning to currentFile
	fsDetails.root_dir.numentries++;
	dEntry.inode_num=currentInode;			//Populating the directory Entry
	strcpy(dEntry.name,filename);
	fsDetails.root_dir.entry[currentFile]=dEntry;  
	
	//Initiating Inode entries
	inDetails.id=currentInode;
	inDetails.type=O_RDWR;
	inDetails.nlink=1;
	inDetails.device=dev0;
	inDetails.size=0;
	
	//Initiate filetable entries
	oft[currentFile].state=FSTATE_OPEN;
	oft[currentFile].fileptr=0;
	oft[currentFile].de=&dEntry;
	oft[currentFile].in=inDetails;
	
	next_open_fd++;
	bs_bwrite(dev0,SB_BLK,0,&fsDetails,sizeof(struct fsystem));	//Write the fsystem to block 0 
	fs_put_inode_by_num(dev0,currentInode,&inDetails);
//	printf("Returning the file detail as %d and inode number as %d\n ", currentFile, dEntry.inode_num);
	return currentFile;
}

//FS_SEEK
int fs_seek(int fd, int offset) {
	oft[fd].fileptr+=offset;	//updating the offset
	return fd;
}

//FS_READ
int fs_read(int fd, void *buf, int nbytes) {
	int blockCount,blockTotalSize;
	int readInode=oft[fd].de->inode_num;		//retrieve Inode form the fd 
	int ptr=oft[fd].fileptr;		//filepointer
	struct inode inDetails;
	
	fs_get_inode_by_num(0,readInode,&inDetails);	//Get the inode fromthe inode number
	
	if (nbytes>(inDetails.size-ptr)){
		nbytes=inDetails.size-ptr;
	}
	
	if(inDetails.size % MDEV_BLOCK_SIZE == 0){
		blockCount=inDetails.size/MDEV_BLOCK_SIZE;
	}
	else{
		blockCount=inDetails.size/MDEV_BLOCK_SIZE+1;
	}
	blockTotalSize=blockCount*MDEV_BLOCK_SIZE;
	char data[blockTotalSize];
	
	bs_bread(0,inDetails.blocks[0],0,data,blockTotalSize);
	memcpy(buf,data+ptr,nbytes-ptr);
	oft[fd].fileptr=nbytes;
	fs_put_inode_by_num(0,readInode,&inDetails);
	return nbytes;
}

//FS_WRITE
int fs_write(int fd, void *buf, int nbytes) {
	struct inode inDetails;
	struct fsystem fsDetails;
	char data[MDEV_BLOCK_SIZE];
	int firstFreeBlock=18;
	int blockNumber=0;
	int i;
	int size=0;
	int counter=0;
	int block_to_write;
	int inode_number=oft[fd].de->inode_num;
//	printf("writing into inode %d",inode_number);
	
	if(next_open_fd<fd){
		printf("Invalid File Descriptor");
		return SYSERR;
	}
	
	bs_bread(dev0, SB_BLK, 0, &fsDetails, sizeof(struct fsystem));
	fs_get_inode_by_num(0,inode_number,&inDetails);
	blockNumber=firstFreeBlock+((inode_number)*INODEBLOCKS);
	i=nbytes;
	while (i>=MDEV_BLOCK_SIZE){
		memcpy(data,buf+size,MDEV_BLOCK_SIZE);
		blockNumber=block_to_write+counter;
		fs_setmaskbit(blockNumber);
		bs_bwrite(0,blockNumber,0,data,MDEV_BLOCK_SIZE);
		size=size+MDEV_BLOCK_SIZE;
		inDetails.blocks[counter]=blockNumber;
		counter++;
		i=i-MDEV_BLOCK_SIZE;
	}
	memcpy(data,buf+size, MDEV_BLOCK_SIZE);
	blockNumber=block_to_write+counter;
	fs_setmaskbit(blockNumber);
	bs_bwrite(0,blockNumber,0,data,MDEV_BLOCK_SIZE);
	inDetails.blocks[counter]=blockNumber;
	oft[fd].fileptr=nbytes;
	inDetails.size=nbytes;
	fs_put_inode_by_num(0,inode_number,&inDetails);
	bs_bwrite(dev0, SB_BLK, 0, &fsDetails, sizeof(struct fsystem));
    return nbytes;
}

#endif /* FS */
