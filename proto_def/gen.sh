protoc -I ./ --grpc_out=../proto_code --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $1
protoc -I ./ --cpp_out=../proto_code $1
