#ifndef TEST_CLIENT_H
#define TEST_CLIENT_H

// `Test` service is defined in proto_def/test.proto

#include "test.grpc.pb.h"
#include "grpc_sync_client_for_chroutine.hpp"

template <> int call_Service_API<rpcpb::Test, rpcpb::TestReq, rpcpb::TestRsp>::call_impl(); // for API: Test::HowAreYou

#endif