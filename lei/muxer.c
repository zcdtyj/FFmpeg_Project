#include <stdio.h>
#include <libavformat/avformat.h>

#define USE_H264BSF 0
#define USE_AACBSF 0

int main(int argc, char *argv[]) 
{
    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ifmt_ctx_v = NULL, *ifmt_ctx_a = NULL,  *ofmt_ctx = NULL;
    AVPacket pkt;
    int ret, i;
    int videoIndex_v = -1, videoIndex_out = -1;
    int audioIndex_a = -1, audioIndex_out = -1;
    int frame_index = 0;
    int64_t cur_pts_v = 0, cur_pts_a = 0;

    const char *in_filename_v = "test.h264";
    const char *in_filename_a = "test.mp3";
    const char *out_filename = "test.mp4";

    av_register_all();

    if((ret = avformat_open_input(&ifmt_ctx_v, in_filename_v, 0, 0)) < 0 ) {
        printf("Could not open file.");
        goto end;
    }

    if((ret = avformat_avformat_find_stream_info(ifmt_ctx_v, 0)) < 0) {
        printf("Failed to retrieve input stream information");
        goto end;
    }

    if((ret = avformat_open_input(&ifmt_ctx_a, in_filename_a, 0, 0)) < 0) {
        printf("Could not open file.");
        goto end; 
    }


    if((ret = avformat_avformat_find_stream_info(ifmt_ctx_a, 0)) < 0) {
        printf("Failed to retrieve input stream information");
        goto end;
    }
    printf("===================Input information==================\n");
    av_dump_format(ifmt_ctx_v, 0, in_filename_v, 0);
    av_dump_format(ifmt_ctx_a, 0, in_filename_a, 0);
    printf("======================================================\n");

    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if(!ofmt_ctx) {
        printf("Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    ofmt = ofmt_ctx->oformat;

    for(i = 0; i < ifmt_ctx_v->nb_streams; i++) {
        if(ifmt_ctx_v->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            AVStream *in_stream = ifmt_ctx_v->streams[i];
            AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec);
            videoIndex_v = i;
            if(!out_stream) {
                printf("Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
                goto end;
            }
            videoIndex_out = out_stream->index;
            if(avcodec_copy_context(out_stream->codec, in_stream->codec)) {
                printf("Failed to copy context from input to output\n");
                goto end;
            }
            out_stream->codec->codec_tag = 0;
            if(ofmt_ctx->ofomat->flags & AVFMT_GLOBALHEADER) {
                out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
                break;    
            }
        }
    }

    for(i = 0; i < ifmt_ctx_a->nb_streams; i++) {
        if(ifmt_ctx_a->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            AVStream *in_stream = ifmt_ctx_a->streams[i];
            AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec);
            audioIndex_a = i;
            if(!out_stream) {
                printf("Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
                goto end;
            }
            audioIndex_out = out_stream->index;
            if(avcodec_copy_context(out_stream->codec, in_stream->codec)) {
                printf("Failed to copy context from input to output\n");
                goto end;
            }
            out_stream->codec->codec_tag = 0;
            if(ofmt_ctx->ofomat->flags & AVFMT_GLOBALHEADER) {
                out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
                break;    
            }
        }
    }
    
    printf("==================Output Information===================\n");
    av_dump_format(ofmt_ctx, 0, out_filename, 1);
    printf("=======================================================\n");
    if(!(ofmt->flags & AVFMT_NOFILE)) {
        if(avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE)) {
            printf("Could not open output file");
            goto end;
        }
    }

    if(avformat_write_header(ofmt_ctx, NULL) < 0) {
        printf("Error occurred when opening out file\n");
        goto end;
    }

#if USE_H264BSF
    AVBitStreamFilterContext *h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
#endif

#if USE_AACBSF
    AVBitStreamFilterContext *aacbsfc = av_bitstream_filter_init("aac_adtstoasc");
#endif

    while(1) {
        AVFormatContext *ifmt_ctx;
        int stream_index = 0;
        AVStream *in_stream, *out_stream;

        if(av_compare_ts(cur_pts_v, ifmt_ctx_v->streams[videoIndex_v]->time_base, cur_pts_a, ifmt_ctx_a->streams[audioIndex_a]->time_base) <= 0) {
            ifmt_ctx = ifmt_ctx_v;
            stream_index = videoIndex_out;

            if(av_read_frame(ifmt_ctx, &pkt) >= 0) {
                do {
                    in_stream = ifmt_ctx->streams[pkt.stream_index];
                    out_stream = ofmt_ctx->streams[stream_index];

                    if(pkt.stream_index == videoIndex_v) {
                        if(pkt.pts == AV_NOPTS_VALUE) {
                            AVRational time_base1 = in_stream->time_base;                            
                            int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(in_stream->r_frame_rate);
                            pkt.pts=(double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
							pkt.dts=pkt.pts;
							pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
							frame_index++;
                        }

                        cur_pts_v = pkt.pts;
                        break;
                    }
                }while(av_read_frame(ifmt_ctx, &pkt) >= 0);
            }else {
                break;
            }
        } else {
            ifmt_ctx = ifmt_ctx_a;
            stream_index = audioIndex_out;

            if(av_read_frame(ifmt_ctx, &pkt) >= 0) {
                do {
                    in_stream = ifmt_ctx->streams[pkt.stream_index];
                    out_stream = ofmt_ctx->streams[stream_index];

                    if(pkt.stream_index == audioIndex_a) {
                        if(pkt.pts == AV_NOPTS_VALUE) {
                            AVRational time_base1=in_stream->time_base;
							//Duration between 2 frames (us)
							int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(in_stream->r_frame_rate);
							//Parameters
							pkt.pts=(double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
							pkt.dts=pkt.pts;
							pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
							frame_index++;
                        }
                        cur_pts_a = pkt.pts;
                        break;
                    }
                } while(av_read_frame(ifmt_ctx, &pkt) >= 0);
            }else {
                break;
            }
        }
#if USE_H264BSF
        av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif        

#if USE_AACBSF
        av_bitstream_filter_filter(aacbsfc, out_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif        
    }
}