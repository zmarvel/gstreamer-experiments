
# camcoder

Camcoder is a configurable encoder for live video streams. It's designed to make it easy to go
from interfacing with a new, unencoded video stream to viewing the image feed in software like
VLC.

## Pre-release

This is a development release. Some features of this project, including the configuration file,
plugin API, and library API, are not yet stable.


## Project goals

- Hardware-accelerated encoding
- Streaming file formats:
  - HLS
  - DASH
- Multiple sources
- Different source types:
  - Files
  - TCP server
  - TCP client
  - Plugins
- Frame manipulation
  - C plugin API
  - Scriptable plugin API
- Portability
  - Linux
  - Windows
  - BSD
- Library interface for integration with other software


## Project non-goals

- Audio encoding


## Contributing

The issue tracker is hosted on the [project Gitea instance](http://natu.zackmarvel.com:3000/zack/camcoder).


### Building

For now, I'm building on Debian Buster. The Dockerfile should indicate the build dependencies.

```
mkdir build
cd build
cmake ..
make
```


## gstreamer-tutorials

Using [gstreamermm][gstreamermm] to implement [the tutorials in the GStreamer documentation][gstreamer_tutorials] in
C++.

Uploaded the [gstreamermm docs][gstreamermm_docs] to my website for easy reference.


[gstreamermm]: https://gitlab.gnome.org/GNOME/gstreamermm
[gstreamer_tutorials]: https://gstreamer.freedesktop.org/documentation/tutorials/index.html
[gstreamermm_docs]: http://www.zackmarvel.me/static/gstreamermm-1.10.0/doc/index.html
