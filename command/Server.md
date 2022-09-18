# Server command

## UDP Server
create udp server
~~~
ffmpeg -re -i video.h264 -vcodec copy -f h264 udp://127.0.0.1:6666
~~~

create udp client
~~~
ffplay -f h264 udp://127.0.0.1:6666
~~~

## RTP Server
create rtp server
~~~
ffmpeg -re -i video.h264 -vcodec copy -f rtp rtp://127.0.0.1:6666
~~~

## RTMP server
pull rtmp server
you need deploy rtmp + nginx + ffmpeg server
- start nginx server　<br>
sudo nginx
- live: deploy rtmp path
~~~
ffmpeg -i test.flv -f flv rtmp://127.0.0.1/live/test
~~~


## RTSP server
FFMPEG + EasyDarwin
- start EasyDarwin <br>
sh start.sh
- stop EasyDarwin <br>
sh stop.sh
- configure EasyDarwin <br>
easydarwin.ini
~~~
ffmpeg -re -i test.mp4 -f rtsp -rtsp_transport tcp rtsp://127.0.0.1/live.sdp
~~~

## rtmp+nvenc推流
~~~
ffmpeg -y -vsync 0 -pix_fmt yuv420p -s 3840*2160 -r 60 -i videoplayback.yuv -c:v h264_nvenc -b:v 8M -f flv rtmp://127.0.0.1:1935/mylive/ets
~~~
