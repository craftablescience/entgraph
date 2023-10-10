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

	// todo: remove sample code
	{
		NodeId id1 = this->graphModel.addNode("logic_auto");
		this->graphModel.setNodeData(id1, NodeRole::Caption, "logic_auto");
		this->graphModel.setNodeData(id1, NodeRole::Position, QPointF(0, 0));
		this->graphModel.setNodeData(id1, NodeRole::InPortCount, 3);
		this->graphModel.setNodeData(id1, NodeRole::OutPortCount, 5);

		NodeId id2 = this->graphModel.addNode("logic_relay");
		this->graphModel.setNodeData(id2, NodeRole::Caption, "logic_relay");
		this->graphModel.setNodeData(id2, NodeRole::Position, QPointF(300, 300));
		this->graphModel.setNodeData(id2, NodeRole::InPortCount, 3);
		this->graphModel.setNodeData(id2, NodeRole::OutPortCount, 5);

		this->graphModel.addConnection({id1, 2, id2, 1});
	}

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
