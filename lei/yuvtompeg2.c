#include <stdio.h>
#include <string.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

int main(int argc, char *argv[]) {
    AVFormatContext *oc;
    AVOutputFormat *fmt;
    AVStream *video_st;
    double video_pts;
    uint8_t *video_outbuf;
    uint8_t *picture_buf;
    AVFrame *picture;
    int size;
    int ret;
    int video_outbuf_size;

    FILE *fin = fopen("../resource/test_yuv420p_320x180.yuv", "rb");
    const char *filename = "../resource/test.mpg";

    av_register_all();
    oc = avformat_alloc_context();
    fmt = av_guess_format(NULL, filename, NULL);
    oc->oformat = fmt;
    
    if(avio_open(&oc->pb, filename, AVIO_FLAG_READ_WRITE) < 0) 
    {
        return -1;
    }

    video_st = NULL;
    if(fmt->video_codec != AV_CODEC_ID_NONE) {
        AVCodecContext *c;
        video_st = avformat_new_stream(oc, NULL);
        c = video_st->codec;
        c->codec_id = fmt->video_codec;
        c->codec_type = AVMEDIA_TYPE_VIDEO;

        c->bit_rate = 1024000;
        
        c->width = 320;
        c->height = 180;

        c->pix_fmt = AV_PIX_FMT_YUV420P;

        c->time_base.num = 1;
        c->time_base.den = 25;

        c->gop_size = 12;

        if(c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
            c->max_b_frames = 2;
        }

        if(c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
            c->mb_decision = 2; 
        }

        // if(!strcmp(oc->oformat->name. "mp4") || !strcmp(oc->oformat->name, "mov") || !strcmp(oc->oformat, "3gp"))
        // {
        //     c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        // }
    }

    av_dump_format(oc, NULL, filename, 1);
    if(video_st) {
        AVCodecContext *c;
        AVCodec *codec;
        c = video_st->codec;
        codec = avcodec_find_encoder(c->codec_id);
        if(!codec) {
            return -1;
        }

        if(avcodec_open2(c, codec, NULL) < 0) {
            return -1;
        }

        if(!(oc->oformat->flags & AVFMT_RAWPICTURE))
        {
            video_outbuf_size = 200000;
            video_outbuf = (uint8_t *)av_malloc(video_outbuf_size);
        }

        picture = av_frame_allo();
        size = avpicture_get_size(c->pix_fmt, c->width, c->height);
        picture_buf = (uint8_t *)av_malloc(size);
        if(!picture_buf) {
            av_free(picture);
        }
        avpicture_fill((AVPicture *)picture, (const uint8_t *)picture_buf, c->pix_fmt, c->width, c->height);
    }
    
    avformat_write_header(oc, NULL);
    for(int i = 0; i < 300; i++) {
        if(video_st) {
            video_pts = (double)(video_st->pts.val * video_st->time_base.num / video_st->time_base.den);    
        } else {
            video_pts = 0.0;
        }

        if(!video_st) {
            break;
        }

        AVCodecContext *c;
        c = video_st->codec;
        size = c->width * c->height;

        if(fread(picture_buf, 1, size * 3 /2, fin) < 0) {
            break;
        } 

        picture->data[0] = picture_buf;
        picture->data[1] = picture_buf + size;
        picture->data[2] = picture_buf + size * 5 / 4;

        if(oc->oformat->flags & AVFMT_RAWPICTURE) {
            AVPacket pkt;
            av_init_packet(&pkt);
            pkt.flags |= AV_PKT_FLAG_KEY;
            pkt.stream_index = video_st->index;
            pkt.data = (uint8_t *)picture;
            pkt.size = sizeof(AVPicture);
            ret = av_write_frame(oc, &pkt);
        } else {
            int out_size = avcodec_encode_video2(c, video_outbuf, video_outbuf_size, picture);
            if(out_size > 0) {
                AVPacket pkt;
                av_init_packet(&pkt);
                pkt.pts = av_rescale_q(c->coded_frame->pts, c->time_base, video_st->time_base);
                if(c->coded_frame->key_frame) {
                    pkt.flags |= AV_PKT_FLAG_KEY;
                }
                pkt.stream_index = video_st->index;
                pkt.data = video_outbuf;
                pkt.size = out_size;
                ret = av_write_frame(oc, &pkt);
            } 
        }
    }

    if(video_st)
    {
        avcodec_close(video_st->codec);
        av_free(picture);
        av_free(video_outbuf);
    }
    av_write_trailer(oc);
    for(int i = 0; i < oc->nb_streams; i++) {
        av_freep(&oc->streams[i]->codec);
        av_freep(&oc->streams[i]);
    }
    av_free(oc);
}