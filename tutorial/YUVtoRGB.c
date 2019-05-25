#include <stdio.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>

int main(int argc, char *argv[]) 
{
    FILE *src_file = fopen("../resource/test_yuv420p_320x180.yuv", "rb");
    const int src_w = 320, src_h = 180;
    enum AVPixelFormat src_pixfmt = AV_PIX_FMT_YUV420P;
    int src_bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(src_pixfmt));

    FILE *dst_file = fopen("../resource/test_yuv420p_640x480.rgb", "wb+");
    const int dst_w = 640, dst_h = 480;
    enum AVPixelFormat dst_pixfmt  = AV_PIX_FMT_RGB24;
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
        printf("Could not allocate source image\n");
        return -1;
    }

    ret = av_image_alloc(dst_data, dst_linesize, dst_w, dst_h, dst_pixfmt, 1);
    if(ret < 0) {
        printf("Could not allocate destination image\n");
        return -1;
    }

    // img_convert_ctx = sws_alloc_context();
    // av_opt_show2(img_convert_ctx, stdout, AV_OPT_FLAG_VIDEO_PARAM, NULL);

    // av_opt_set_int(img_convert_ctx, "sws_flags", SWS_BICUBIC | SWS_PRINT_INFO, 0);
    // av_opt_set_int(img_convert_ctx, "srcw", src_w, 0);
    // av_opt_set_int(img_convert_ctx, "srch", src_h, 0);
    // av_opt_set_int(img_convert_ctx, "src_format", src_pixfmt, 0);
    // av_opt_set_int(img_convert_ctx, "src_range", 1, 0);

    // av_opt_set_int(img_convert_ctx, "dstw", dst_w, 0);
    // av_opt_set_int(img_convert_ctx, "dsth", dst_h, 0);
    // av_opt_set_int(img_convert_ctx, "dst_format", dst_pixfmt, 0);
    // av_opt_set_int(img_convert_ctx, "dst_range", 1, 0);
    // sws_init_context(img_convert_ctx, NULL, NULL);
   
   img_convert_ctx = sws_getContext(src_w, src_h,src_pixfmt, dst_w, dst_h, dst_pixfmt, rescale_method, NULL, NULL, NULL);

    while(1) {
        if(fread(temp_buffer, 1, src_w * src_h * src_bpp / 8, src_file) != src_w * src_h * src_bpp / 8){
            printf("Finish color space convert\n");
            break;   
        }
        
        switch(src_pixfmt) {
            case AV_PIX_FMT_GRAY8:
                memcpy(src_data[0], temp_buffer, src_w * src_h);
                break;
            case AV_PIX_FMT_YUV420P:
                memcpy(src_data[0], temp_buffer, src_w * src_h);
                memcpy(src_data[1], temp_buffer + src_w * src_h, src_w * src_h / 4);
                memcpy(src_data[2], temp_buffer + src_w * src_h * 5 / 4, src_w * src_h / 4);
                break;
            case AV_PIX_FMT_YUV422P:
                memcpy(src_data[0], temp_buffer, src_w * src_h);
                memcpy(src_data[1], temp_buffer + src_w * src_h, src_w * src_h / 2);
                memcpy(src_data[2], temp_buffer + src_w * src_h * 3 / 2, src_w * src_h / 2);
                break;
            case AV_PIX_FMT_YUV444P:
                memcpy(src_data[0], temp_buffer, src_w * src_h);
                memcpy(src_data[1], temp_buffer + src_w * src_h, src_w * src_h);
                memcpy(src_data[2], temp_buffer + src_w * src_h * 2, src_w * src_h);
                break;
            case AV_PIX_FMT_YUYV422:
                memcpy(src_data[0], temp_buffer, src_w * src_h * 2);
                break;
            case AV_PIX_FMT_RGB24:
                memcpy(src_data[0], temp_buffer, src_w * src_h * 3);
                break;
            default:
                printf("Not Support input pixel Format.\n");    
                break;
        }

        sws_scale(img_convert_ctx, src_data, src_linesize, 0, src_h, dst_data, dst_linesize);
        printf("Finish process frame %5d\n", frame_idx);
        frame_idx++;

        switch(dst_pixfmt) {
        case AV_PIX_FMT_GRAY8:
            fwrite(dst_data[0], 1, dst_w * dst_h, dst_file);
            break;
        case AV_PIX_FMT_YUV420P:
            fwrite(dst_data[0], 1, dst_w * dst_h, dst_file);
            fwrite(dst_data[1], 1, dst_w * dst_h / 4, dst_file);
            fwrite(dst_data[2], 1, dst_w * dst_h / 4, dst_file);    
            break;
        case AV_PIX_FMT_YUV422P:
            fwrite(dst_data[0], 1, dst_w * dst_h, dst_file);
            fwrite(dst_data[1], 1, dst_w * dst_h / 2, dst_file);
            fwrite(dst_data[2], 1, dst_w * dst_h / 2, dst_file);    
            break;
        case AV_PIX_FMT_YUV444P:
            fwrite(dst_data[0], 1, dst_w * dst_h, dst_file);
            fwrite(dst_data[1], 1, dst_w * dst_h, dst_file);
            fwrite(dst_data[2], 1, dst_w * dst_h, dst_file);    
            break;
        case AV_PIX_FMT_YUYV422:
            fwrite(dst_data[0], 1, dst_w * dst_h * 2, dst_file);
            break;
        case AV_PIX_FMT_RGB24:
            fwrite(dst_data[0], 1, dst_w * dst_h * 3, dst_file);
            break;  
         default:
                printf("Not Support output pixel Format.\n");    
                break;      
        }
    }

    sws_freeContext(img_convert_ctx);
    free(temp_buffer);
    fclose(src_file);
    fclose(dst_file);
    av_freep(&src_data[0]);
    av_freep(&dst_data[0]);

    return 0;
}