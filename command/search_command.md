## ffmpeg支持的所有编码器/解码器/封装/解封装/协议

###编码器/解码器
~~~
./configure --list-encoders

./configure --list-decoders
~~~

### 封装/解封装
~~~
./configure --list-muxers

./configure --list-demuxers
~~~

### 协议
~~~
./configure --list-protocols
~~~

---

## ffmpeg查看所有的参数
~~~
ffmpeg -h full
~~~

### ffmpeg查看所有支持的formats/encoders/decoders
~~~
ffmpeg -formats
ffmpeg -encoders
ffmpeg -decoders
ffmpeg -filters
~~~

---
### ffmpeg查看某一中特定的类型
~~~
ffmpeg -h muxer=flv
ffmpeg -h demuxer=flv
ffmpeg -h encoder=libx264
ffmpeg -h decoder=libx264
ffmpeg -h filter=colorkey
~~~
