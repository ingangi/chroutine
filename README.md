# usage
With `chroutine`, we can easily develop highly concurrent programs in C++ and write code with synchronization semantics.

You can run the examples and see the code how to use it:
```
├── test.cpp  (genaral use of chroutine)
├── http_client_example  (http client)
├── rpc_example  (rpc server and client)

make ALLTEST
```

# env install (Relies on gRPC and libcurl)

## install grpc

my system: CentOS Linux release 7.4.1708

### get ready

```shell
sudo yum install autoconf automake autogen libtool shtool build-essential pkg-config libgflags-dev libgtest-dev clang libc++-dev
```

### build and install

```shell
git clone https://github.com/grpc/grpc
cd grpc
git submodule update --init
make
[sudo] make install
```


### error and solution

if get configure error: 

> configure in doc/example fails with "cannot find install-sh, install.sh, or shtool in "." "./.." "./../..""

try:

```shell
cd third_party/protobuf; 
autoreconf -i -v -f
```

`git submodule update --init` will download third party code, try to download yourself and put them in right place if failed in China:

```shell
Submodule 'third_party/abseil-cpp' (https://github.com/abseil/abseil-cpp) registered for path 'third_party/abseil-cpp'
Submodule 'third_party/benchmark' (https://github.com/google/benchmark) registered for path 'third_party/benchmark'
Submodule 'third_party/bloaty' (https://github.com/google/bloaty.git) registered for path 'third_party/bloaty'
Submodule 'third_party/boringssl' (https://github.com/google/boringssl.git) registered for path 'third_party/boringssl'
Submodule 'third_party/boringssl-with-bazel' (https://boringssl.googlesource.com/boringssl) registered for path 'third_party/boringssl-with-bazel'
Submodule 'third_party/cares/cares' (https://github.com/c-ares/c-ares.git) registered for path 'third_party/cares/cares'
Submodule 'third_party/gflags' (https://github.com/gflags/gflags.git) registered for path 'third_party/gflags'
Submodule 'third_party/googletest' (https://github.com/google/googletest.git) registered for path 'third_party/googletest'
Submodule 'third_party/protobuf' (https://github.com/google/protobuf.git) registered for path 'third_party/protobuf'
Submodule 'third_party/zlib' (https://github.com/madler/zlib) registered for path 'third_party/zlib'
```

## install protoc

it was build but not installed:

```shell
cd third_party/protobuf
sudo make install
```

### try to generate protobuf code

```shell
cd proto_def
sh gen.sh test.proto
```

You should see four files:

```shell
├── proto_code
│   ├── test.grpc.pb.cc
│   ├── test.grpc.pb.h
│   ├── test.pb.cc
│   └── test.pb.h
```

## install libcurl

see:

```
https://github.com/curl/curl.git
```