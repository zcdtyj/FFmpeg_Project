#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

int main(int argc, char *argv[])
{
    AVFormatContext *pFormatCtx;
    AVOutputFormat *fmt;
    AVStream *video_st;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;

    uint8_t *picture_buf;
    AVFrame *picture;
    AVPacket pkt;
    int y_size;
    int got_picture = 0;
    int size;

    int ret = 0;

    FILE *in_file = NULL;
    int in_w = 320, in_h = 180;

    const char *out_file = "../resource/test_yuv420p_320x180.h264";
    in_file = fopen("../resource/test_yuv420p_320x180.yuv", "rb");

    av_register_all();

    pFormatCtx = avformat_alloc_context();
    avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file);
    fmt = pFormatCtx->oformat;

    // fmt = av_guess_format(NULL, out_file, NULL);
	// pFormatCtx->oformat = fmt;

	if(avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0) 
	{
	    printf("Filed to open output file!\n");
        return -1;
	}

    video_st = avformat_new_stream(pFormatCtx, 0);
    if(video_st == NULL) {
        return -1;
    }

    pCodecCtx = video_st->codec;
    pCodecCtx->codec_id = AV_CODEC_ID_H264;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

    pCodecCtx->width = in_w;
    pCodecCtx->height = in_h;

    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;

    pCodecCtx->bit_rate = 400000;
    pCodecCtx->gop_size = 250;

    pCodecCtx->qmax = 51;
    pCodecCtx->qmin = 10;

    av_dump_format(pFormatCtx, 0, out_file, 1);
    pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if(!pCodec) {
        printf("Codec not found.\n");
        return -1;
    }

    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("could not open codec.\n");
        return -1;
    }

    picture = av_frame_alloc();
    picture->width = pCodecCtx->width;
    picture->height = pCodecCtx->height;
    picture->format = pCodecCtx->pix_fmt;
    size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
    picture_buf = (uint8_t *)av_malloc(size);
    avpicture_fill((AVPicture *)picture, (const uint8_t *)picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);

    avformat_write_header(pFormatCtx, NULL);

    int i = 0;
    y_size = pCodecCtx->width * pCodecCtx->height;
    av_new_packet(&pkt, size);
    while(fread(picture_buf, 1, y_size * 3 / 2, in_file) > 0) {
        picture->data[0] = picture_buf;
        picture->data[1] = picture_buf + y_size;
        picture->data[2] = picture_buf + y_size * 5 / 4;

        picture->pts = i;
        i++;
        ret = avcodec_encode_video2(pCodecCtx, &pkt, picture, &got_picture);
        if(ret < 0) {
            printf("Encode Error.\n");
            return -1;
        }

        if(got_picture == 1) {
            pkt.stream_index = video_st->index;
            av_write_frame(pFormatCtx, &pkt);
            av_free_packet(&pkt);
        }
    }
    av_write_trailer(pFormatCtx);
    if(video_st) {
        avcodec_close(video_st->codec);
        av_free(picture);
        av_free(picture_buf);
    }
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);
    fclose(in_file);   
}