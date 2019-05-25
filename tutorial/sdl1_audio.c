#include <stdlib.h>
#include <string.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>

#include <SDL.h>

#define MAX_AUDIO_FRAME_SIZE 192000
#define OUTPUT_PCM 1
#define USE_SDL 1

static uint8_t *audio_chunk;
static uint32_t *audio_len;
static uint8_t *audio_pos;

void fill_audio(void *udata, uint8_t *stream, int len) {
    SDL_memset(stream, 0, len);
	if(audio_len == 0)
		return;

	len = (len > audio_len ? audio_len : len);
	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);

	audio_pos += len;
	audio_len -= len;
}

int main(int argc, char *argv[]) 
{
    AVFormatContext *pFormatCtx;
	int i, audioStream;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec;
    
	char url[] = "../resource/test.mp3";

	av_register_all();
	pFormatCtx = avformat_alloc_context();

	if(avformat_open_input(&pFormatCtx, url, NULL, NULL) != 0) {
	    return -1;
	}

	if(avformat_find_stream_info(pFormatCtx, NULL) < 0)
		return -1;

	av_dump_format(pFormatCtx, 0, url, 0);

	audioStream = -1;

	for(i = 0; i < pFormatCtx->nb_streams; i++)
		if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
		    audioStream = i;
			break;
		}

	if(audioStream == -1)
		return -1;
    
	pCodecCtx = pFormatCtx->streams[audioStream]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	avcodec_open2(pCodecCtx, pCodec, NULL);

	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	av_init_packet(packet);
	uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;

	int out_nb_samples = pCodecCtx->frame_size;
	enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
	int out_sample_rate = 44100;
	int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);

	int out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);
	uint8_t *out_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE * 2);

	AVFrame *pFrame;
	pFrame = av_frame_alloc();

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
	    return -1;
	}
    
	SDL_AudioSpec wanted_spec;
    wanted_spec.freq = out_sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = out_channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = out_nb_samples;
	wanted_spec.callback = fill_audio;
	wanted_spec.userdata = pCodecCtx;

	if(SDL_OpenAudio(&wanted_spec, NULL) < 0) {
	    return -1;
	}

	printf("Bitrate:\t %3d \n", pFormatCtx->bit_rate);
	printf("Decoder name:\t %s \n", pCodecCtx->codec->long_name);
	printf("Channels:\t %d\n", pCodecCtx->channels);
	printf("Sample per Second\t %d \n", pCodecCtx->sample_rate);

	uint32_t ret, len = 0;
	int got_picture;
	int index = 0;
	int64_t in_channel_layout = av_get_default_channel_layout(pCodecCtx->channels);

	struct SwrContext *au_convert_ctx;
	au_convert_ctx = swr_alloc();
	au_convert_ctx = swr_alloc_set_opts(au_convert_ctx, out_channel_layout, out_sample_fmt, out_sample_rate,
			in_channel_layout, pCodecCtx->sample_fmt, pCodecCtx->sample_rate, 0, NULL);
	swr_init(au_convert_ctx);

	SDL_PauseAudio(0);
	while(av_read_frame(pFormatCtx, packet) >= 0) {
	    if(packet->stream_index == audioStream) {
		    ret = avcodec_decode_audio4(pCodecCtx, pFrame, &got_picture, packet);
			if(ret < 0) {
			    return -1;
			}

			if(got_picture > 0) {
			    swr_convert(au_convert_ctx, &out_buffer, MAX_AUDIO_FRAME_SIZE, (const uint8_t **)pFrame->data, pFrame->nb_samples);
				index++;
			}

			while(audio_len > 0) {
				SDL_Delay(1);
			}
            
			audio_chunk = (uint8_t *)out_buffer;
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
	avformat_close_input(pFormatCtx);
	return 0;
}
