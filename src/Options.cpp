#include "Options.h"

#include <QApplication>
#include <QFileInfo>
#include <QSettings>
#include <QStyle>

bool isStandalone() {
    QFileInfo nonportable(QApplication::applicationDirPath() + "/.nonportable");
    return !(nonportable.exists() && nonportable.isFile());
}

void setupOptions(QSettings& options) {
    if (!options.contains(OPT_STYLE)) {
        options.setValue(OPT_STYLE, QApplication::style()->name());
    }
    QApplication::setStyle(options.value(OPT_STYLE).toString());
}
