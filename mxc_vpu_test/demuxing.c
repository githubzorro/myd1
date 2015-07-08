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
        const char *fileName = "test.mp4"; 
        err = av_open_input_file(&pFormatCtx, fileName, NULL, 0, NULL); 
     
        AVPacket packet; 
        av_init_packet(&packet); 
        int ret = av_read_frame(pFormatCtx, &packet); 
        if(ret >= 0) 
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
            bool isKey = ((packet.flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY);     
        } 
        av_free_packet(&packet);         
} 

