#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavcodec/avcodec.h>

#define INBUF_SIZE 4096

static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize, char *filename)
{
    FILE *f;
    int i;

   f = fopen(filename, "w");

    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
    for(i = 0; i < ysize; i++) {
        fwrite(buf + i * wrap, 1, xsize, f);        
    }

    fclose(f);
}

static decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt, const char *filename)
{
    char buf[1024];
    int ret;

    ret = avcodec_send_packet(dec_ctx, pkt);
    if(ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding\n");
        exit(1);
    }

    while(ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return;
        }else if(ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }

        printf("saving frame %3d\n", dec_ctx->frame_number);
        fflush(stdout);

        snprintf(buf, sizeof(buf), "%s-%d", filename, dec_ctx->frame_number);
        pgm_save(frame->data[0], frame->linesize[0],
            frame->width, frame->height, buf);
    }
}

int main(int argc, char *argv[]) 
{
    const char *filename, *outfilename;
    const AVCodec *codec;
    AVCodecParserContext *parser;
    AVCodecContext *c = NULL;
    FILE *f;
    AVFrame *frame;
    uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    uint8_t *data;
    size_t data_size;
    int ret;
    AVPacket *pkt;

    if(argc <= 2) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        exit(0);
    }

    filename = argv[1];
    outfilename = argv[2];

    av_register_all();
    pkt = av_packet_alloc();
    if(!pkt) {
        exit(1);
    }

    memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);

    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if(!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    parser = av_parser_init(codec->id);
    if(!parser) {
        fprintf(stderr, "parser not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if(!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    if(avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    f = fopen(filename, "rb");
    if(!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

    frame = av_frame_alloc();
    if(!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    
    while(!feof(f)) {
        data_size = fread(inbuf, 1, INBUF_SIZE, f);
        if(!data_size)
            break;
        
        data = inbuf;
        while(data_size > 0) {
            ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
                                   data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            if(ret < 0) {
                fprintf(stderr, "Error while parsing\n");
                exit(1);                
            }

            data += ret;
            data_size -= ret;

            if(pkt->size) 
                decode(c, frame, pkt, outfilename);
        }
    }

    decode(c, frame, NULL, outfilename);

    fclose(f);

    av_parser_close(parser);
    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);

    return 0;
}