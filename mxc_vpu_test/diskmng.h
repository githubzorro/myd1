/* 
 * comm.h
 * Authors: zorro
 *   Rickard E. (Rik) Faith <faith@redhat.com> 
 *  handle the key signal and uart cmd
 */  

#ifndef DISKMNG_H
#define DISKMNG_H

#include <time.h>


struct video_node{
	char video_a[50];
	char video_b[50];
	char video_c[50];
	char video_d[50];
	int locked;
	time_t st_mtime;
	struct video_node *next;
};

typedef struct video_node *link_list;


extern int enable_record;

int key_thread(void * arg);

 #endif 
