protoc -I ./ --grpc_out=./ --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $1
protoc -I ./ --cpp_out=./ $1