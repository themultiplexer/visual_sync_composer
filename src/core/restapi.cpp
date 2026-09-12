#include "restapi.h"



static bool running = true;

void RestApi::start_server() {
    auto const address = net::ip::make_address("0.0.0.0");
    unsigned short port = static_cast<unsigned short>(8080);

    net::io_context ioc{1};

    tcp::acceptor acceptor{ioc, {address, port}};
    tcp::socket socket{ioc};
    http_server(acceptor, socket);

    ioc.run();
}