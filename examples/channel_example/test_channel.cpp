#include "engine.hpp"
#include "channel.hpp"
#include "chan_selecter.hpp"

using namespace chr;

void test_channel_basic() {
    static auto chan_sptr = channel_t<int>::create(10);
    auto &chan = *chan_sptr;

    // 2 read chroutines
    ENGIN.create_chroutine([&](void *){
        SPDLOG(INFO, "reader 1 in thread:{}", readable_thread_id(std::this_thread::get_id()));
        for (int i=0; i < 20; i++) {
            int r = -1;
            SPDLOG(INFO, "[{}]reader 1 try to read r ...", i);
            chan >> r;
            SPDLOG(INFO, "[{}]reader 1 read r:{}", i, r);
        }
    }, nullptr);
    ENGIN.create_chroutine([&](void *){
        SPDLOG(INFO, "reader 2 in thread:{}", readable_thread_id(std::this_thread::get_id()));
        for (int i=0; i < 20; i++) {
            int r = -1;
            SPDLOG(INFO, "[{}]reader 2 try to read r ...", i);
            chan >> r;
            SPDLOG(INFO, "[{}]reader 2 read r:{}", i, r);
        }
    }, nullptr);
    
    
    // 1 write chroutine
    ENGIN.create_chroutine([&](void *){
        SPDLOG(INFO, "writer 1 in thread:{}", readable_thread_id(std::this_thread::get_id()));
        SLEEP(3000);
        for (int i=0; i < 30; i++) {
            SPDLOG(INFO, "[{}]try to write chan", i);
            chan << i;
            SPDLOG(INFO, "[{}]write chan over", i);
            SLEEP(1000);
        }
    }, nullptr);
}

void test_channel_select() {
    static auto chan_int = channel_t<int>::create();
    static auto chan_string = channel_t<std::string>::create();
    static auto chan_char = channel_t<char>::create();

    ENGIN.create_chroutine([&](void *){    
        int int_read = 0;
        std::string string_read;
        char char_read = '0';
        chan_selecter_t chan_selecter;

        // all callbacks of all cases will run in this chroutine
        chan_selecter.add_case(chan_int.get(), &int_read, [&](){
            SPDLOG(INFO, "int read:{}", int_read);
        });
        
        chan_selecter.add_case(chan_string.get(), &string_read, [&](){
            SPDLOG(INFO, "string read:{}", string_read);
        });
        
        chan_selecter.add_case(chan_char.get(), &char_read, [&](){
            SPDLOG(INFO, "char read:{}", char_read);
        });

        // if nothing todo with default, do not add it
        // chan_selecter.default_case([](){
        //     SPDLOG(INFO, "nothing to read now");
        //     SLEEP(100);  // if you add the default case, pay attention to yeild
        // });

        for (;;) {
            chan_selecter.select();
        }
    }, nullptr);


    ENGIN.create_chroutine([&](void *){   
        int i = 0;
        for (;;i++) {
            SLEEP(2000);
            switch (i%3)
            {
                case 0:
                    *chan_int << i;
                    break;
                case 1:
                    *chan_string << "hi~~~";
                    break;
                case 2:
                    *chan_char << '0'+i;
                    break;
            
                default:
                    break;
            }
        }

    }, nullptr);

}


void test_channel_select_timeout() {
    static auto chan_data = channel_t<int>::create();
    static auto chan_timeout = channel_t<int>::create();

    ENGIN.create_chroutine([&](void *){    
        int int_read = 0;
        chan_selecter_t chan_selecter;

        ENGIN.create_son_chroutine([&](void *) {
            SLEEP(3000);    // timeout 3 seconds
            *chan_timeout << 1;
        }, nullptr);
        
        ENGIN.create_son_chroutine([&](void *) {
            SLEEP(3100);
            *chan_data << 1;
        }, nullptr);

        chan_selecter.add_case(chan_data.get(), &int_read, [&](){
            SPDLOG(INFO, "nt read:{}", int_read);
        });
        
        chan_selecter.add_case(chan_timeout.get(), &int_read, [&](){
            SPDLOG(INFO, "read data timeout !!!");
        });

        chan_selecter.select();
    }, nullptr);
}

int main(int argc, char **argv)
{
    ENGINE_INIT(2);

    // test_channel_basic();
    // test_channel_select();
    test_channel_select_timeout();


    ENGIN.run();
}