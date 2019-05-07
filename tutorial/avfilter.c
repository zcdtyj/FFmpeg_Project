#include <stdio.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <SDL/SDL.h>

#define ENABLE_SDL 1

#define ENABLE_YUVFILE 1

const char *filter_descr = "movie=my_logo.png[wm];[in][wm]overlay=5:5[out]";

static AVFormatContext *pFormatctx;
static AVCodecContext *pCodecCtx;
AVFilterContext *buffersink_ctx;
AVFilterContext *buffersrc_ctx;
AVFilterGraph *filter_graph;
static int video_stream_index = -1;

static int open_input_file(const char *filename) {
    int ret;
    AVCodec *dec;

    if((ret = avformat_open_input(&pformatCtx, filename, NULL, NULL)) < 0) {
        printf("Cannot open input file\n");
        return ret;
    }

    if((ret = avformat_find_stream_info(pFormatCtx, NULL)) < 0) {
        printf("Cannot find stream information\n");
        return ret;
    }

    ret = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if(ret < 0) {
        printf("Cannot find a video stream in the input file\n");
        return ret;
    }

    video_stream_index = ret;
    pCodecCtx = pFormatCtx->streams[video_stream_index]->codec;

    if((ret = avcodec_open2(pCodecCtx, dec, NULL)) < 0) {
        printf("cannot open video decoder\n");
        return ret;
    }

    return 0;
}

static int init_filters(const_char *filters_descr) 
{
    char args[512];
    int ret;
    AVfilter *buffersrc = avfilter_get_by_name("buffer");
    AVFilter *buffersink = avfilter_get_by_name("ffbuffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVfilterInOut *inputs = avfilter_inout_alloc();
    enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE};
    AVBufferSinkParams *buffersink_params;

    filter_graph = avfilter_graph_alloc();
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pix_aspect=%d/%d", 
             pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
             pCodecCtx->time_base.num, pCodecCtx->time_base.den,
             pCodecCtx->sample_aspect_ratio.num, pCodecCtx->sample_aspect_ratio.den);
    
    ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",args, NULL, filter_graph);

}

int amin(int argc, char *argv[]) {
    int ret;
    AVPacket packet;
    AVframe *pFrame;
    AVFrame *pFrame_out;

    int got_frame;
    av_register_all();
    avfilter_register_all();

    if((ret = open_input_file("test.flv")) < 0)
        goto end;
    if((ret = init_filters(ilter_descr)) < 0)
        goto end;

    #if ENABLE_YUVFILE
        FILE *fp_yuv = fopen("test_yuv420p_320x180.yuv", "wb+");
    #endif
    
    #if ENABLE_SDL
        SDL_Surface *screen;
        SDL_Overlay *bmp;
        SDL_Rect rect;
        if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
            printf("Could not initialize SDL - %s\n", SDL_GetError());
            return -1;
        }
        screen = SDL_SetVideomode(pCodecCtx->width, pCodecCtx->height, 0, 0);
        if(!screen) {
            printf("SDL: could notset video mode - exiting\n");
            return -1;
        }
        bmp = SDL_CreateYUVOverlay(pCodecCtx->width, pCodecCtx->height, SDL_YV12_OVERLAY, screen);

}