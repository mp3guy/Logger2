Logger2
=======

Tool for logging RGB-D data from the ASUS Xtion Pro Live with OpenNI2

Should build on Linux, MacOS and Windows. Haven't built on Windows yet though, so someone feel free try it out!

Requires CMake, Boost, Qt4, OpenNI2, ZLIB and OpenCV. 

Grabs RGB and depth frames which can then be compressed (lossless ZLIB on depth and JPEG on RGB) or uncompressed. 

Frames can be cached in memory and written out when logging is finished, or streamed to disk. 

Multiple threads are used for the frame grabbing, compression and GUI. A circular buffer is used to help mitigate synchronisation issues that may occur. 

Supports disabling auto settings on the camera. 

The binary format is specified in Logger2.h

Run with -t to enable TCP streaming support on port 5698

<p align="center">
  <img src="http://mp3guy.github.io/img/Logger2.png" alt="Logger2"/>
</p>


