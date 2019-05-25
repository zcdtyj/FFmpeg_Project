#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

in flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index) {
    int ret;
    int got_frame;
    AVPacket enc_pkt;
    if(!(fmt_ctx->streams[stream_index]->codec->codec->capabilities & CODEC_CAP_DELAY))
        return 0;

    while(1) {
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        ret = avcodec_encode_audio2(fmt_ctx->streams[stream_index]->codec, &enc_pkt, NULL, &got_frame);
        av_frame_free(NULL);

        if(ret < 0) {
            return;
        }

        if(!got_frame) {
            ret = 0;
            break;
        }

        printf("Flush Encoder: succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);

        ret = av_write_frame(fmt_ctx, &enc_pkt);
        if(ret < 0)
            break;            
    }

    return ret;
}

int main(int argc, char *argv[]) {
    AVFormatContext *pFormatCtx;
    AVOutputFormat*fmt;
    AVStream *audio_st;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;

    uint8_t *frame_buf;
    AVFrame *pFrame;
    AVPacket pkt;

    int got_frame = 0;
    int ret = 0;
    int size = 0;

    FILE *in_file = NULL;
    int framenum = 1000;
    const char *out_file = "../resource/ws.aac";
    int i;

    in_file = fopen("../resource/ws.pcm", "rb");

    av_register_all();
    pFormatCtx = avformat_alloc_context();
    fmt = av_guess_format(NULL, out_file, NULL);
    pFormatCtx->oformat = fmt;

    if(avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0) {
        printf("Failed to open output file\n");    
        return -1;
    }

    audio_st = avformat_new_stream(pFormatCtx, 0);
    if(audio_st == NULL) {
        return -1;
    }

    pCodecCtx = audio_st->codec;
    pCodecCtx->codec_id = fmt->audio_codec;
    pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
    pCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
    pCodecCtx->sample_rate = 44100;
    pCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
    pCodecCtx->channels = av_get_channel_layout_nb_channels(pCodecCtx->channel_layout);
    pCodecCtx->bit_rate = 64000;

    av_dump_format(pFormatCtx, 0, out_file, 1);

    pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if(!pCodec) {
        printf("Can not find encoder\n");
        return -1;
    }

    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("Failed to open encoder!\n");
        return -1;
    }

    pFrame = av_frame_alloc();
    pFrame->nb_samples = pCodecCtx->frame_size;
    pFrame->format = pCodecCtx->sample_fmt;

    size = av_samples_get_buffer_size(NULL, pCodecCtx->channels, pCodecCtx->frame_size, pCodecCtx->sample_fmt, 1);
    frame_buf = (uint8_t *)av_malloc(size);
    avcodec_fill_audio_frame(pFrame, pCodecCtx->channels, pCodecCtx->sample_fmt, (const uint8_t *)frame_buf, size, 1);

    avformat_write_header(pFormatCtx, NULL);
    av_new_packet(&pkt, size);
    
    for(i = 0; i < framenum; i++) {
        if(fread(frame_buf, 1, size, in_file) <= 0) {
            printf("Failed to read raw data!\n");
            return -1;
        } else if(feof(in_file)) {
            break;
        }

        pFrame->data[0] = frame_buf;
        pFrame->pts = i * 100;

        ret = avcodec_encode_audio2(pCodecCtx, &pkt, pFrame, &got_frame);
        if(ret < 0) {
            printf("Failed to encode!\n");
            return -1;
        }

        if(got_frame == 1) {
            printf("Succeed to encode 1 frame \tsize:%5d\n", pkt.size);
            pkt.stream_index = audio->index;
            ret = av_write_frame(pFormatCtx, &pkt);
            av_free_packet(&pkt);
        }

        ret = flush_encoder(pFormatCtx, 0);
        if(ret < 0) {
            printf("Flushing encoder failed\n");
            return -1;
        }

        av_write_trailer(pFormatCtx);

        if(audio_st) {
            avcodec_close(audio_st->codec);
            av_free(pFrame);
            av_free(frame_buf);
        }

        avio_close(pFormatCtx->pb);
        avformat_free_context(pFormatCtx);
        fclose(in_file);

        return 0;
    }
}
