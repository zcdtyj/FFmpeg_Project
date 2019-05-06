#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __STDC_CONSTANT_MACROS

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <SDL2/SDL.h>

#define MAX_AUDIO_FRAME_SIZE 192000

static uint8_t *audio_chunk;
static uint32_t audio_len;
static uint8_t *audio_pos;

void fill_audio(void *udata, uint8_t *stream, int len) {
    SDL_memset(stream, 0, len);
	if(audio_len == 0)
		return;

    len = (len > audio_len ? audio_len:len);
	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_len -= len;
}

int main(int argc, char *argv[]) {
    AVFormatContext *pFormatCtx;
	int i, audioStream;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec;
	AVPacket *packet;
	uint8_t *out_buffer;
	AVFrame *pFrame;
	SDL_AudioSpec wanted_spec;
    int ret;
	uint32_t len = 0;
    int got_picture;
	int index = 0;
	int64_t in_channel_layout;
	struct SwrContext *au_convert_ctx;
    
	char url[] = "test.mp3";
	av_register_all();
	avformat_network_init();

	pFormatCtx = avformat_alloc_context();
	if(avformat_open_input(&pFormatCtx, url, NULL, NULL) != 0) {
	    printf("Ccouldn't open input stream.\n");
		return -1;
	}

	if(avformat_find_stream_info(pFormatCtx, NULL) < 0) {
	    printf("Could not find stream info.\n");
		return -1;
	}

	av_dump_format(pFormatCtx, 0, url, 0);

	audioStream = -1;
	for(i = 0;  i < pFormatCtx->nb_streams; i++) {
	    if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
		    audioStream = i;
			break;
		}
	}

	if(audioStream == -1) {
	    printf("Did not find a audio stream.\n");
		return -1;
	}

    pCodecCtx = pFormatCtx->streams[audioStream]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec == NULL) {
	    printf("Codec not found.\n");
		return -1;
	}

	if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
	    printf("could not open codec.\n");
		return -1;
	}

    packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	av_init_packet(packet);

	uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
	int out_nb_samples = pCodecCtx->frame_size;
	enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    int out_sample_rate = 44100;
	int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
    int out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);

	out_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE * 2);
	pFrame = av_frame_alloc();

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
	    printf("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}

	wanted_spec.freq = out_sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = out_channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = out_nb_samples;
	wanted_spec.callback = fill_audio;
	wanted_spec.userdata = pCodecCtx;
    
	if(SDL_OpenAudio(&wanted_spec, NULL) < 0) {
	    printf("can't open audio.\n");
		return -1;
	}

	in_channel_layout = av_get_default_channel_layout(pCodecCtx->channels);
    au_convert_ctx = swr_alloc();
	au_convert_ctx = swr_alloc_set_opts(au_convert_ctx, out_channel_layout, out_sample_fmt, 
			out_sample_rate, in_channel_layout, pCodecCtx->sample_fmt, pCodecCtx->sample_rate, 0, NULL);
	swr_init(au_convert_ctx);

    SDL_PauseAudio(0);
	while(av_read_frame(pFormatCtx, packet) >= 0) {
	    if(packet->stream_index == audioStream) {
		    ret = avcodec_decode_audio4(pCodecCtx, pFrame, &got_picture, packet);
			if(ret < 0) {
			    printf("Error in decoding audio frane.\n");
				return -1;
			}

			if(got_picture > 0) {
			    swr_convert(au_convert_ctx, &out_buffer, MAX_AUDIO_FRAME_SIZE, (const uint8_t **)pFrame->data, pFrame->nb_samples);
                printf("index:%5d\t pts:%lld\t packet_size:%d\n", index, packet->pts, packet->size);
			    index++;
			}

			while(audio_len > 0)
				SDL_Delay(1);

			audio_chunk = (uint8_t *) out_buffer;
			audio_len = out_buffer_size;
			audio_pos = audio_chunk;
		}
		av_free_packet(packet);
	}
	swr_free(&au_convert_ctx);
	SDL_CloseAudio();
	SDL_Quit();

	av_free(out_buffer);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}
