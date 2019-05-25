# Readme

## Introduction

This is an example using [Pistache](https://github.com/oktal/pistache) library  to serve an OpenCV image filter. This example
use a compiled version of OpenCV 4.0.0 and requires following packages:

- OpenCV 4.0.0
- Crow Library v0.1
- GCC with support to C++17
- CMake 1.13

## How to build

After building and install OpenCV 4.0.0, GCC and CMake, to build this application you should follow this steps:

```
git clone https://github.com/arthurafarias/microservice-opencv-filter-gaussian
cd microservice-opencv-filter-gaussian
git submodule update --init --recursive
mkdir build
cd build
cmake ..
make -$(nproc)
```

### Testing

After building, to start the service, you should issue following command inside build folder:

```
./ocv_microservice_crow
```

ocv_microservice_crow is the executable that gonna serve the microservice on 18080 tcp port.

To test the service you should submit a POST request to http://localhost:18080/filters/gaussian. The body of this request should be your binary PNG image. The response should be a gaussian filtered image.
