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

#include "config/Config.h"
#include "config/Options.h"
#include "graph/EntityGraph.h"
#include "wrapper/VMFWrapper.h"

constexpr auto VMF_SAVE_FILTER = "Valve Map Format (*.vmf);;All files (*.*)";

Window::Window(QWidget* parent)
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
		auto* action = themeMenu->addAction(themeName, [=] {
			QApplication::setStyle(themeName);
			Options::set(OPT_STYLE, themeName);
		});
		action->setCheckable(true);
		if (themeName == Options::get<QString>(OPT_STYLE)) {
			action->setChecked(true);
		}
		themeMenuGroup->addAction(action);
	}

	// Help menu
	auto* helpMenu = this->menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(this->style()->standardIcon(QStyle::SP_DialogHelpButton), tr("&About"), Qt::Key_F1, [&] {
		this->about();
	});
	helpMenu->addAction(this->style()->standardIcon(QStyle::SP_DialogHelpButton), "About &Qt", Qt::ALT | Qt::Key_F1, [&] {
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

    // todo: fgd parser
    QMap<QString, QMap<QString, PortIndex>> entityInputs{
            {"logic_relay", {{"Enable", 0}, {"Disable", 1}}},
    };

	for (auto& entity : entities) {
		NodeId id = model.addNode(entity.classname, entity.id);
		if (!entity.targetname.isEmpty()) {
			namedEntityIds[entity.targetname] = id;
		}
		model.setNodeData(id, NodeRole::Caption, entity.targetname.isEmpty() ? entity.classname : entity.targetname + " (" + entity.classname + ")");
		model.setNodeData(id, NodeRole::Position, QPointF(0, 0));
	}
	for (auto& entity : entities) {
        NodeId id = entity.id;
        model.setNodeData(id, NodeRole::InPortCount, entityInputs[entity.classname].size());
        for (PortIndex i = 0; i < entityInputs[entity.classname].size(); i++) {
            model.setPortData(id, PortType::In, i, QVariant::fromValue(ConnectionPolicy::Many), PortRole::ConnectionPolicyRole);
            model.setPortData(id, PortType::In, i, entityInputs[entity.classname].key(i), PortRole::Caption);
        }
        model.setNodeData(id, NodeRole::OutPortCount, entity.connections.length());
        for (PortIndex i = 0; i < entity.connections.length(); i++) {
            model.setPortData(id, PortType::Out, i, QVariant::fromValue(ConnectionPolicy::Many), PortRole::ConnectionPolicyRole);
            model.setPortData(id, PortType::Out, i, entity.connections[i].output, PortRole::Caption);
            if (namedEntityIds.contains(entity.connections[i].targetname)) {
				model.addConnection({static_cast<NodeId>(entity.id), i, namedEntityIds[entity.connections[i].targetname], entityInputs[entity.targetname][entity.connections[i].output]});
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
