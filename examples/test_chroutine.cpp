#include <iostream>
#include <unistd.h>
#include <thread>
#include "engine.hpp"

using namespace chr;

void fake_io_work(int costtime = 10)
{    
    std::time_t now = get_time_stamp();
    while (get_time_stamp() - now < costtime) {
        SLEEP(20);
    }
}

typedef struct test_return_data_t{
    std::string b;
    int a;
}test_return_data_t;

void fun_1(void *arg)
{
    int tick = 0;
    while (1) {
        tick++;
        LOG << "fun_1 tick = " << tick << " (" << std::this_thread::get_id() << ")" << std::endl;
        fake_io_work(1000);

        if (tick % 3 == 0) {
            // example of sync with son chroutine: 
            // call a `son` chroutine in the `father` chroutine, and get the result after son is done.

            // if timeout happens during son's running, son will be stopped and removed immediately            
            reporter_base_t * rpt = ENGIN.create_son_chroutine([](void *d) {
                test_return_data_t *data = (test_return_data_t *)d;
                LOG << "son-of-func_1 (" << std::this_thread::get_id() << ") " << get_time_stamp() << std::endl;
                fake_io_work(5000);
                data->a = 888;
                data->b = "hello father";
                LOG << "son-of-func_1 (" << std::this_thread::get_id() << ") OVER " << get_time_stamp() << std::endl;
            }, reporter_t<test_return_data_t>::create(), 5100);

            // son is over, check the result
            if (rpt) {
                LOG << "fun_1 (" << std::this_thread::get_id() << ") finish wait, son result:" << rpt->get_result() << std::endl;
                if (rpt->get_result() == result_done) {
                    test_return_data_t *p_data = static_cast<test_return_data_t*>(rpt->get_data());
                    LOG << "son chroutine is done, test_return_data_t.a=" << p_data->a << ", b=" << p_data->b << std::endl;
                }
            }
        }
    }
}

void fun_3(void *arg)
{
    while (1) {
        LOG << __FUNCTION__ << " running ...\n";
        //SLEEP(2000);
        usleep(5000000);  //testing block os thread
        break;
    }
}

int main(int argc, char **argv)
{
    ENGINE_INIT(2);

    // ENGIN.create_chroutine(fun_1, nullptr);  
    ENGIN.create_chroutine([](void *){
    int tick = 0;
        while (1) {
            LOG << "fun_2 tick = " << ++tick << " (" << std::this_thread::get_id() << ")" << std::endl;
            SLEEP(1000);

            // create another chroutine in the same thread
            if (tick == 2) {
                LOG << "try to create another chroutin\n";
                ENGIN.create_son_chroutine(fun_3, nullptr);
            }
        }
    }, nullptr);

    ENGIN.run();    
    LOG << "over ..." << std::endl;
}