#include <QGuiApplication>
#include <QApplication>
#include <QThread>
#include <QGuiApplication>
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QThread>

#include <QQmlApplicationEngine>    // Provides the engine that loads and runs QML UI modules.
#include <QQmlContext>              // Provides access to the QML context for setting context properties.

#include <QQuickWindow>

#include "core/audiocontroller.h"
#include "core/wifieventprocessor.h"
#include "ui/ogltest.h"


std::array<uint8_t, 6> my_mac = {0xDC, 0x4E, 0xF4, 0x0A, 0x3F, 0x9F};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("VisualSync"));
    QCoreApplication::setApplicationName(QStringLiteral("VisualSync"));

    WifiEventProcessor *eventProcessor = new WifiEventProcessor(my_mac, "wlxdc4ef40a3f9f");

    if (getuid() == 0) {
        printf("Dropping privs\n");
        /* process is running as root, drop privileges */
        if (setgid(1000) != 0)
            printf("setgid: Unable to drop group privileges: %s\n", strerror(errno));
        if (setuid(1000) != 0)
            printf("setuid: Unable to drop user privileges: %s\n", strerror(errno));
    }

    setenv("HOME", "/home/josh/", 1);
    setenv("USER", "josh", 1);
    setenv("XDG_RUNTIME_DIR", "/run/user/1000", 1);
    setenv("XDG_DATA_HOME", "/home/josh/.local/share", 1);
    setenv("XDG_CONFIG_HOME", "/home/josh/.config", 1);
    setenv("XDG_CACHE_HOME", "/home/josh/.cache", 1);

    printf("%d\n", getuid());
    printf("%s\n", getenv("USER"));

    AudioController audioController(eventProcessor);

    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QQmlApplicationEngine engine;

    qmlRegisterType<OGLTest>(
        "visual_sync_composer",
        1, 0,
        "OGLTest"
    );
    engine.rootContext()->setContextProperty(QStringLiteral("audioController"), &audioController);


    engine.loadFromModule("visual_sync_composer", "Main");
    //engine.load(QUrl("qrc:Main.qml"));
    int exitCode = QGuiApplication::exec();
    return exitCode;
}
