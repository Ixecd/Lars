rm -rf build
cmake -B build -DCMAKE_PREFIX_PATH=/usr/local/protobuf
cmake --build build
rm -rf build