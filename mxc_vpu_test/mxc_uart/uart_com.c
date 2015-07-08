/************************************
  *file name: uart_com.c
  *description:Function of serial communication
  *author: zorro
  *created date: 06/17/2015
  ************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <sys/types.h>   /*Provide the definition of type pid_t*/
#include <sys/stat.h>    /*I/O operations*/
#include <fcntl.h>
#include <unistd.h>


/*private definition-------------------------------------*/
#define MAX_COM_NUM  20
#define GNR_COM 0
#define USB_COM 1
#define COM_TYPE GNR_COM

#define SEL_FILE_NUM   	2
#define BUFFER_SIZE	20	
#define TARGET_COM_PORT	4
#define RECV_FILE_NAME          "/home/hhwork/recv_file"
#define TIME_DELAY      100	/*select wait 3 seconds*/

/* The frame format is "start_byte,tx_id(tbd),cmd_len,cmd_id,data, check_sum"*/
#define FRAME_HEAD_SIZE 3       /*include start_byte,tx_id,cmd_len*/	
#define START_BYTE      0xAA


/*function declaration------------------------------------*/
static void cmd_unpack(unsigned char buff[], int *reply_cmd);
static int check_frame_head(unsigned char buff[], int *data_len);
static int check_frame_sum(unsigned char buff [],int data_len);

/*global variable----*/
static int cmd_to_dsp = 0;

/*
struct termios
{
	unsigned short c_iflag;		/* input mode flag
	unsigned short c_oflag;		/* output mode flag
	unsigned short c_cflag;		/* control mode flag
	unsigned short c_lflag;		/* Local mode flag
	unsigned char c_line;		/* Line discipline
	unsigned char c_cc[NCC];		/* control feature
	speed_t c_ispeed;			/* input speed
	speed_t c_ospeed;			/* output speed
}; */


