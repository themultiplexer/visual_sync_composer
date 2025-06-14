#include <GL/glew.h>
#include "audiowindow.h"
#include "downloader.h"
#include "mainwindow.h"
#include "thread.h"

#include <QGuiApplication>
#include <QApplication>
#include <QtNetwork/QNetworkReply>
#include <QThread>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGuiApplication>
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QImage>
#include <QGraphicsPixmapItem>
#include <QMenu>
#include <QAction>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QImage>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QThread>
#include <thread>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <ranges>

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

AudioWindow *audioWindow;

std::string vector_to_json_string(const std::vector<int>& vec)
{
    json::value json_array = json::value::array();

    for (size_t i = 0; i < vec.size(); ++i) {
        json_array[i] = json::value::number(vec[i]);
    }

    return json_array.serialize();
}

std::string string_vector_to_json_string(const std::vector<std::string>& vec)
{
    json::value json_array = json::value::array();
    for (size_t i = 0; i < vec.size(); ++i) {
        json::value obj = json::value::object();
        obj[U("name")] = json::value::string(vec[i]);
        obj[U("id")] = json::value::number(i);
        obj[U("active")] = json::value::boolean(audioWindow->activeEffect == i);
        json_array[i] = obj;
    }

    return json_array.serialize();
}

void handle_get(http_request request) {
    auto path = uri::decode(request.relative_uri().path());
    std::cout << "get" << std::endl;
    if(path == U("/test")) {
        http_response jsonresponse(status_codes::OK);
        jsonresponse.headers().add(U("Access-Control-Allow-Origin"), U("*"));
        std::vector<EffectPresetModel *> presets = EffectPresetModel::readJson("effects.json");
        auto view = presets | std::views::transform([](EffectPresetModel* item) { return item->name; });
        std::vector<std::string> my_vector(view.begin(), view.end());
        std::string json_str = string_vector_to_json_string(my_vector);
        jsonresponse.set_body(json_str);
        request.reply(jsonresponse);
    }
    http_response response(status_codes::NotFound);
    //jsonresponse.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.set_body(U("Not found"));
    request.reply(response);
}

void handle_post(http_request request) {
    auto path = uri::decode(request.relative_uri().path());
    std::cout << "post" << std::endl;
    request.extract_json().then([=](pplx::task<json::value> task) {
            if(path == U("/")) {
                try {
                    json::value json_data = task.get();
                    std::wcout << L"Received JSON: " << json_data.serialize().c_str() << std::endl;
                    std::cout << json_data["value"] << std::endl;
                    std::vector<EffectPresetModel *> presets = EffectPresetModel::readJson("effects.json");
                    audioWindow->setNewEffect(presets[json_data["value"].as_integer()]);
                } catch (...) {
                    std::cerr << "Failed to parse JSON." << std::endl;
                }
                http_response jsonresponse(status_codes::OK);
                jsonresponse.headers().add(U("Access-Control-Allow-Origin"), U("*"));
                jsonresponse.set_body(U("Received POST"));
                request.reply(jsonresponse);
            }


    });
}

void handle_option(http_request request) {
    http_response response (status_codes::OK);
    response.headers().add(U("Allow"), U("GET, POST, OPTIONS"));
    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.headers().add(U("Access-Control-Allow-Methods"), U("GET, POST, OPTIONS"));
    response.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type, Content-Length, Authorization"));
    request.reply(response);
}

static bool running = true;

void start_server() {
    http_listener listener(U("http://0.0.0.0:8080"));
    listener.support(methods::POST, handle_post);
    listener.support(methods::GET, handle_get);
    listener.support(methods::OPTIONS, handle_option);

    try {
        listener
            .open()
            .then([] { std::cout << "Server is listening on http://localhost:8080\n"; })
            .wait();

        // Keep thread alive while server is running
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        listener.close().wait();
        std::cout << "Server stopped.\n";
    } catch (std::exception &e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
}

std::array<uint8_t, 6> my_mac = {0xDC, 0x4E, 0xF4, 0x0A, 0x3F, 0x9F};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("fusion");

    QPalette palette = QPalette();
    palette.setColor(QPalette::Window, QColor(53, 53, 53));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor(25, 25, 25));
    palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    palette.setColor(QPalette::ToolTipBase, Qt::black);
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor(53, 53, 53));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, Qt::black);
    a.setPalette(palette);

    WifiEventProcessor *ep = new WifiEventProcessor(my_mac, "wlxdc4ef40a3f9f");

    audioWindow = new AudioWindow(ep);
    audioWindow->setWindowTitle("Hello World");
    audioWindow->resize(1920, 1080);
    audioWindow->show();

    std::thread server_thread(start_server);
    /*
    MainWindow *mainWindow = new MainWindow(ep);
    mainWindow->setWindowTitle("Hello World");
    mainWindow->resize(1920, 1080);
    mainWindow->show();
    */
    return a.exec();
    running = false;
}
