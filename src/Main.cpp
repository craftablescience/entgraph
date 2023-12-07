#include <memory>

#include <QApplication>
#include <QSettings>
#include <QStyle>

#include "config/Config.h"
#include "config/Options.h"
#include "Window.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(ENTGRAPH_PROJECT_ORGANIZATION_NAME);
    QCoreApplication::setApplicationName(ENTGRAPH_PROJECT_NAME);
    QCoreApplication::setApplicationVersion(ENTGRAPH_PROJECT_VERSION);

#if !defined(__APPLE__) && !defined(_WIN32)
    QGuiApplication::setDesktopFileName(ENTGRAPH_PROJECT_NAME);
#endif

    std::unique_ptr<QSettings> options;
    if (Options::isStandalone()) {
        auto configPath = QApplication::applicationDirPath() + "/config.ini";
        options = std::make_unique<QSettings>(configPath, QSettings::Format::IniFormat);
    } else {
        options = std::make_unique<QSettings>();
    }
    Options::setupOptions(*options);

    auto* window = new Window();
    window->show();

    return QApplication::exec();
}
