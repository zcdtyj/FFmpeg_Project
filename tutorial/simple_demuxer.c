#include <stdio.h>
#include <libavformat/avformat.h>

#define USE_H264BSF 0

int main(int argc, char *argv[]) 
{
    AVOutputFormat *ofmt_a = NULL, *ofmt_v = NULL;
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx_a = NULL, *ofmt_ctx_v = NULL;
    AVPacket pkt;
    int ret, i;
    int videoindex = -1, audioindex = -1;
    int frame_index = 0;

    const char *in_filename = "../resource/test.mp4";
    const char *out_filename_v = "../resource/test_video.h264";
    const char *out_filename_a = "../resource/test_video.aac";

    av_register_all();
    if((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        printf("Could not open input file.\n");
        goto end;
    }    

    if((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        printf("Failed to retrieve input stream information\n");
        goto end;
    }

    avformat_alloc_output_context2(&ofmt_ctx_v, NULL, NULL, out_filename_v);
    if(!ofmt_ctx_v) {
        printf("Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    ofmt_v = ofmt_ctx_v->oformat;

    avformat_alloc_output_context2(&ofmt_ctx_a, NULL, NULL, out_filename_a);
    if(!ofmt_ctx_a) {
        printf("Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    ofmt_a = ofmt_ctx_a->oformat;

    for(i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVFormatContext *ofmt_ctx;
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVStream *out_stream = NULL;

        if(ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            out_stream = avformat_new_stream(ofmt_ctx_v, in_stream->codec->codec);
            ofmt_ctx = ofmt_ctx_v;
        } else if(ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioindex = i;
            out_stream = avformat_new_stream(ofmt_ctx_a, in_stream->codec->codec);
            ofmt_ctx = ofmt_ctx_a;
        } else {
            break;
        }

        if(!out_stream) {
            printf("Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        if(avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
            printf("Failed to copy context from input to output.\n");
            goto end;
        }

        out_stream->codec->codec_tag = 0;

        /* Some formats want stream headers to be separate. mp4 need ts do not need*/
        if(ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER; 

        printf("\n==============Input Video============\n");
        av_dump_format(ifmt_ctx, 0, in_filename, 0);   
        printf("\n==============Output Video============\n");
        av_dump_format(ifmt_ctx_v, 0, out_filename, 1);   
        printf("\n==============Output Audio============\n");
        av_dump_format(ifmt_ctx_a, 0, out_filename, 1);
        printf("\n======================================\n");

        if(!(ofmt_v->flags & AVFMT_NOFILE)) {
            if(avio_open(&ofmt_ctx_v->pb, out_filename_v, AVIO_FLAG_WRITE) < 0) {
                printf("Could not open output file '%s'", out_filename_v);
                goto end;
            }
        }

        if(!(ofmt_a->flags & AVFMT_NOFILE)) {
            if(avio_open(&ofmt_ctx_a->pb, out_filename_a, AVIO_FLAG_WRITE) < 0) {
                printf("Could not open output file '%s'", out_filename_a);
                goto end;
            }
        }

         if(avformat_write_header(ofmt_ctx_v, NULL) < 0) {
             printf("Error occurred when opening video output file\n");
             goto end;
         }

         if(avformat_write_header(ofmt_ctx_a, NULL) < 0) {
             printf("Error occurred when opening audio output file\n");
             goto end;
         }   

    #if USE_HBSF
        AVBitStreamFilterContext *h264bsrc = av_bitstream_filter_init("h264_mp4toannexb");
    #endif

        while(1) {
            AVformatContext *ofmt_ctx;
            AVStream *in_stream, *out_stream;

            if(av_read_frame(ifmt_ctx, &pkt) < 0) {
                break;
            }    

            in_stream = ifmt_ctx->streams[pkt.stream_index];

            if(pkt.stream_index == videoindex) {
                out_stream = ofmt_ctx_v->streams[0];
                ofmt_ctx = ofmt_ctx_v;   
                printf("Write Video Packet. size:%d\tpts;%lld\n", pkt.size, pkt.pts);               
            }else if(pkt.stream_index== audioindex) {
                out_stream = ofmt_ctx_a->streams[0];
                ofmt_ctx = ofmt_ctx_a;
                printf("Write Audio Packet. size:%d\tpts:%lld\n", pkt.size, pkt.pts);                
            }else {
                continue;
            }

            pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_Round_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
            pkt.pos = -1;
            pkt.stream_index = 0;
            




        }    

}