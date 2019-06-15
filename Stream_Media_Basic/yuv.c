#include <stdio.h>

/**
 * Split Y, U, V planes in YUV420P file.
 * @param url  Location of Input YUV file.
 * @param w    Width of Input YUV file.
 * @param h    Height of Input YUV file.
 * @param num  Number of frames to process.
 *
 */
int simple_yuv420_split(char *url, int w, int h, int num) 
{
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_420_y.y", "wb+");
    FILE *fp2 = fopen("output_420_u.u", "wb+");
    FILE *fp3 = fopen("output_420_v.v", "wb+");

    unsigned char *pic = (unsigned char *)malloc(w * h * 3 / 2);
    for(int i = 0; i < num; i++) {
        fread(pic, 1, w * h * 3 / 2, fp);
        fwrite(pic, 1, w * h, fp1);
        fwrite(pic + w * h, 1, w * h / 4, fp2);
        fwrite(pic + w * h * 5 / 4, 1, w * h / 4, fp3);
    }

    free(pic);
    fclose(fp);
    fclose(fp1);
    fclose(fp2);
    fclose(fp3);

    return 0;
}


/**
 * Split Y, U, V planes in YUV444P file.
 * @param url  Location of YUV file.
 * @param w    Width of Input YUV file.
 * @param h    Height of Input YUV file.
 * @param num  Number of frames to process.
 *
 */
int simplest_yuv444_split(char *url, int w, int h, int num)
{
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_444_y.y", "wb+");
    FILE *fp2 = fopen("output_444_u.u", "wn+");
    FILE *fp3 = fopen("output_444_v.v", "wb+");
    unsigned char *pic = (unsigned char *)malloc(w * h * 3);

    for(int i = 0; i < num; i++) {
        fread(pic, 1, w * h * 3, fp);
        fwrite(pic, 1, w * h, fp1);
        fwrite(pic + w * h, 1, w * h, fp2);
        fwrite(pic + 2 * w * h, 1, w * h, fp3);
    }

    free(pic);
    fclose(fp);
    fclose(fp1);
    fclose(fp2);
    fclose(fp3);

    return 0;
}


/**
 * Convert YUV420P file to gray picture
 * @param url     Location of Input YUV file.
 * @param w       Width of Input YUV file.
 * @param h       Height of Input YUV file.
 * @param num     Number of frames to process.
 */
int simplest_yuv420_gray(char *url, int w, int h, int num) 
{
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_gray.yuv", "wb+");
    unsigned char *pic = (unsigned char *)malloc(w * h * 3 / 2);

    for(int i = 0; i < num; i++) {
        fread(pic, 1, w * h * 3 / 2, fp);
        memset(pic + w * h, 128, w * h / 2);
        fwrite(pic, 1, w * h * 3 / 2, fp1);
    }

    free(pic);
    fclose(fp);
    fclose(fp1);

    return 0;
}


/**
 * Halve Y value of YUV420P file
 * @param url     Location of Input YUV file.
 * @param w       Width of Input YUV file.
 * @param h       Height of Input YUV file.
 * @param num     Number of frames to process.
 */
int simplest_yuV420_halfy(char *url, int w, int h, int num) {
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_half.yuv", "wb+");

    unsigned char *pic = (unsigned char *)malloc(w * h * 3 / 2);
    for(int i = 0; i < num; i++) {
        fread(pic, 1, w * h * 3 / 2, fp);
        for(int j = 0; j < w * j; j++) {
            pic[j] = pic[j] / 2;
        }

        fwrite(pic, 1, w * h * 3 /2, fp1);    
    }

    free(pic);
    fclose(fp);
    fclose(fp1);

    return 0;
}

int main(int argc, char *argv[]) 
{
    simple_yuv420_split("/home/zhucd/FFmpeg_Project/resource/test_yuv420p_320x180.yuv", 320, 180, 1);

    return 0;
}