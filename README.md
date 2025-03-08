## RtspWebPlayer
use ffmpeg wasm play h264 frame from websocket

## build

```
./scripts/wasmenv.sh

cd ..
mkdir build
cd build
emcmake cmake ..
make -j8
```