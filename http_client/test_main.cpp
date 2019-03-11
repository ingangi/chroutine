#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#include "engin.hpp"
#include "curl_req.h"
#include "curl_stub.h"
 
int main(int argc, char *argv[])
{
    ENGINE_INIT(1);
    ENGIN.create_chroutine([](void *){
        curl_stub_t *client = new curl_stub_t;
        while (1) {
            std::shared_ptr<curl_rsp_t> rsp = client->exec_curl("http://...");
            std::cout << "rsp code:" << rsp.get()->get_rsp_code() << std::endl;
            SLEEP(5000);
        }
        delete client;
    }, nullptr);

    ENGIN.run();
    std::cout << "over ..." << std::endl;
}