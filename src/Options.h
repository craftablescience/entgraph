#pragma once

class QSettings;

constexpr auto OPT_STYLE = "style";

bool isStandalone();

void setupOptions(QSettings& options);
