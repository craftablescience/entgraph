#pragma once

#include <QMainWindow>

class QAction;
class QCloseEvent;
class QSettings;

class EntityGraph;

class Window : public QMainWindow {
	Q_OBJECT;

public:
	explicit Window(QWidget* parent = nullptr);

	void open(const QString& startPath = QString());

	void save();

	void saveAs();

	void closeFile();

	void about();

	void aboutQt();

	void markModified(bool modified);

	void clearContents();

protected:
	void closeEvent(QCloseEvent* event) override;

private:
	EntityGraph* graph;

	QAction* openAction;
	QAction* saveAction;
	QAction* saveAsAction;
	QAction* closeFileAction;

	bool modified;

	bool load(const QString& path);

	[[nodiscard]] bool promptUserToKeepModifications();

	void freezeActions(bool freeze, bool freezeCreationActions = true);
};
