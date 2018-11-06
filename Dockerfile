FROM trzeci/emscripten:sdk-tag-1.38.15-64bit
MAINTAINER Joel Martin <github@martintribe.org>

RUN dpkg --add-architecture i386 && \
    apt-get -y update && \
    apt-get -y install git-core cmake g++ \
        lib32gcc-6-dev libsdl2-dev:i386 libsdl2-image-dev:i386 libedit-dev:i386

RUN git clone https://github.com/WebAssembly/binaryen/ && \
    cd binaryen && \
    cmake . && make && \
    make install

RUN git clone --recursive https://github.com/WebAssembly/wabt/ && \
    cd wabt && \
    make gcc-release && \
    make install-gcc-release && \
    cp /src/wabt/bin/* /usr/local/bin/

# Cache emscripten port of SDL2
RUN echo 'BINARYEN_ROOT="/usr/local"' >> /root/.emscripten && \
    echo 'RELOCATABLE=""' >> /root/.emscripten && \
    echo "int main() {}" > /tmp/nop.c && \
    emcc -s WASM=1 -s SIDE_MODULE=1 -O2 -s USE_SDL=2 /tmp/nop.c -o /tmp/nop.wasm && \
    emcc --show-ports && \
    rm /tmp/nop*

    #rm -r /root/.emscripten_cache* && \

# To make sure emcc registers a timestamp difference properly do this
# as a separate run command
RUN touch /root/.emscripten_sanity

# Additional tools for building wac/wace and wace OS
RUN apt-get -y install python3 nasm xorriso grub-common grub-pc-bin


# TODO: combine with top install
RUN apt-get -y install freeglut3-dev:i386
