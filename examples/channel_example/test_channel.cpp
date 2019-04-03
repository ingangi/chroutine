#include "engine.hpp"
#include "channel.hpp"
#include "chan_selecter.hpp"

using namespace chr;

void test_channel_basic() {
    static auto chan_sptr = channel_t<int>::create(10);
    auto &chan = *(chan_sptr.get());

    // 2 read chroutines
    ENGIN.create_chroutine([&](void *){
        LOG << "reader 1 in thread:" << std::this_thread::get_id() << std::endl;
        for (int i=0; i < 20; i++) {
            int r = -1;
            LOG << "[" << i << "]reader 1 try to read r ..." << std::endl;
            chan >> r;
            LOG << "[" << i << "]reader 1 read r:" << r << std::endl;
        }
    }, nullptr);
    ENGIN.create_chroutine([&](void *){
        LOG << "reader 2 in thread:" << std::this_thread::get_id() << std::endl;
        for (int i=0; i < 20; i++) {
            int r = -1;
            LOG << "[" << i << "]reader 2 try to read r ..." << std::endl;
            chan >> r;
            LOG << "[" << i << "]reader 2 read r:" << r << std::endl;
        }
    }, nullptr);
    
    
    // 1 write chroutine
    ENGIN.create_chroutine([&](void *){
        LOG << "writer 1 in thread:" << std::this_thread::get_id() << std::endl;
        SLEEP(3000);
        for (int i=0; i < 30; i++) {
            LOG << "[" << i << "]try to write chan" << std::endl;
            chan << i;
            LOG << "[" << i << "]write chan over" << std::endl;
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
            LOG << "int read:" << int_read << std::endl;
        });
        
        chan_selecter.add_case(chan_string.get(), &string_read, [&](){
            LOG << "string read:" << string_read << std::endl;
        });
        
        chan_selecter.add_case(chan_char.get(), &char_read, [&](){
            LOG << "char read:" << char_read << std::endl;
        });

        // if nothing todo with default, do not add it
        // chan_selecter.default_case([](){
        //     LOG << "nothing to read now" << std::endl;
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
                    *(chan_int.get()) << i;
                    break;
                case 1:
                    *(chan_string.get()) << "hi~~~";
                    break;
                case 2:
                    *(chan_char.get()) << '0'+i;
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

        chan_selecter.add_case(chan_data.get(), &int_read, [&](){
            LOG << "int read:" << int_read << std::endl;
        });
        
        chan_selecter.add_case(chan_timeout.get(), &int_read, [&](){
            LOG << "read data timeout !!!" << std::endl;
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
    LOG << "over ..." << std::endl;
}