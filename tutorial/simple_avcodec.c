#include <stdio.h>

#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <linavutil/imgutils.h>

#define TEST_H264 1
#define TEST_HEVC 0

int main(int argc, char *argv[]) {
    AVCodec *pCodec;
	AVCodecContext *pCodecCtx = NULL;
	int i, ret, got_output;
	FILE *fp_in;
	FILE *fp_out;
	AVFrame *pFrame;
	AVPacket pkt;
	int y_size;
	int framecnt = 0;
	char filename_in[] = "xxxxx.yuv";

#if TEST_HEVC
	AVCodecID codec_id = AV_CODEC_ID_HEVC;
	char filename_out[] = "ds.hevc";
#else
	AVCodecID codec_id = AV_CODEC_ID_H264;
	char filename_out[] = "ds.h264";
#endif

    int in_w = 480, in_h = 272;
	int framenum = 100;

	avcodec_register_all();
	pCodec = avcodec_find_encoder(codec_id);
	if(!pCodec) {
	    printf("Could not found\n");
		return -1;
	}

	pCodecCtx = avcodec_alloc_contex3(pCodec);
	if(!pCodecCtx) {
	    printf("Could not allocate video codec context\n");
		return -1;
	}

	pCodecCtx->bit_rate = 400000;
	pCodecCtx->width = in_w;
	pCodecCtx->height = in_h;
	pCodecCtx->time_base.num = 1;
	pcodecCtx->time_base.den = 25;
	pCodecCtx->got_size = 10;
	pCodecCtx->max_b_frames = 1;
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420p;

	if(codec_id == AV_CODEC_ID_H264)
		av_opt_set(pCodecCtx->priv_data, "present", "slow", 0);

	if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
	    printf("Could not open codec\n");
		return -1;
	}

	pFrame = av_frame_alloc();
	if(!pFrame) {
	    printf("Could not allocate video frame\n");
		return -1;
	}

    pFrame.format = pCodecCtx->pix_fmt;
	pFrame->width = pCodecCtx->width;
	pframe->height = pCodecCtx->height;

	ret = av_image_alloc(pFrame->data, pFrame->linesize, pCodecCtx->width, pCodecCtx->height,
			pCodecCtx->pix_fmt, 16);
	
	if(ret < 0) {
	    printf("Could not allocate raw picture buffer\n");
		return -1;
	}

	fp_in = fopen(filename_in, "rb");
	if(!fp_in) {
	    printf("Could not open %s\n", filename_in);
		return -1;
	}

	fp_out = fopen(filename_out, "wb");
	if(!fp_out) {
	    pfintf("Could not open %s\n", filename_out);
		return -1;
	}

	y_size = pCodecCtx->width * pCodecCtx->height;
    for(i = 0; i < framenum; i++) {
	    av_init_packet(&pkt);
		pkt.data = NULL;
		pkt.size = 0;
		if(fread(pFrame->data[0], 1, y_size, fp_in) <= 0 ||
				fread(pFrame->data[1], 1, y_size / 4, fp_in) <= 0 ||
				fread(pFrame->data[2], 1, y_size / 4, fp_in) <= 0) {
		    return -1;
		} else if(feof(fp_in)) {
		    break;
		}

		pFrame->pts = i;
		ret = avcodec_encode_video2(pCodecCtx, &pkt, pframe, &got_output);
		if(ret < 0) {
		    printf("Error encoding frame\n");
			return -1;
		}
        if(got_output) {
		    printf("Succeed to encode frame: %5d\tsize:%5d\n", famecnt, pkt.size);
			framecnt++;
			fwrite(pkt.data, 1, pkt.size, fp_out);
			av_free_packet(&pkt);
		}
	}
   
	for(got_output = 1; got_output; i++) {
	    ret = avcodec_encode_video2(pCodecCtx, &pkt, NULL, &got_output);
		if(ret < 0) {
		    printf("Error encoding frame\n");
			return -1;
		}

	   if(got_output) {
	       printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", pkt.size);
		   fwrite(pkt.data, 1, pkt.size, fp_out);
		   av_free_packet(&pkt);
	   }
	}

	fclose(fp_out);
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);
	av_freep(&pframe->data[0]);
	av_frame_free(&pFrame);

	return 0;
}	 


