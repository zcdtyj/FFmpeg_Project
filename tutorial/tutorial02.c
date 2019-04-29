#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include <SDL.h>
#include <SDL_thread.h>

#include <stdio.h>

int main(int argc, char *argv[]) {
    AVFormatContext *pFormatCtx = NULL;
	int videoStream = 0;
	AVCodecContext *pCodecCtx = NULL;
	AVCodec *pCodec = NULL;
	AVFrame *pFrame = NULL;
	AVPacket packet;

	int frameFinished = -1;
	int i = 0;
	AVDictionary *optionsDict = NULL;
	struct SwsContext *sws_ctx = NULL;

	SDL_Overlay *bmp = NULL;
	SDL_Surface *screen = NULL;
	SDL_Rect rect;
	SDL_Event event;

	if(argc < 2) {
	    fprintf(stderr, "Usage: test <file>\n");
		exit(1);
	}

	av_register_all();
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
	    fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
		exit(1);
	}

	if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL) != 0)
		return -1;

	if(avformat_find_stream_info(pFormatCtx, NULL) < 0)
		return -1;

    av_dump_format(pFormatCtx, 0, argv[1], 0);

	videoStream = -1;
	for(i = 0; i < pFormatCtx->nb_streams; i++) {
	    if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
		    videoStream = i;
			break;
		}
	}

	if(videoStream == -1)
		return -1;

	pCodecCtx = pFormatCtx->streams[videoStream]->codec;

	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec == NULL) {
        fprintf(stderr, "Unsupported codec!\n");
		return -1;
	}

	if(avcodec_open2(pCodecCtx, pCodec, &optionsDict) < 0) {
	    return -1;
	}

	pFrame = av_frame_alloc();
	screen  = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 24, 0);

	if(!screen) {
	    fprintf(stderr, "SDL: could not set video mode - exiting\n");
		exit(1);
	}

	bmp = SDL_CreateYUVOverlay(pCodecCtx->width,
			                   pCodecCtx->height,
							   SDL_YV12_OVERLAY,
							   screen);

	sws_ctx = sws_getContext(
			pCodecCtx->width,
			pCodecCtx->height,
			pCodecCtx->pix_fmt,
			pCodecCtx->width,
			pCodecCtx->height,
			AV_PIX_FMT_YUV420P,
			SWS_BILINEAR,
			NULL, NULL, NULL
			);

	while(av_read_frame(pFormatCtx, &packet) >= 0) {
	    if(packet.stream_index == videoStream) {
		    avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

			if(frameFinished) {
			    SDL_LockYUVOverlay(bmp);

				AVPicture pict;
				pict.data[0] = bmp->pixels[0];
				pict.data[1] = bmp->pixels[2];
				pict.data[2] = bmp->pixels[1];

				pict.linesize[0] = bmp->pitches[0];
				pict.linesize[1] = bmp->pitches[2];
				pict.linesize[2] = bmp->pitches[1];
			

			    sws_scale(
			        sws_ctx,
			        (uint8_t const * const *)pFrame->data,
			        pFrame->linesize,
			        0,
			        pCodecCtx->height,
			        pict.data,
			        pict.linesize
		            );

			   SDL_UnlockYUVOverlay(bmp);

			   rect.x = 0;
			   rect.y = 0;
			   rect.w = pCodecCtx->width;
			   rect.h = pCodecCtx->height;
			   SDL_DisplayYUVOverlay(bmp, &rect);
			   SDL_Delay(40);
		    }
	    }

	    av_free_packet(&packet);

	    SDL_PollEvent(&event);
	    switch(event.type) {
	    case SDL_QUIT:
		    SDL_Quit();
		    exit(0);
		    break;
	    default:
		    break;
	    }
	}

    av_free(pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}
