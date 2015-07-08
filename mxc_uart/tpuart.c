#define TRUE 1
#define FALSE 0

unsigned char rcv_buf[20];

unsigned char data_t;
int real_read;
int step = 0;
int data_read_len, data_len, check_nor;
int i, j;
bool check_flag = FALSE;


static void da_request_init(void);

memset(rcv_buf, 0, BUFFER_SIZE);


while (1){
		


		    /*read frame head from serial port*/
		   real_read = read(serial_fd, &data_t, 1);

		   if ((real_read <= 0) && (errno != EAGAIN))
		   	{

		         printf("\nzorro, ERROR:READ FRAME HEAD error1 real_read = %d, \n", real_read);

		    }

		    switch (step)
		    	{
		    	case 0x00:  //DA_DECODE_SYN_HEAD
				 	if (data_t == 0xAA)
					{
						rcv_buf[0] = data_t;
						data_read_len = 0;
						step++;
				 	}
				break;
				case 0x01: //DA_DECODE_GET_SYN_COUNTER
				      if (data_t == 0xAA)
					  	 da_request_init(); 
					  else
					  	{
                              rcv_buf[1] = data_t;
					          step++;

					    }
				         
					break;
				case 0x02: //DA_DECODE_GET_DATA_LENGTH
						
						if (data_t < 2)  
							
                         da_request_init(); 
                            
                       else
                       	{
                            data_read_len = data_t;
							data_len = data_read_len - 1;
						    rcv_buf[2] = data_read_len;
							step ++;
					    }
            
					break;
				case 0x03: //DA_DECODE_GET_DATA
				       
				     if (data_read_len < 17)
				     	{
                           rcv_buf[3+i++] = data_t;
					       if (i == data_len)
						   	{
						      step++;
							  
					        }
			
					    }
					 else
					 	
                        da_request_init(); 
											    
					 	 break;
					
				case 0x04: //DA_DECODE_CHECK
				
				check_nor = rcv_buf[0];
				for (j = 1; j < 3 + i; j++)
					{
			           check_nor ^= rcv_buf[j];
		            }
				if (check_nor == data_t)
					{
                      step = 0;
					  check_flag = TRUE;
                      
				    }
		
		        else
					
                     step = 0;
					 break;

				    
						
		    	}











switch(cmd)
{
case 34:
case 36:
case 38:



	
}




		   
		   static void da_request_init(void)
		   {
			   int index;
			   data_read_len = 0;  
			   
			   for (index = 0; index < 20; index++)  
			   {
				   rcv_buf[index] = 0;	
			   }
			   step = 0;  
			  
		   }




#define DA_COM_PROTOCOL_BASIC_SIZE 5 /* include: head(1) syn_frame_counter(1) length(1) message_id(1) and check(1) byte of frame */
#define DA_COM_HEAD_FLAG          (int)0xAAu


bool da_send_response(int message_id, const int datas[], int data_size)
{
    bool send_result;
    int dt_index = 0; /* send data index */
    int msg_dt_index;
    int check_xor;
    
    int send_data_array[16] = 
    {
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
    };
    /* response process */
    static int da_rsp_syn_counter = 0;

    /* check send buffer size is enough or not */
    if (data_size <= (int)(16 - DA_COM_PROTOCOL_BASIC_SIZE))
    {
        /* set head flag */
        send_data_array[dt_index] = DA_COM_HEAD_FLAG;
        dt_index++;
        /* set syn. frame counter(allways 0) */
        send_data_array[dt_index] = da_rsp_syn_counter;
        dt_index++;
        /* set data length ( = message_id + data_size + check_byte) */
        send_data_array[dt_index] = data_size + 2; /* can not overflow */
        dt_index++;
        /* set message id */
        send_data_array[dt_index] = message_id;
        dt_index++;
        /* intial check value */
        check_xor = ((DA_COM_HEAD_FLAG ^ da_rsp_syn_counter) ^ (data_size + 2)) ^ message_id; /* start at Length byte 0 */
        /* copy message data to send buffer */
        for (msg_dt_index = 0; msg_dt_index < data_size; msg_dt_index++)
        {
            send_data_array[dt_index] = datas[msg_dt_index];
            /* calculator new check value */
            check_xor = check_xor ^ datas[msg_dt_index];
            dt_index++;
        }
        /* set check value */
        send_data_array[dt_index] = check_xor;
        dt_index++;
        /* try to transmit infromation */
        send_result = sci_send(DA_COM_SCI_CHANNEL, send_data_array, dt_index);
        
    }
    else
    {
        /* send buffer is not enough */
        send_result = FALSE;
    }
    
    return send_result;
}


/************************************************
**************************************************/
#define DA_SERVICE_COUNTER 11
typedef void (*service_handle)(void);

typedef struct da_cmd_service
{
    const int msg_id;
    const int msg_length;
    service_handle handle_function;
}


int cmd_index;

   static const da_cmd_service da_service_configs[DA_SERVICE_COUNTER] = 
		   {
			   {0x30,	 2, &da_req_dvr_menu}, 
			   {0x32,    2, &da_req_dvr_status},
			   {0x34,	 2, &da_req_record_set}, /* no date transmit mode size is 2, with date is 6 */
			   {0x36,	 2, &da_req_vedio_list},
			   {0x38,    2, &da_req_replay_vedio},
			   {0x3A, 	 2, &da_req_vedio_control},
			   {0x3C,    2, &da_req_sys_infor},
			   {0x3E,	 2, &da_req_parameters},
			   {0x40,	 4, &da_req_set_para},
			   {0x42, 	 1, &da_req_emergency},
			   {0X44,	 2, &da_req_play_rt_vedio}
			  
		   };






		   static void da_req_dvr_menu(void)
		   	{

			
		   	}

		   static void da_req_dvr_status(void)
		   	{
		   	}

		   static void da_req_record_set(void)
		   	{
		   	}

		   static void da_req_vedio_list(void)
		   	{
		   	}

		   static void da_req_replay_vedio(void)
		   	{
		   	}

		   static void da_req_sys_infor(void)
		   	{
		   	}

		   static void da_req_vedio_control(void)
		   	{
		   	}

		   static void da_req_parameters(void)
		   	{
		   	}

		   static void da_req_set_para(void)
		   	{
		   	}

		   static void da_req_emergency(void)
		   	{
		   	}

		   static void da_req_play_rt_vedio(void)
		   	{
		   	}




/***************************/

static int da_get_service_index(int service_id)
{
    int service_index = 0;
    int match_index = 0xFF;

    for (; service_index < DA_SERVICE_COUNTER; service_index++)
    {
        if (da_service_configs[service_index].msg_id == service_id)
        {
            match_index = service_index;
            break; /* exit for */
        }
        else
        {
            /* match MISRA-C */
        }
    }
    return match_index;
}






