#include "engine.hpp"

using namespace chr;

void test_stack() {
    for (int i = 0; i < 1000; i++) {
        ENGIN.create_chroutine([i](void *){
            while (1) {
                SLEEP(i+10);
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