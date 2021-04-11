FROM debian:buster

RUN apt update && \
    apt install -y \
        gcc-8 g++-8 cmake \
        libgstreamermm-1.0 libgstreamermm-1.0-dev \
        libgtkmm-3.0-dev libgtkmm-3.0-1v5

RUN apt install -y git clang-format

RUN useradd -U -m -s /bin/bash user
