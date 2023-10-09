#pragma once

#include <QMainWindow>

class QSettings;

class Window : public QMainWindow {
	Q_OBJECT;

public:
	explicit Window(QSettings& options, QWidget* parent = nullptr);
};
