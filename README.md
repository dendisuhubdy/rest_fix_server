# REST API Server

[![Build Status](https://travis-ci.com/bitwyre/rest_fix_server.svg?branch=master)](https://travis-ci.com/bitwyre/rest_fix_server)

## Introduction

This is the REST API Server that takes in POST request from the front end and convert them to FIX messages

## Dependencies

On Ubuntu do

```bash
sudo apt install libboost-all-dev librdkafka-dev libzstd-dev libsasl2-dev libssl-dev zlib1g-dev
```

We don't have support for MacOS yet.

## Build

```bash
mkdir -p  build
cd buld
cmake ..
make -j8
```

## Run

```bash
./build/rest_api_server
```

## Sending POST Request

```bash
# insert order
curl -d "{ \"symbol\": \"BTCUSD\", \"quantity\": 20, \"price\": 20 }"  -X POST http://xxxx:port/insertorder
# cancel order
curl -d "{ \"symbol\": \"BTCUSD\", \"quantity\": 20, \"price\": 20 }"  -X POST http://xxx:port/cancelorder
```

# Copyright

2019, Bitwyre Technologies LLC
