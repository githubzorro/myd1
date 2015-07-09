#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>		/*for open write*/
#include <unistd.h>

 
 
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif
 
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"


 int /* write n bytes to a file descriptor */
 fwriten(int fd, void *vptr, size_t n)
 {
	 int nleft;
	 int nwrite;
	 char  *ptr;
 
	 ptr = vptr;
	 nleft = n;
	 while (nleft > 0) {
		 if ( (nwrite = write(fd, ptr, nleft)) <= 0) {
			 perror("fwrite: ");
			 return (-1);			 /* error */
		 }
 
		 nleft -= nwrite;
		 ptr   += nwrite;
	 }
 
	 return (n);
 } /* end fwriten */


 /*==============fwrite_thread====================
 *Function: demuxing the raw data frome mp4 file
 *Author: zorro
 *Date :7/8/2015
 *======================================*/
 int
 main(int argc, char** argv)
 { 
      
	av_register_all(); 
	AVFormatContext * pFormatCtx = NULL; 
	int err = 0;
	int ret;
	int sps_len;
	const char *dst_name = "demux.h264";
	const char *fileName = "output002.mp4";
	int dst_fd;
	char nal_head[4] = {0x00, 0x00, 0x00, 0x01};


	dst_fd = open(dst_name, O_CREAT | O_RDWR | O_TRUNC,
				S_IRWXU | S_IRWXG | S_IRWXO);
	if (dst_fd < 0) {
		perror("file open");
		printf("\n\n\nzorro, 07/09/2015, open file %s failed\n",dst_name);
		return -1;
	}
	
	av_register_all(); 
	if (av_open_input_file(&pFormatCtx, fileName, NULL, 0, NULL) < 0){
		printf("Open Input Error!\n");  
           	return -1;
	} 

	
	AVPacket packet; 
	av_init_packet(&packet); 
	while (av_read_frame(pFormatCtx, &packet) >= 0)
	{
		int streamIndex = packet.stream_index; 
		AVStream *pStream = pFormatCtx->streams[streamIndex]; 
		AVCodecContext *pCodecCtx = pStream->codec; 
		// 计算timestamp 

		// 转换时间到1/1000000秒 
		AVRational time_base; 
		time_base.num = 1; 
		time_base.den = 1000000; 

		// 25.0     1/25,   29.97    1001/30000 

		// 获取 dts/pts 
		const long long int dts = av_rescale_q(packet.dts, pStream->time_base, time_base); 
		const long long int pts = av_rescale_q(packet.pts, pStream->time_base, time_base); 
		unsigned char *data = packet.data; 
		int size = packet.size; 
		int Keyframe = ((packet.flags & PKT_FLAG_KEY) == PKT_FLAG_KEY);

		//strncpy(data, nal_head, 4);
		/*sps head-----*/
		if ((data[4] & 0x1f) == 0x07){
			sps_len = data[3];

			/*pps head-----*/
			data[sps_len+4] = 0;
			data[sps_len+5] = 0;
			data[sps_len+6] = 0;
			data[sps_len+7] = 0x01;
		}
		data[0] = 0;
		data[1] = 0;
		data[2] = 0;
		data[3] = 0x01;
		ret = fwriten(dst_fd, data, size);
		if (ret < 0){
			printf("\n\nzorro,write frame data Error\n");
			break;
		}

		av_free_packet(&packet);
	}
	av_free_packet(&packet);
	printf("\n\nzorro, 07/09/2015, demuxing end\n");
} 

