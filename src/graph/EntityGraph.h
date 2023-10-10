#pragma once

#include <QWidget>

#include <QtNodes/GraphicsView>

#include "EntityGraphModel.h"

class QAction;

namespace QtNodes {

class BasicGraphicsScene;

} // namespace QtNodes

class EntityGraph : public QWidget {
	Q_OBJECT;

public:
	explicit EntityGraph(QWidget* parent = nullptr);

private:
	EntityGraphModel graphModel;

	QtNodes::BasicGraphicsScene* graphScene;
	QtNodes::GraphicsView graphView;

	QAction* addEntityAction;
};
