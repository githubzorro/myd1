/* 
 * comm.c
 * Authors: zorro, modify 
 *   Rickard E. (Rik) Faith <faith@redhat.com> 
 *  handle the key signal and uart cmd
 */ 
  
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <linux/input.h>
#include <dirent.h> 	      /*for opendir&readdir*/
#include <sys/statfs.h>   /*for statfs */

#define DEV_PATH  "/dev/input/event0"

#define VIDEOA "va"
#define VIDEOB "vb"
#define VIDEOC "vc"
#define VIDEOD "vd"

#define ONEMIN 60
#define MINSPACE 80


static struct input_event event;

int enable_record = 0;

struct link_list list_head = NULL;



/*==============insert_node================
*Function: insert a node before the pos position
*Author: zorro
*Date :6/3/2015
*Note: Not used in this application
*======================================*/
int
insert_node(link_list head,int pos,char *name, int lock, time_t time)  
{  
  
    int j=0;  
    link_list p = head;  
    link_list s = (link_list)malloc(sizeof(video_node));  


    //printf("\nzorro,insert a file node %s before %d\n",name,pos);  
    while(p&&j<pos-1){
        p=p->next;
        ++j;
    }
    if(!p||j>pos-1)
	return -1;  
  
    strcpy(s->video_a, name);
    s->locked = lock;
    s->st_mtime = time;
    s->next=p->next;  
    p->next=s;  
    return 0;  
}

/*===============clear_list================
*Function: clear a list, set the list to NULL 
*Author: zorro
*Date :6/3/2015
*Note! not used in this application
*======================================*/
int
clear_list(link_list head)
{
    link_list p,q;

    if (head == NULL){
	return -1;
    }
	
    p=head->next;           /*  p point to the first node */

    while(p){
    	q=p->next;
	free(p);
	p=q;
    }

    head->next=NULL;        /*set the first node NULL*/

    return 0;
}

/*==============search_list================
*Function: search the list,get the node with the same time 
*Author: zorro
*Date :6/3/2015
*======================================*/
link_list
search_list(link_list head, time_t time)
{

	link_list p=head;

	while(p!=NULL){
		if(p->st_mtime >= time){
			if ((p->st_mtime - time) <= ONEMIN)
				return p;
		}else{
			if ((time - p->st_mtime) <= ONEMIN)
				return p;
		}
		p=p->next;
	}

	return NULL;

}



/*===============create_list================
*Function: create a list and add node one by one 
*Author: zorro
*Date :6/3/2015
*Note! four videos will be locked or unlocked together
*======================================*/
int
create_list(link_list head, char *video_name, int lock, time_t time)
{
	link_list p = head, s;
	
	/*the node of this time is already created*/
	if ((s = search_list(link_list head,time_t time) == NULL){
		s = (link_list)malloc(sizeof(video_node));
		memset(s, 0, sizeof (video_node));
		s->st_mtime = time;
		s->locked &= lock;     /*Note! zorro*/
		while(p!=NULL)  {  
	    	    p=p->next;  
		}
		p=s;
		s->next = NULL;
	}
	
	if (strstr(video_name,VIDEOA) != NULL){
		strcpy(s->video_a, video_name);
	}
	else if (strstr(video_name,VIDEOB) != NULL){
		strcpy(s->video_b, video_name);
	}
	else if (strstr(video_name,VIDEOC) != NULL){
		strcpy(s->video_c, video_name);
	}
	else if (strstr(video_name,VIDEOD) != NULL){
		strcpy(s->video_d, video_name);

	}

	return 0;
}

/*==============traverse_list================
*Function: traverse the list, find the earliest video node
*Author: zorro
*Date :6/3/2015
*======================================*/
link_list
traverse_list(link_list head)
{	
	time_t time = 0;
	link_list p=head, q=NULL;
	while(p!=NULL){
		
		if ((time < p->st_mtime)&&(p->locked == 0)){
			time = p->st_mtime;
			q=p;
		}
	    p=p->next;  
	}  

	return q;
}

/*===============delete_node================
*Function: Delete the N node from the list 
*Author: zorro
*Date :6/3/2015
*======================================*/  
int
delete_node(link_list head, link_list d_node)
{  
    link_list p=head;  
	
    while (p){
        if (p->next == d_node)
	    break;
        p=p->next;  
    }  
    if (p == NULL)  
        return -1;
	
    p->next = d_node->next;   
    //printf("\nzorro,the file node deleted:%s\n",q->name);  
    free(p);  
    return 0;  
} 

/*===============readFileList================
*Function: read file list in basePath
*Author: zorro
*Date :6/2/2015
*======================================*/
int
readFileList(char *basePath)
{
    DIR *dir;
    struct dirent *ptr;
    char base[1000];

    if ((dir=opendir(basePath)) == NULL)
    {
        perror("Open dir error...");
        return -1;
    }

    while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
            continue;
        else if(ptr->d_type == 8){      ///file
            create_list(list_head, ptr->d_name, 0, 0);
	 printf("d_name:%s/%s\n",basePath,ptr->d_name);
        }
        else if(ptr->d_type == 10)    ///link file
            printf("d_name:%s/%s\n",basePath,ptr->d_name);
        else if(ptr->d_type == 4)      ///dir
        {
            memset(base,'\0',sizeof(base));
            strcpy(base,basePath);
            strcat(base,"/");
            strcat(base,ptr->d_name);
            readFileList(base);
        }
    }
    closedir(dir);
    return 0;
}


