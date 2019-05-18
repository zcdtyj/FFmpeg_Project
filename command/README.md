# H264(libx264):

encode
~~~
ffmpeg -s 320x180 -i test_yuv420p_320x180.yuv -b:v 1024k -r 25 -vcodec libx264 test_yuv420p_320x180.h264
~~~

decode
~~~
ffmpeg -i test_yuv420p_320x180.h264 test_yuv420p_320x180.yuv
~~~
