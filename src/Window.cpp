#include "Window.h"

#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QFile>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStyle>
#include <QStyleFactory>

#include "cfg/Config.h"
#include "graph/EntityGraph.h"
#include "wrapper/VMFWrapper.h"
#include "Options.h"

constexpr auto VMF_SAVE_FILTER = "Valve Map Format (*.vmf);;All files (*.*)";

Window::Window(QSettings& options, QWidget* parent)
		: QMainWindow(parent)
		, modified(false) {
	this->setWindowIcon(QIcon(":/icon.png"));
	this->setMinimumSize(900, 500);

	// File menu
	auto* fileMenu = this->menuBar()->addMenu(tr("&File"));

	this->openAction = fileMenu->addAction(this->style()->standardIcon(QStyle::SP_DirIcon), tr("&Open..."), Qt::CTRL | Qt::Key_O, [&] {
		this->open();
	});
	this->saveAction = fileMenu->addAction(this->style()->standardIcon(QStyle::SP_DialogSaveButton), tr("&Save"), Qt::CTRL | Qt::Key_S, [&] {
		this->save();
	});
	this->saveAsAction = fileMenu->addAction(this->style()->standardIcon(QStyle::SP_DialogSaveButton), tr("Save &As..."), Qt::CTRL | Qt::SHIFT | Qt::Key_S, [&] {
		this->saveAs();
	});
	this->closeFileAction = fileMenu->addAction(this->style()->standardIcon(QStyle::SP_BrowserReload), tr("&Close"), Qt::CTRL | Qt::Key_X, [&] {
		this->closeFile();
	});

	fileMenu->addSeparator();

	fileMenu->addAction(this->style()->standardIcon(QStyle::SP_DialogCancelButton), tr("&Exit"), Qt::ALT | Qt::Key_F4, [&] {
		this->close();
	});

	// Options menu
	auto* optionsMenu = this->menuBar()->addMenu(tr("&Options"));

	auto* themeMenu = optionsMenu->addMenu(this->style()->standardIcon(QStyle::SP_DesktopIcon), tr("&Theme..."));
	auto* themeMenuGroup = new QActionGroup(this);
	themeMenuGroup->setExclusive(true);
	for (const auto& themeName : QStyleFactory::keys()) {
		auto* action = themeMenu->addAction(themeName, [=, &options] {
			QApplication::setStyle(themeName);
			options.setValue(OPT_STYLE, themeName);
		});
		action->setCheckable(true);
		if (themeName == options.value(OPT_STYLE).toString()) {
			action->setChecked(true);
		}
		themeMenuGroup->addAction(action);
	}

	// Help menu
	auto* helpMenu = this->menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(this->style()->standardIcon(QStyle::SP_DialogHelpButton), tr("&About"), Qt::Key_F1, [&] {
		this->about();
	});
	helpMenu->addAction(this->style()->standardIcon(QStyle::SP_DialogHelpButton), "About &Qt", [&] {
		this->aboutQt();
	});

	this->graph = new EntityGraph(this);
	this->setCentralWidget(this->graph);

	// Finalize window
	this->clearContents();
}

void Window::open(const QString& startPath) {
	auto path = QFileDialog::getOpenFileName(this, tr("Open VMF"), startPath, VMF_SAVE_FILTER);
	if (path.isEmpty()) {
		return;
	}
	if (!this->load(path)) {
		this->clearContents();
	}
	this->graph->setDisabled(false);
}

void Window::save() {
	// todo: save
	this->markModified(false);
}

void Window::saveAs() {
	// todo: save as
	this->markModified(false);
}

void Window::closeFile() {
	this->clearContents();
}