/*======================================
*Function: set_com_config
*description:Setting up the serial port parameters 
*Author: zorro
*Created Date :6/18/2015
*======================================*/
int
set_com_config(int fd, int baud_rate, int data_bits, char parity, int stop_bits)
{
	struct termios options;
	int speed;

	/*Save and test the existing serial interface parameter Settings, here if the serial number and other errors,
	   There will be a relevant error message*/
	if (tcgetattr(fd, &options) != 0){
		perror("tcgetattr");
		printf("\n\n\nzorro, tcgetattr err: %s\n", strerror(errno));
		return -1;
	}

	/*set the character size*/
	cfmakeraw(&options); /*configured to the original model*/
	options.c_cflag &= ~CSIZE;

	/*set the baud rate*/
	switch (baud_rate){
		case 2400:
		{
			speed = B2400;
		}
		break;
		case 4800:
		{
			speed = B4800;
		}
		break;
		case 9600:
		{
			speed = B9600;
		}
		break;
		case 19200:
		{
			speed = B19200;
		}
		break;
		case 38400:
		{
			speed = B38400;
		}
		break;
		default:
		case 115200:
		{
			speed = B115200;
		}
		break;
	}
	cfsetispeed(&options, speed);
	cfsetospeed(&options, speed);

	/* set the stop bit */
	switch (data_bits){
		case 7:
		{
			options.c_cflag |= CS7;
		}
		break;

		default:
		case 8:
		{
			options.c_cflag |= CS8;
		}
		break;
	}

	/* Set the parity bit */
	switch (parity){
		default:
		case  'n':
		case  'N':
		{
			options.c_cflag &= ~PARENB;
			options.c_iflag &= ~INPCK;
		}
		break;

		case  'o':
		case  'O':
		{
			options.c_cflag |= (PARODD | PARENB);
			options.c_iflag |= INPCK;
		}
		break;

		case 'e':
		case 'E':
		{
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;
		}
		break;

		case 's': /*as no parity*/
		case 'S':
		{
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
		}
		break;			
	}

	switch (stop_bits){
		default:
		case  1:
		{
			options.c_cflag &= ~CSTOPB;
		}
		break;

		case 2:
		{
			options.c_cflag |= CSTOPB;
		}
		break;
	}

	/*Set the waiting time and minimum received characters*/
	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 1;

	/*Handle not receive characters, clean the receive or transmit buff*/
	tcflush(fd, TCIFLUSH);
	
	/*Activate the new configuration*/
	if ((tcsetattr(fd, TCSANOW, &options)) !=0){
		perror("tcsetattr");
		printf("\n\n\nzorro, tcsetattr err: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

/*======================================
*Function: open_port
*description:open a serial port
*Author: zorro
*Created Date :6/18/2015
*======================================*/
int
open_port(int com_port)
{
	int fd;
#if (COM_TYPE == GNR_COM) /*use ordinary serial port*/
	char *dev[] = {"/dev/ttyS0", "/dev/ttyS1","/dev/ttyS2","/dev/ttymxc2"};
#else				/*use USB  serial port*/
	char *dev[] = {"/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyUSB0"};
#endif

	if ((com_port < 0) || (com_port > MAX_COM_NUM)){
		return -1;
	}

	/* open the serial port*/
	fd = open(dev[com_port -1], O_RDWR|O_NOCTTY|O_NDELAY);
	if (fd < 0){
		perror("open serial port");
		printf("\n\n\nzorro, can't open %s: %s\n", dev[com_port -1],strerror(errno));
		return -1;
	}

	/*recover to blocking mode */
	if (fcntl(fd, F_SETFL, 0) < 0){
		perror("fcntl F_SETFL\n");
		printf("\n\n\nzorro, fcntl F_SETFL: %s\n", strerror(errno));
	}

	/*  whether the terminal equipment*/
	if (isatty(STDIN_FILENO) == 0){
		perror("standard input is not a terminal device");
		printf("\n\n\standard input is not a terminal device: %s\n", strerror(errno));
	}
	return fd;
}

/*======================================
*Function: cmd_unpack
*description:unpack the cmd frame
*Author: zorro
*Created Date :6/18/2015
*======================================*/   
static void
cmd_unpack(unsigned char buff[], int *preply_cmd)
{
	int cmd_id;

	cmd_id = buff[3];
	switch (cmd_id){
		case 0:
			printf("\nzorro, cmd id  = 0, \n");
			break;
		case 1:
			printf("\nzorro, cmd id  = 1, \n");
			break;
		case 2:
			printf("\nzorro, cmd id  = 2, \n");
			break;
		default:
			printf("\nzorro, cmd id  = %d, \n", cmd_id);

			break;
	}
}

/*======================================
*Function: check_frame_head
*description:check the frame head
*Author: zorro
*Created Date :6/18/2015
*======================================*/  
static int
check_frame_head(unsigned char buff[], int *pdata_len)
{
	int i,temp_len,err;

	temp_len = 0;

	if (*pdata_len < 0){
		err = -1;
	}
	else{
		//search the frame head
		for (i = 3; i > 0; i--){
			if (START_BYTE == buff[i-1]){	
			    	temp_len = (*pdata_len - (i-1));	
				if (i != 1){
					/*translation the byte After the new frame head to front of the buff */
					int copy_pos = 0;
					
					for (; i <=  *pdata_len; i++){
						buff[copy_pos] = buff[i -1];
						copy_pos++;
					}
				}
				break;  //find the head then break this loop
			}
		}
	
		*pdata_len = temp_len;

		err = 0;
	}
	
	return err;
	
}

/*======================================
*Function: check_frame_sum
*description: check the frame crc
*Author: zorro
*Created Date :6/18/2015
*======================================*/ 
static int
check_frame_sum(unsigned char buff [ ],int data_len)
{
	int err, i, xor_sum = 0;
	if (data_len < 0){
		err = -1;
	}
	else{
		for (i =0; i < (data_len - 1); i++){
			xor_sum ^= buff[i];
		}
		
		if (xor_sum == buff[data_len - 1]){
			err = 0;
		}
		else{
			err = 1;
		}
	}

	return err;
}


/*======================================
*Function: uart_com_thread
*description: this is communication thread with MP5
*Author: zorro
*Created Date :6/18/2015
*======================================*/
void
uart_com_thread(void)
{

	int serial_fd, recv_fd, maxfd;
	unsigned char buff[BUFFER_SIZE];
	fd_set inset, tmp_inset;
	struct timeval tv;
	unsigned loop = 1;
	int read_sum = 0, real_read, real_reread, i;
	int cmd_len;
	int reply_cmd;
	

	/*The serial port data written to this file*/
	if ((recv_fd = open(RECV_FILE_NAME, O_CREAT|O_WRONLY, 0644))  < 0){
		perror("open");
		printf("\n\n\nzorro, can't open recv_file: %s\n", strerror(errno));
		return 1;
	}
	
	printf("the recv_file fd = %d,\n", recv_fd);

	//fds[0] = STDIN_FILENO; /*The standard input*/

	if ((serial_fd = open_port(TARGET_COM_PORT)) < 0){
		perror("open_port");
		printf("\n\n\nzorro, can't open target com port: %s\n", strerror(errno));
		return 1;
	}

	printf("the serial_file fd = %d,\n", serial_fd);
	/*config com*/
	if (set_com_config(serial_fd, 115200, 8, 'N', 1) < 0){
		perror("set_com_config");
		printf("\n\n\nzorro, can't set com fonfig: %s\n", strerror(errno));
		return 1;
	}

	printf("Input some words(enter 'quit' to exit):\n");

	while (1){
		
		   memset(buff, 0, BUFFER_SIZE);

		   do
		   {
			    /*read frame head from serial port*/
			   real_read = read(serial_fd, &buff[read_sum], FRAME_HEAD_SIZE);

			   if ((real_read <= 0) && (errno != EAGAIN)){

			         printf("\nzorro, ERROR:READ FRAME HEAD error1 real_read = %d, \n", real_read);

			   }
				
		   	   read_sum += real_read;

			   if (read_sum >= FRAME_HEAD_SIZE)
			   {
			   	if (check_frame_head(buff, &read_sum) < 0)
				{	
					  printf("\nzorro, ERROR:check head  error read_sum = %d, \n", read_sum);
				}
			   }
		   }while(read_sum < FRAME_HEAD_SIZE);

		
	   	cmd_len = buff[2];

		do
		{
			   /*read frame data from serial port*/
		   	   real_read = read(serial_fd, &buff[read_sum], cmd_len);

			   if ((real_read <= 0) && (errno != EAGAIN)){
			         	printf("\nzorro, ERROR:READ FRAME DATA error2 real_read = %d, \n", real_read);
			   }
			   
			   read_sum += real_read;

			   if (read_sum == (cmd_len+FRAME_HEAD_SIZE)){
			   	/*check the frame*/
			   	if (check_frame_head(buff, &read_sum) < 0){	
				 	 printf("\nzorro, ERROR:check head  error read_sum = %d, \n", read_sum);
				}
			   }
		}while(read_sum < (cmd_len+FRAME_HEAD_SIZE));

		buff[read_sum] = '\0';
		/*write the data to commen file*/
		write(recv_fd, buff, read_sum);
	
		
		int check_result;
		check_result = check_frame_sum(buff, read_sum);
		read_sum = 0;	//recv one frame, reset the read byte cnt;
		if (!check_result){
			cmd_unpack(buff,&reply_cmd);
		}
		else if (check_result > 1){
			continue;
		}
		else{
			printf("\nzorro, ERROR:check sum  error check_result = %d, \n", check_result);
		}
			
	    	write(serial_fd, buff, strlen(buff));

		//if (i == 0)
		//{   /*write the data to seria port*/
		//	  write(fds[1], buff, strlen(buff));
		//	  printf("Input some words (enter 'quit' to exit) : \n");
		//}

		  /*if read the "quit' , then exit */
		if (strncmp(buff, "quit", 4) == 0){
			 close(recv_fd);
		  	 pthread_exit();
		}

	   }
		

	return 0;

}


/*------------end of file------------*/
