cd third-party/pistache
mkdir -p build
cd build
make -j16
sudo make install

cd ../../../
cd third-party/librdkafka
mkdir -p build
cd build
make -j16
sudo make install

cd ../../../
cd third-party/cppkafka
mkdir -p build
cd build
make -j16
sudo make install

cd ../../../
cd third-party/json
mkdir -p build
cd build
make -j16
sudo make install

