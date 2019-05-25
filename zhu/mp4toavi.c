#include <stdio.h>
#include <libavformat/avformat.h>

int main(int argc, char *argv[]) 
{
    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ifmt_ctx = NULL; *ofmt_ctx = NULL;
    AVPacket pkt;
    const char *in_filename, *out_filename;
    int ret, i;
    int frame_index = 0;

    in_filename = "../resource/test,mp4";
    out_filename = "../resource/test.avi";
    
    av_register_all();
    if((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        printf("Could not open input file.\n");
        goto end;
    }

    
}