void Window::about() {
	QString creditsText = "# " ENTGRAPH_PROJECT_NAME_PRETTY " v" ENTGRAPH_PROJECT_VERSION "\n\n<br/>\n\n";
	QFile creditsFile(QCoreApplication::applicationDirPath() + "/CREDITS.md");
	if (creditsFile.open(QIODevice::ReadOnly)) {
		QTextStream in(&creditsFile);
		while(!in.atEnd()) {
			creditsText += in.readLine() + '\n';
		}
		creditsFile.close();
	}

	QMessageBox about(this);
	about.setWindowTitle(tr("About"));
	about.setIconPixmap(QIcon(":/icon.png").pixmap(64, 64));
	about.setTextFormat(Qt::TextFormat::MarkdownText);
	about.setText(creditsText);
	about.exec();
}

void Window::aboutQt() {
	QMessageBox::aboutQt(this);
}

void Window::markModified(bool modified_) {
	this->modified = modified_;

	if (this->modified) {
		this->setWindowTitle(ENTGRAPH_PROJECT_NAME_PRETTY " v" ENTGRAPH_PROJECT_VERSION " (*)");
	} else {
		this->setWindowTitle(ENTGRAPH_PROJECT_NAME_PRETTY " v" ENTGRAPH_PROJECT_VERSION);
	}

	this->saveAction->setDisabled(!this->modified);
}

void Window::clearContents() {
	if (this->modified && this->promptUserToKeepModifications()) {
		return;
	}

	this->graph->clear();
	this->graph->setDisabled(true);

	this->markModified(false);
	this->freezeActions(true, false); // Leave creation actions unfrozen
}

void Window::closeEvent(QCloseEvent* event) {
	if (this->modified && this->promptUserToKeepModifications()) {
		event->ignore();
		return;
	}
	event->accept();
}

bool Window::load(const QString& path) {
	this->clearContents();
	this->freezeActions(true);

	// todo: load map
#if 0
	VMFWrapper vmfWrapper{path};
	if (!vmfWrapper) {
		return false;
	}
	auto entities = vmfWrapper.getEntities();
	auto& model = this->graph->model();
	QMap<QString, NodeId> namedEntityIds;

	for (auto& entity : entities) {
		NodeId id = model.addNode(entity.classname, entity.id);
		if (!entity.targetname.isEmpty()) {
			namedEntityIds[entity.targetname] = id;
		}
		model.setNodeData(id, NodeRole::Caption, entity.targetname.isEmpty() ? entity.classname : entity.targetname + "(" + entity.classname + ")");
		model.setNodeData(id, NodeRole::Position, QPointF(0, 0));
		model.setNodeData(id, NodeRole::InPortCount, 1);
		model.setPortData(id, PortType::In, 0, QVariant::fromValue(ConnectionPolicy::Many), PortRole::ConnectionPolicyRole);
		model.setNodeData(id, NodeRole::OutPortCount, entity.connections.length());
	}
	for (auto& entity : entities) {
		if (entity.connections.isEmpty()) {
			continue;
		}
		for (auto& connection : entity.connections) {
			if (namedEntityIds.contains(connection.targetname)) {
				model.addConnection({static_cast<NodeId>(entity.id), 0, namedEntityIds[connection.targetname], 0});
			}
		}
	}
#endif

	this->freezeActions(false);
	return true;
}

bool Window::promptUserToKeepModifications() {
	auto response = QMessageBox::warning(this, tr("Save changes?"), tr("Hold up! Would you like to save your changes first?"), QMessageBox::Ok | QMessageBox::Discard | QMessageBox::Cancel);
	if (response == QMessageBox::Cancel) {
		return true;
	}
	if (response == QMessageBox::Discard) {
		return false;
	}
	if (response == QMessageBox::Ok) {
		this->save();
		return false;
	}
	return true;
}

void Window::freezeActions(bool freeze, bool freezeCreationActions) {
	this->openAction->setDisabled(freeze && freezeCreationActions);
	this->saveAction->setDisabled(freeze || !this->modified);
	this->saveAsAction->setDisabled(freeze);
	this->closeFileAction->setDisabled(freeze);
}
