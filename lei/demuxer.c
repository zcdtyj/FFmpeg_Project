#include <stdio.h>
#include <libavformat/avformat.h>

#define USF_H264BSF 0

int main(int argc, char *argv[])
{
    AVOutputFormat *ofmt_a = NULL, *ofmt_v =NULL;
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx_a = NULL, *ofmt_ctx_v = NULL;

    AVPacket pkt;
    int ret, i;
    int videoIndex = -1, audioIndex = -1;
    int frame_index = 0;

    const char *in_filename = "test.flv";
    const char *out_filename_v = "test.h264";
    const char *out_filename_a = "test.aac";

    av_register_all();

    if((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        printf("Could not open input file.");
        goto end;
    }
    
    if((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        printf("Failed to retrieve inputstream information");
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
        printf("Could not create output context");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    ofmt_a = ofmt_ctx_a->oformat;

    for(i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVFormatContext *ofmt_ctx;
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVStream *out_stream = NULL;

        if(ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
            out_stream = avformat_new_stream(ofmt_ctx_v, in_stream->codec->codec);
            ofmt_ctx = ofmt_ctx_v;
        } else if(ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioIndex = i;
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
           printf("Failed to copy context from input to output stream codec context\n");
           goto end; 
        } 
        out_stream->codec->codec_tag = 0;
        if(ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

        //Dump Format
        printf("\n=====================Input Video=====================\n");
        av_dump_format(ifmt_ctx, 0, in_filename, 0);
        printf("\n=====================Input Video=====================\n");
        av_dump_format(ofmt_ctx_v, 0, out_filename_v, 1);
        printf("\n=====================Input Audio=====================\n");
        av_dump_format(ofmt_ctx_a, 0, out_filename_a, 1);
        printf("\n=====================================================\n");

        if(!(ofmt_v->flags & AVFMT_NOFILE)) {
            if(avio_open(&ofmt_ctx_v->pb, out_filename_v, AVIO_FLAG_WRITE) < 0) {
                printf("Could not open file '%s'", out_filename_v);
                goto end;    
            }
        }

        if(!(ofmt_a->flags & AVFMT_NOFILE)) {
            if(avio_open(&ofmt_ctx_a->pb, out_filename_a, AVIO_FLAG_WRITE) < 0) {
               printf("Could notopen output file '%s'", out_filename_a);
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

#if USE_H264BSF
        AVBitStreamFilterContext *h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
#endif
        while(1) {
            AVFormatContext *ofmt_ctx;
            AVStream *in_stream, *out_stream;

            if(av_read_frame(ifmt_ctx, &pkt) < 0)
                break;
            in_stream = ifmt_ctx->streams[pkt.stream_index];

            if(pkt.stream_index == videoIndex) {
                out_stream = ofmt_ctx_v->stream[0];
                ofmt_ctx = ofmt_ctx_v;
                printf("Write video packet. size:%d\t pts:%lld\n", pkt.size, pkt.pts);
#if USE_H264BSF
                av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);                
#endif                
            } else if(pkt.stream_index == audioIndex) {
                out_stream = ofmt_ctx_a->streams[1];
                ofmt_ctx = ofmt_ctx_a;
                printf("Write Audio Packet. size:%d\t pts:%lld\n", pkt.size, pkt.data);
            } else {
                continue;
            }

            pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));            
            pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
            pkt.pos = -1;
            pkt.stream_index = 0;

            if(av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
                printf("Error muxing packet\n");
                break;
            }

            av_free_packet(&pkt);
            frame_index++;            
        }
#if USE_H264BSF
    av_bitstream_filter_close(h264sffc);
#endif 
        av_write_trailer(ofmt_ctx_a);
        av_write_trailer(ofmt_ctx_v);

end:
        avformat_close_input(&ifmt_ctx);
        if(ofmt_ctx_a && !(ofmt_a->flags & AVFMT_NOFILE)) {
            avio_close(ofmt_ctx_a->pb);
        }
        if(ofmt_ctx_v && !(ofmt_a->flags & AVFMT_NOFILE)) {
            avio_close(ofmt_ctx_v->pb);
        }

        avformat_free_context(ofmt_ctx_a);
        avformat_free_context(ofmt_ctx_v);

        if(ret < 0 && ret != AVERROR_EOF) {
            printf("Error occurred.\n");
            return -1;
        }
        return 0;
    }


}