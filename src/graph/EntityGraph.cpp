#include "EntityGraph.h"

#include <QAction>
#include <QHBoxLayout>

#include <QtNodes/BasicGraphicsScene>
#include <QtNodes/ConnectionStyle>

EntityGraph::EntityGraph(QWidget* parent)
		: QWidget(parent) {
	// Set up some style stuff
	this->graphView.setStyleSheet(R"(QFrame { border: none; })");
	QtNodes::ConnectionStyle::setConnectionStyle(R"({ "ConnectionStyle": { "UseDataDefinedColors": true } })");

	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	this->graphScene = new QtNodes::BasicGraphicsScene(graphModel);
	this->graphScene->setOrientation(Qt::Horizontal);

	this->graphView.setScene(this->graphScene);
	layout->addWidget(&this->graphView);

	// Setup context menu for creating new nodes
	this->graphView.setContextMenuPolicy(Qt::ActionsContextMenu);
	this->addEntityAction = new QAction(tr("Add Entity..."), &this->graphView);
	this->addEntityAction->setShortcut(Qt::CTRL | Qt::Key_A);
	QObject::connect(this->addEntityAction, &QAction::triggered, [&] {
		// Mouse position in scene coordinates
		QPointF posView = this->graphView.mapToScene(this->graphView.mapFromGlobal(QCursor::pos()));
		const NodeId newId = this->graphModel.addNode(QString());
		this->graphModel.setNodeData(newId, NodeRole::Position, posView);
	});
	this->graphView.insertAction(this->graphView.actions().front(), this->addEntityAction);
}

void EntityGraph::clear() {
	this->graphModel.clear();
}
