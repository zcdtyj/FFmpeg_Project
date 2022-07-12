# Command

## Video Encode and Decode
### H264(libx264):
encode
~~~
ffmpeg -s 320x180 -i test_yuv420p_320x180.yuv -b:v 1024k -r 25 -vcodec libx264 test_yuv420p_320x180.h264
~~~

decode
~~~
ffmpeg -i test_yuv420p_320x180.h264 test_yuv420p_320x180.yuv
~~~

###h264_mp4toannexb and hevc_mp4toannexb
~~~
ffmpeg -i INPUT.mp4 -codec copy -bsf:v h264_mp4toannexb OUTPUT.ts
~~~

### MPEG2
encode
~~~
ffmpeg -s 320x180 -i test_yuv420p_320x180.yuv -b:v 1024k -r 25 -vcodec mpeg2video test_yuv420p_320x180.mpg
~~~

decode
~~~
ffmpeg -i test_yuv420p_320x180.mpg test_yuv420p_320x180.yuv
~~~


## Extract data
### PCM
extract audio data
~~~
ffmpeg -i input.mp4 -vn -acodec copy output.aac
~~~

### H.264
extract video data
~~~
ffmpeg -i input.mp4 -an -vcodec copy output.h264
~~~


## Audio Encode and Decode
### MP3
aac to mp3
~~~
ffmpeg -i audio.aac -c:a libmp3lame -ac 2 -q:a 2 audio.mp3
~~~

# Muxer and Demuxer
## Convert
### Muxer
flv to mp4
~~~
ffmpeg -i input.flv -c copy -f mp4 output.mp4
~~~


# Scale
## scale
### scale command
vga to FHD
~~~
ffmpeg -i test.mp4 -vf scale=1920:1080 test_1920x1080.mp4
~~~

# Tool
## fragment
Take a Fragment
~~~
ffmpeg -ss 00:03:00 -i video.mp4 -to 00:02:00 -c copy cut.mp4
~~~

# ffprobe
## show information
### information
~~~
ffprobe -show_data test.mp4
ffprobe -show_packets test.mp4
ffprobe -show_streams test.mp4
~~~

### output format
support XML, INI, JSON, CSV, FLAT
~~~
ffprobe -of json -show_streams test.mp4
~~~
