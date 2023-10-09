#include "Window.h"

#include <QMenuBar>
#include <QSettings>

Window::Window(QSettings& options, QWidget* parent)
		: QMainWindow(parent) {
	this->setWindowIcon(QIcon(":/icon.png"));
	this->setMinimumSize(900, 500);

	// File menu
	auto* fileMenu = this->menuBar()->addMenu(tr("&File"));
}
