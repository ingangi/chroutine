#include "engine.hpp"
#include "raw_tcp_server.hpp"

using namespace chr;

int main(int argc, char **argv)
{
    ENGINE_INIT(3);

    ENGIN.create_chroutine([&](void *){
        raw_tcp_server_t* server = static_cast<raw_tcp_server_t*>(raw_tcp_server_t::create("0.0.0.0", "50061").get());
        if (server) {
            server->start();
            while (1) {
                raw_data_block_sptr_t d = nullptr;
                server->read(d);
                if (d) {
                    SPDLOG(INFO, "data from fd {}, length {}: {}", d->m_key->get_fd(), d->m_len, (const char*)(d->m_buf));
                    server->write(d->m_key, d->m_buf, d->m_len);
                }
            }
        }
        SPDLOG(INFO, "test chroutine exit");
    }, nullptr);

    ENGIN.run();
}