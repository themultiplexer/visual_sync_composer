
## macOS

```
mkdir VSC
pushd VSC

git clone --recursive https://github.com/themultiplexer/visual_sync_composer

brew install cmake qt6 libomp gcc libpng pkgconfig fftw cpprestsdk nlohmann-json hidapi boost glew glfw3

git clone https://github.com/mborgerding/kissfft.git
pushd kissfft
mkdir build
pushd build
cmake ..
make -j
make install
popd
popd

git clone https://github.com/thestk/rtaudio
pushd rtaudio
mkdir build
pushd build
cmake ..
make -j
make install
popd
popd

git clone --recursive https://github.com/themultiplexer/visual_sync_composer

pushd visual_sync_composer/build
cmake ..
make -j

./VisualSyncComposer.app/Contents/MacOS/VisualSyncComposer

```