/*===============disk_ckeck================
*Function: check the sd card space
*Author: zorro
*Date :6/3/2015
*======================================*/
static int
disk_ckeck(int *available)
{
    struct statfs diskInfo;  
    if (statfs("/home/carl/", &diskInfo) == -1){
	return -1;
    }

    unsigned long long blocksize = diskInfo.f_bsize;    //每个block里包含的字节数  
    unsigned long long totalsize = blocksize * diskInfo.f_blocks;   //总的字节数，f_blocks为block的数目  
    printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",   
        totalsize, totalsize>>10, totalsize>>20, totalsize>>30);  
      
    unsigned long long freeDisk = diskInfo.f_bfree * blocksize; //剩余空间的大小  
    unsigned long long availableDisk = diskInfo.f_bavail * blocksize;   //可用空间大小  
    printf("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n",   
        freeDisk>>20, freeDisk>>30, availableDisk>>20, availableDisk>>30); 

    *available = (int)(availableDisk>>20);

    return 0;
}

/*===============disk_manage================
*Function: manage the sd card space,To determine whether 
		there is enough space,remove the earliest file to free
		enough space.
*Author: zorro
*Date :6/3/2015
*======================================*/
int
disk_manage(void)
{
    int available = 0;
    link_list p;

    if (disk_ckeck(&available) == -1){
	printf("\nzorro, cant open the  disk!!\n\n");
	return -1;
    }

	/*for example Recording 5 minutes of video, every video is size of  20MB*/
    if (available < MINSPACE){
	p = traverse_list(list_head);
	if (p!=NULL){
		if (strstr(p->video_a,VIDEOA) != NULL){
			if (remove(p->video_a) = -1){
				perror("remove video_a: ");
			}
		}
		if (strstr(p->video_b,VIDEOB) != NULL){
			if (remove(p->video_b) = -1){
				perror("remove video_b: ");
			}
		}
		if (strstr(p->video_c,VIDEOC) != NULL){
			if (remove(p->video_c) = -1){
				perror("remove video_c: ");
			}
		}
		if (strstr(p->video_d,VIDEOD) != NULL){
			if (remove(p->video_d) = -1){
				perror("remove video_d: ");
			}
		}
			
		if (delete_node(list_head, p) == -1)
			printf("\nzorro, remove node err!!\n\n");;

	}else{
		printf("\nzorro, no space can be released!!\n\n");
		return -1;
	}
    }

    return 0;
}

/*===============key_thread================
*Function: read the key button,and set the enable record flag
*Author: zorro
*Date :6/1/2015
*======================================*/

int
key_thread(void *arg)
{
	int i, rc;
	int fd = 0;
	char basePath[1000];


	/*get the current absoulte path */
	memset(basePath,'\0',sizeof(basePath));
	getcwd(basePath, 999);
	printf("the current dir is : %s\n",basePath);

	/*get the video list Under the directory "/encvideo"*/
	memset(basePath,'\0',sizeof(basePath));
	strcpy(basePath,"/encvideo");

	readFileList(basePath);

	/*open the /dev/input/event of key*/  
	if ((fd = open(DEV_PATH, O_RDONLY, 0)) < 0) {
	          printf("\nzorro  %s: open failed, fd = %d\n", DEV_PATH, fd);
		return 0;
	}
         /*read the key event*/  
        while ((rc = read(fd, &event, sizeof(event))) > 0) {
              printf("\n zorro %-24.24s.%06lu type 0x%04x; code 0x%04x;"
			" value 0x%08x; \n", 
                      	ctime(&event.time.tv_sec),
                       	event.time.tv_usec,
                       	event.type, event.code, event.value);
                if (event.type == EV_KEY) { 
                    if (event.code > BTN_MISC) {
                        printf("Button %d %s", 
                               event.code & 0xff,
                               event.value ? "press" : "release");
                    } else {
                              printf("Key %d (0x%x) %s",
                               event.code & 0xff,
                               event.code & 0xff,
                               event.value ? "press" : "release");

			if ((event.code & 0xff) == 0x0073)
		          enable_record += (int)event.value;	/*Even number represents Enable, odd number Disable*/
                    }
                }
            }
	printf("rc = %d, (%s)\n", rc, strerror(errno));
	close(fd);

	return 0;
}
