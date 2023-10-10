#include <memory>

#include <QApplication>
#include <QSettings>
#include <QStyle>

#include "cfg/Config.h"
#include "Options.h"
#include "Window.h"
#include "FGDParser.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    FGD::FGDTokenizer tokenizer("/media/trico/Storage Drive X2/SteamLibrary/steamapps/common/Portal 2 Community Edition/p2ce/p2ce.fgd");



    QCoreApplication::setOrganizationName(ENTGRAPH_ORGANIZATION_NAME);
    QCoreApplication::setApplicationName(ENTGRAPH_PROJECT_NAME);
    QCoreApplication::setApplicationVersion(ENTGRAPH_PROJECT_VERSION);

#if !defined(__APPLE__) && !defined(_WIN32)
    QGuiApplication::setDesktopFileName(ENTGRAPH_PROJECT_NAME);
#endif

    std::unique_ptr<QSettings> options;
    if (isStandalone()) {
        auto configPath = QApplication::applicationDirPath() + "/config.ini";
        options = std::make_unique<QSettings>(configPath, QSettings::Format::IniFormat);
    } else {
        options = std::make_unique<QSettings>();
    }
    setupOptions(*options);

    auto* window = new Window(*options);
    window->show();

    return QApplication::exec();
}
