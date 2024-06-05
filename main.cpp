#include <cstdint>
#include <stdio.h>
extern "C"
{
    #include <libavcodec/codec.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include "libavcodec/codec_par.h"
    #include "libavformat/avio.h"
    #include "libavutil/avutil.h"
    #include "libavutil/dict.h"
    #include "libavutil/pixfmt.h"
    #include <libavutil/imgutils.h>
    #include "libavcodec/packet.h"
    #include "libavutil/frame.h"
    #include "libavutil/mem.h"
    #include "libavutil/opt.h"
}

int flush_encoder(AVFormatContext* fmt_context,unsigned int stream_index)
{
    // int ret;
    // int got_frame;
    // AVPacket enc_pkt;
    
    // // if(fmt_context->streams[stream_index]->codecpar)
    // while (1) 
    // {
    //     enc_pkt.data = NULL;
    //     enc_pkt.size = 0;

    //     av_init_packet(&enc_pkt);
    //     ret = avcodec_encode_video2()
    // }

}

int main()
{
    AVFormatContext *avformat_context = avformat_alloc_context();
    // char filepath[20] = {0};
    const char* filepath = "output.h264";
    const AVOutputFormat *avoutput_format = av_guess_format(NULL,filepath ,NULL);
    avformat_context->oformat = avoutput_format;

    if(avio_open(&avformat_context->pb, filepath, AVIO_FLAG_WRITE) < 0)
    {
        return -1;
    }
    
    AVStream *av_video_stream = avformat_new_stream(avformat_context, NULL);
    const AVCodec *av_codec = avcodec_find_encoder(avformat_context->oformat->video_codec);
    // const AVCodec *av_codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if(av_codec == NULL)
    {
        printf("did not find encoder\n");
        return -1;
    }

    printf("encoder name: %s\n",av_codec->name);

    AVCodecContext *codec_ctx = avcodec_alloc_context3(av_codec);
    codec_ctx->codec_id = avformat_context->oformat->video_codec;
    codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    codec_ctx->width = 640;
    codec_ctx->height = 360;
    codec_ctx->time_base = {1,25};

    codec_ctx->bit_rate = 468000;
    codec_ctx->gop_size = 12;
    // codec_ctx->qmax = 51;
    // codec_ctx->qmin = 10;
    // codec_ctx->max_b_frames = 0;

    AVDictionary *param = NULL;
    if(codec_ctx->codec_id == AV_CODEC_ID_H264)
    {
        printf("add param\n");
        av_dict_set(&param, "preset", "slow", 0);
        av_dict_set(&param, "tune", "zerolatency", 0);
    }
    // av_opt_set(codec_ctx->priv_data,"tune","zerolatency",0);
    int open_result = avcodec_open2(codec_ctx,av_codec,&param);
    printf("open_result: %d\n",open_result);
    
    int ret =  avcodec_parameters_from_context( av_video_stream->codecpar, codec_ctx);
    printf("ret: %d\n",ret);

    if(avcodec_is_open(codec_ctx) > 0)
    {
        printf("open ..................\n");
    }

    if(av_codec_is_encoder(codec_ctx->codec) > 0)
    {
        printf("encoder ..................\n");
    }

    if (!avcodec_is_open(codec_ctx) || !av_codec_is_encoder(codec_ctx->codec))
    {
        printf("11111111111111111 ..................\n");
    }else
    {
        printf("222222222222222222 ..................\n");
    }
    
    int flag = avformat_write_header(avformat_context,NULL);
    int buffer_size = av_image_get_buffer_size(codec_ctx->pix_fmt,
                                                codec_ctx->width,
                                                codec_ctx->height,
                                                1);

    int y_size = codec_ctx->width * codec_ctx->height;
    uint8_t *out_buffer = (uint8_t*)av_malloc(buffer_size);
    FILE *infile = fopen("./0.yuv","rb");
    if(!infile)
    {
        printf("open file not exist\n");
        return -1;
    }

    AVFrame *av_frame = av_frame_alloc();
    av_image_fill_arrays(av_frame->data,
                        av_frame->linesize,
                        out_buffer, 
                        codec_ctx->pix_fmt, 
                        codec_ctx->width, 
                        codec_ctx->height,
                        0);
    
    AVPacket *av_packet = (AVPacket*)av_malloc(buffer_size);

    int i = 0;
    int result = 0;
    int current_frame_index = 0;
    

    while (true) 
    {
        if(fread(out_buffer, 1 , y_size*3/2, infile) <= 0)
        {
            printf("fread over\n");
            break;
        }
        printf("start encode\n");

        av_frame->data[0] = out_buffer;
        av_frame->data[1] = out_buffer+y_size;
        av_frame->data[2] = out_buffer+y_size*5/4;

        av_frame->pts = i;
        i++;
        
  
        result = avcodec_send_frame(codec_ctx,av_frame);
        if(result == AVERROR(EINVAL))
        {
            printf("encoder not find\n");
        }
        printf("avcodec_send_frame result: %d\n",result);
        result = avcodec_receive_packet(codec_ctx,  av_packet);
        printf("result: %d\n",result);
        if(result == 0)
        {
            av_packet->stream_index = av_video_stream->index;
            result = av_write_frame(avformat_context,av_packet);
            printf("write to output context over\n");
        }

    }

   av_write_trailer(avformat_context);

   avcodec_close(codec_ctx);
   av_free(av_frame);
   av_free(out_buffer);
   av_packet_free(&av_packet);
   avio_close(avformat_context->pb);
   avformat_free_context(avformat_context);
   fclose(infile);

    return 0;
}