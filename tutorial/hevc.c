#include <stdio.h>
#include <libavcodec/avcodec.h>

#define TEST_H264 1
#define TEST_HEVC 0

int main(int argc, char *argv[])
{
    AVCodec *pCodec;
	AVCodecContext *pCodecCtx = NULL;
	AVCodecParserContext *pCodecParserCtx = NULL;

	FILE *fp_in;
	FILE *fp_out;

	AVFrame *pFrame;
	const int in_buffer-size = 4096;
	uint8_t in_buffer[in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE] = {0};
	uint8-t *cur_ptr;
	int cur_size;
	AVPacket packet;
	int ret, got_picture;
	int y_size;

#if TEST_HEVC

}
