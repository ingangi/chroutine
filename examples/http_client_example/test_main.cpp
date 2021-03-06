#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include "engine.hpp"

using namespace chr;
 
int main(int argc, char *argv[])
{
    ENGINE_INIT(3);
    ENGIN.create_chroutine([](void *){
        while (1) {
            std::shared_ptr<curl_rsp_t> rsp = ENGIN.exec_curl("xxx");
            if (rsp.get())
                SPDLOG(INFO, "rsp code:{}", rsp.get()->get_rsp_code());
            SLEEP(5000);
        }
    }, nullptr);

    ENGIN.run();
}