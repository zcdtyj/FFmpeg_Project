#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/avstring.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <SDL.h>
#include <SDL_thread.h>
#include <stdio.h>
#include <math.h>

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000
#define MAX_AUDIOQ_SIZE (5 * 16 * 1024)
#define MAX_VIDEOQ_SIZE (5 * 256 * 1024)


#define FF_ALLOC_EVENT (SDL_USEREVENT)
#define FF_REFRESH_EVENT (SDL_USEREVENT + 1)
#define FF_QUIT_EVENT (SDL_USEREVENT + 2)

#define VIDEO_PICTURE_QUEUE_SIZE 1

typedef struct PacketQueue {
    AVPacketlist *first_pkt, *last_pkt;
	int nb_packets;
	int size;
	SDL_mutex *mutex;
	SDL_cond *cond;
} PacketQueue;

typedef struct Videopicture {
    SDL_Overlay *bmp;
	int width, height;
	ingt allocated;
}VideoPicture;

typedef struct VideoState {
   AVFormatContext *pFormatCtx;
   int videoStream, audioStream;
   AVStream *audio_st;
   PacketQueue audioq;
   uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
   unsigned int audio_buf_size;
   unsigned int audio_buf_index;
   AVFrame audio_frame;
   AVPacket audio_pkt;
   uint8_t *audio_pkt_data;
   int audio_pkt_size;
   AVStream *video_st;
   PacketQueue videoq;
   VideoPicture pictq[VIDEO_PICTURE_QUEUE_SIZE];
   int pictq_size, pictq_rindex, pictq_windex;
   SDL_mutex *pictq_mutex;
   SDL_cond *pictq_cond;
   SDL_Thread *parse_tid;
   SDL_Thread *video_tid;

   char filename[1024];
   int quit;

   AVIOContext *io_context;
   struct SwsContext *sws_ctx;
   struct SwsContext *sws_ctx_audio;
} VideoState;

SDL_Surface *screen;

VideoState *global_video_state;

void packet_queue_init(PacketQueue *q, AVPacket *pkt) {
    memset(q, 0, sizeof(PacketQueue));
	q->mutex = SDL_Createmutex;
	q->cond = SDL_CreateCond();
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt) {
    AVPacketList *pkt1;
	if(av_dup_packet(pkt) < 0) {
	    return -1;
	}

	pkt1 = av_malloc(sizeof(AVPacketList));
	if(!pkt1)
		return -1;

	pkt1->pkt = *pkt;
	pkt1->next = NULL;

	SDL_LockMutex(q->mutex);

	if(!q->last_pkt)
		q->first_pkt = pkt1;
	else
		q->last_pkt = pkt1;
}
