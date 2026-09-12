#ifndef RESTAPI_H
#define RESTAPI_H

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>

#include "core/effectpresetmodel.h"

using namespace std;
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class RestApi {
    void start_server();
};

std::string vector_to_json_string(const std::vector<int>& vec)
{
    nlohmann::json json_array = nlohmann::json::array();
    for (size_t i = 0; i < vec.size(); ++i) {
        json_array[i] = vec[i];
    }
    return json_array.dump();
}

std::string string_vector_to_json_string(const std::vector<EffectPresetModel *>& vec)
{
    nlohmann::json json_array = nlohmann::json::array();

    for (size_t i = 0; i < vec.size(); ++i) {
        nlohmann::json obj;
        obj["name"] = vec[i]->name;
        obj["id"] = i; // vec[i].id
        //obj["active"] = vec[i].active;//(activeEffect == static_cast<int>(i))
        json_array.push_back(obj);
    }

    return json_array.dump();  // serialize to string
}

namespace my_program_state
{
    std::size_t
    request_count()
    {
        static std::size_t count = 0;
        return ++count;
    }

    std::time_t
    now()
    {
        return std::time(0);
    }
}

class http_connection : public std::enable_shared_from_this<http_connection>
{
public:
    http_connection(tcp::socket socket)
        : socket_(std::move(socket))
    {
    }

    // Initiate the asynchronous operations associated with the connection.
    void
    start()
    {
        read_request();
        check_deadline();
    }

private:
    tcp::socket socket_;
    beast::flat_buffer buffer_{8192};
    http::request<http::string_body> request_;
    http::response<http::dynamic_body> response_;

    net::steady_timer deadline_{
        socket_.get_executor(), std::chrono::seconds(60)};

    void
    read_request()
    {
        auto self = shared_from_this();

        http::async_read(
            socket_,
            buffer_,
            request_,
            [self](beast::error_code ec,
                std::size_t bytes_transferred)
            {
                boost::ignore_unused(bytes_transferred);
                if(!ec)
                    self->process_request();
            });
    }

    // Determine what needs to be done with the request message.
    void
    process_request()
    {
        response_.version(request_.version());
        response_.keep_alive(false);

        switch(request_.method())
        {
        case http::verb::post:

            if(request_.target() == "/")
            {
                response_.result(http::status::ok);
                response_.set(http::field::server, "Beast");
                response_.set(http::field::content_type, "text/html");

                    try {
                        const std::string& body = request_.body();
                        nlohmann::json action = nlohmann::json::parse(body);
                        std::cout << "Received JSON: " << action.dump().c_str() << std::endl;
                        std::cout << action["value"] << std::endl;
                        //audioWindow->setNewEffect(action["value"]);
                    } catch (...) {
                        std::cerr << "Failed to parse JSON." << std::endl;
                    }
                    response_.set("Access-Control-Allow-Origin", "*");
                    beast::ostream(response_.body()) << "Got action";

            } else {
                response_.result(http::status::not_found);
                response_.set(http::field::server, "Beast");
            }
            break;

        case http::verb::get:

            if(request_.target() == "/effects")
            {
                response_.result(http::status::ok);
                response_.set(http::field::server, "Beast");
                response_.set(http::field::content_type, "text/html");

                // TODO seperate UI :)
                std::vector<EffectPresetModel *> presets = EffectPresetModel::readJson<EffectPresetModel, 100>("effects.json");
                beast::ostream(response_.body())<< string_vector_to_json_string(presets);
                response_.set("Access-Control-Allow-Origin", "*");
            } else {
                response_.result(http::status::not_found);
                response_.set(http::field::server, "Beast");
            }
            break;

        case http::verb::options:
            response_.result(http::status::ok);
            response_.set(http::field::server, "Beast");
            response_.set("Allow", "GET, POST, OPTIONS");
            response_.set("Access-Control-Allow-Origin", "*");
            response_.set("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            response_.set("Access-Control-Allow-Headers", "Content-Type, Content-Length, Authorization");
            break;

        default:
            // We return responses indicating an error if
            // we do not recognize the request method.
            response_.result(http::status::bad_request);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body())
                << "Invalid request-method '"
                << std::string(request_.method_string())
                << "'";
            break;
        }

        write_response();
    }

    // Asynchronously transmit the response message.
    void
    write_response()
    {
        auto self = shared_from_this();

        response_.content_length(response_.body().size());

        http::async_write(
            socket_,
            response_,
            [self](beast::error_code ec, std::size_t)
            {
                self->socket_.shutdown(tcp::socket::shutdown_send, ec);
                self->deadline_.cancel();
            });
    }

    // Check whether we have spent enough time on this connection.
    void
    check_deadline()
    {
        auto self = shared_from_this();

        deadline_.async_wait(
            [self](beast::error_code ec)
            {
                if(!ec)
                {
                    // Close socket to cancel any outstanding operation.
                    self->socket_.close(ec);
                }
            });
    }
};

// "Loop" forever accepting new connections.
void
http_server(tcp::acceptor& acceptor, tcp::socket& socket)
{
  acceptor.async_accept(socket,
      [&](beast::error_code ec)
      {
          if(!ec)
              std::make_shared<http_connection>(std::move(socket))->start();
          http_server(acceptor, socket);
      });
}



#endif