cd third-party/pistache
rm -rf build
mkdir -p build
cd build
cmake ..
make -j16
sudo make install
cd ../../../

cd third-party/librdkafka
rm -rf build
mkdir -p build
cd build
cmake ..
make -j16
sudo make install
cd ../../../

cd third-party/cppkafka
rm -rf build
mkdir -p build
cd build
cmake ..
make -j16
sudo make install
cd ../../../

cd third-party/json
rm -rf build
mkdir -p build
cd build
cmake ..
make -j16
sudo make install
cd ../../../
