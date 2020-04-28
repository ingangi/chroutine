#include "engine.hpp"
#include "raw_tcp_client.hpp"

using namespace chr;

int main(int argc, char **argv)
{
    ENGINE_INIT(3);

    ENGIN.create_chroutine([&](void *){
        raw_tcp_client_t* cli = static_cast<raw_tcp_client_t*>(raw_tcp_client_t::create("127.0.0.1", "50061").get());
        if (cli) {
            if (cli->connect() != 0) {
                SPDLOG(INFO, "cli->connect() failed");
            }
            std::string say = "hello server";
            while (cli->is_connected()) {                
                cli->write((byte_t*)say.data(), say.length());
                raw_data_block_sptr_t d = nullptr;
                cli->read(d);
                if (d) {
                    SPDLOG(INFO, "data read, length {}: {}", d->m_len, (const char*)(d->m_buf));
                }
                SLEEP(3000);
            }
        }
        SPDLOG(INFO, "test chroutine exit");
    }, nullptr);

    ENGIN.run();
}