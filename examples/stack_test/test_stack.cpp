#include "engine.hpp"

using namespace chr;

void test_stack() {
    for (int i = 0; i < 10000; i++) {
        ENGIN.create_chroutine([](void *){
            while (1) {
                SLEEP(1000);
            }
        }, nullptr);
    }
}


int main(int argc, char **argv)
{
    ENGINE_INIT(1);

    test_stack();

    ENGIN.run();
}