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

	[[nodiscard]] const EntityGraphModel& model() const {
		return this->graphModel;
	}

	[[nodiscard]] EntityGraphModel& model() {
		return this->graphModel;
	}

	void clear();

private:
	EntityGraphModel graphModel;

	QtNodes::BasicGraphicsScene* graphScene;
	QtNodes::GraphicsView graphView;

	QAction* addEntityAction;
};
