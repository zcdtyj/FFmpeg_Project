#include <stdio.h>
#include <stdlib.h>

#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>

int main(int argc, char *argv[])
{
    const char *op = NULL;
    int ret = -1;

    //enable ffmpeg log
    av_log_set_level(AV_LOG_TRACE);
    av_log(NULL, AV_LOG_DEBUG, "hello, my name is zcd!\n");
    av_log(NULL, AV_LOG_ERROR, "error!\n");
    av_log(NULL, AV_LOG_TRACE, "xxxxxxxxxxx!\n");
    
    return 0;
}