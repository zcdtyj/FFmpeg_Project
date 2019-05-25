#include <stdio.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>

int main(int argc, char *argv[]) {
    FILE *src_file = fopen("../resource/sintel_480x272_yuv420p.yuv", "rb");
	const int src_w = 480, src_h = 272;
	AVPixelFormat src_pixfmt = AV_PIX_FMT_YUV420P;

	int src_bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(src_pixfmt));

	FILE *dst_file = fopen("../resource/sintel_1280x720_rgb24.rgb", "wb");
	const int dst_w = 1280, dst_h = 720;
	AVPixelFormat dst_pixfmt = AV_PIX_FMT_RGB24;
	int dst_bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(dst_pixfmt));

	uint8_t *src_data[4];
	int src_linesize[4];

	uint8_t *dst_data[4];
	int dst_linesize[4];

	int rescale_method = SWS_BICUBIC;
	struct SwsContext *img_convert_ctx;
	uint8_t *temp_buffer = (uint8_t *)av_malloc(src_w * src_h * src_bpp / 8);
	
	int frame_idx = 0;
	int ret = 0;
	ret = av_image_alloc(src_data, src_linesize, src_w, src_h, src_pixfmt, 1);
	if(ret < 0) {
	    printf("Could not allocate destination image\n");
		return -1;
	}

	ret = av_image_alloc(dst_data, dst_linesize, dst_w, dst_h, dst_pixfmt, 1);
	if(ret < 0) {
	    printf("Could not allocate destination image\n");
		return -1;
	}

	img_convert_ctx = sws_alloc_context();
    av_opt_show2(img_convert_ctx, stdout, AV_OPT_FLAG_VIDEO_PARAM, 0);
	av_opt_set_int(img_convert_ctx, "sws_flags", SWS_BICUBIC | SWS_PRINT_INFO, 0);
	av_opt_set_int(img_convert_ctx, "srcw", src_w, 0);
	av_opt_set_int(img_convert_ctx, "srch", src_h, 0);
	av_opt_set_int(img_convert_ctx, "src_format", src_pixfmt, 0);
    


}

