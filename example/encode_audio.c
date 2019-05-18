#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>

int main(int argc, char **argv)
{
    const char *filename;
    const AVCodec *codec;
    AVCodecContext *c = NULL;
    AVframe *frame;
    AVPacket *pkt;
    int i, j, k, ret;
    FILE *f;
    uint16_t *samples;
    float t, tincr;

    if(argc <= 1) {
        fprintf(stderr, "Usgae: %s <output file>\n", argv[0]);
        return 0;
    }

    filename = argv[1];

    codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if(!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if(!c) {
        fprintf(stderr, "Could not allocate audio codec context\n");
        exit(1);
    }

    c->bit_rate = 64000;
    c->sample_fmt = AV_SAMPLE_FMT_S16;
    if(!check_sample_fmt(codec, c->sample_fmt)) {
        fprintf(stderr, "Encoder does not support sample format %s", av_get_sample_fmt_name(c->sample_fmt));
        exit(1);
    }

    c->sample_rate = select_sample_rate(codec);
    c->channel_layout = select_channel_layout(codec);
    c->channels = av_get_channel_layout_nb_channels(c->channel_layout);

    if(avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    f = fopen(filename, "wb");
    if(!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

    pkt = av_packet_alloc();
    if(!pkt) {
        fprintf(stderr, "Could not allocate the packet\n")
        exit(1);
    }

    frame = av_frame_alloc();
    if(!frame) {
        fprintf(stderr, "Could not allocate audio frame\n");
        exit(1);
    }

    frame->nb_samples = c->frame_size;
    frame->format = c->sample_fmt;
    frame->channel_layout = c->channel_layout;

    ret = av_frame_get_buffer(frame, 0);
    if(ret < 0) {
        fprintf(stderr, "Could not allocate audio data buffers\n");
        exit(1);
    }
    
    t = 0;
    tincr = 2 * M_PI * 440.0 / c->sample_rate;
    for(i = 0; i < 200; i++) {
        ret = av_frame_make_writable(frame);
        if(ret < 0)
            exit(1);
    }

}