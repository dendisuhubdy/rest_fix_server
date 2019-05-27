# REST API Server

## Introduction

This is the REST API Server that takes in POST request from the front end and convert them to FIX messages

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
