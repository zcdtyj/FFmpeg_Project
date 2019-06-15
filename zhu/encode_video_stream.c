#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>

#define TEST_H264 0
#define TEST_HEVC 1

int main(int argc, char *argv[]) 
{
    AVCodec *pCodec;
    AVCodecContext *pCodecCtx = NULL;
    int i, ret, got_output;
    FILE *fp_in;
    FILE *fp_out;
    AVFrame *pFrame;
    AVPacket pkt;
    int y_size;
    int framecnt = 0;

    char filename_in[] = "../resource/test_yuv420p_320x180.yuv";

#if TEST_HEVC
    enum AVCodecID codec_id = AV_CODEC_ID_HEVC;
    char filename_out[] = "../resource/test.hevc";
#else
    enum AVCodecID codec_id = AV_CODEC_ID_H264;
    char filename_out[] = "../resource/test.h264";
#endif

    int in_w = 320, in_h = 180;
    int framenum = 100;

    avcodec_register_all();
    pCodec = avcodec_find_encoder(codec_id);
    if(!pCodec) {
        printf("Codec not found\n");
        return -1;
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    if(!pCodecCtx) {
        printf("Could not allocate video codec context!\n");
        return -1;
    }

    pCodecCtx->bit_rate = 400000;
    pCodecCtx->width = in_w;
    pCodecCtx->height = in_h;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;
    pCodecCtx->gop_size = 10;
    pCodecCtx->max_b_frames = 1;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->codec_id = codec_id;

    if(codec_id == AV_CODEC_ID_H264) {
        av_opt_set(pCodecCtx->priv_data, "preset", "slow", 0);
    }

    ret = avcodec_open2(pCodecCtx, pCodec, NULL);
    if(ret < 0) {
        printf("fail to open codec\n");
        return -1;
    }


    pFrame = av_frame_alloc();
    if(!pFrame) {
        printf("Could not allocate video frame.\n");
        return -1;
    }

    pFrame->format = pCodecCtx->pix_fmt;
    pFrame->width = pCodecCtx->width;
    pFrame->height = pCodecCtx->height;

    ret = av_image_alloc(pFrame->data, pFrame->linesize, pCodecCtx->width, pCodecCtx->height,
        pCodecCtx->pix_fmt, 16);
    if(ret < 0) {
        printf("Could not allocate raw picture buffer\n");
        return -1;
    }

    fp_in = fopen(filename_in, "rb");
    if(!fp_in) {
        printf("Could not open %s\n", filename_in);
        return -1;
    }

    fp_out = fopen(filename_out, "wb");
    if(!fp_out) {
        printf("Could not open %s\n", filename_out);
        return -1;
    }

    y_size = pCodecCtx->width * pCodecCtx->height;
    for(i = 0; i < framenum; i++) {
        av_new_packet(&pkt, y_size * 3);
        if(fread(pFrame->data[0], 1, y_size, fp_in) <= 0 ||
        fread(pFrame->data[1], 1, y_size / 4, fp_in) <= 0 ||
        fread(pFrame->data[2], 1, y_size / 4, fp_in) <= 0) {
            return -1;
        } else if(feof(fp_in)) {
            break;
        }
 
        pFrame->pts = i;
        ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_output);
        if(ret < 0) {
            printf("Error encoding frame\n");
            return -1;
        }

        if(got_output) {
            printf("Succeed to encode fame:%5d\tsize:%5d\n", framecnt, pkt.size);
            framecnt++;
            fwrite(pkt.data, 1, pkt.size, fp_out);
        }
           av_free_packet(&pkt);
    }

    for(got_output = 1; got_output; i++) {
        ret = avcodec_encode_video2(pCodecCtx, &pkt, NULL, &got_output);
        if(ret < 0) {
            printf("Error encoding frame\n");
            return -1;
        }

        if(got_output) {
            printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", pkt.size);
            fwrite(pkt.data, 1, pkt.size, fp_out);
            av_free_packet(&pkt);
        }
    }

    fclose(fp_out);
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);
    av_frame_free(&pFrame);

    return 0;
}