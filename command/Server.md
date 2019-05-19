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
