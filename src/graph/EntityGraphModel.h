#pragma once

#include <QJsonObject>
#include <QPointF>
#include <QSize>

#include <QtNodes/AbstractGraphModel>
#include <QtNodes/ConnectionIdUtils>
#include <QtNodes/StyleCollection>

using ConnectionId = QtNodes::ConnectionId;
using ConnectionPolicy = QtNodes::ConnectionPolicy;
using NodeFlag = QtNodes::NodeFlag;
using NodeId = QtNodes::NodeId;
using NodeRole = QtNodes::NodeRole;
using PortIndex = QtNodes::PortIndex;
using PortRole = QtNodes::PortRole;
using PortType = QtNodes::PortType;
using StyleCollection = QtNodes::StyleCollection;

class EntityGraphModel : public QtNodes::AbstractGraphModel {
	Q_OBJECT;

public:
	struct NodePortInput {
		QString type;
		QString caption;
		bool allowMultipleConnections;
	};

	struct NodePortOutput : public NodePortInput {
		QString parameter;
		int delay;
	};

	struct NodeData {
		QSize size;
		QPointF position;

		QString type;
		QString caption;

		QList<NodePortInput> inputs;
		QList<NodePortOutput> outputs;
	};

public:
	EntityGraphModel() = default;

	std::unordered_set<NodeId> allNodeIds() const override;

	std::unordered_set<ConnectionId> allConnectionIds(NodeId nodeId) const override;

	std::unordered_set<ConnectionId> connections(NodeId nodeId, PortType portType, PortIndex portIndex) const override;

	bool connectionExists(ConnectionId connectionId) const override;

	NodeId addNode(QString nodeType) override;

	NodeId addNode(QString nodeType, NodeId nodeId);

	/**
	 * Connection is possible when graph contains no connectivity data
	 * in both directions `Out -> In` and `In -> Out`. We're going to
	 * ignore when the input and output have different types, because it
	 * all gets converted to strings anyway, and we shouldn't restrict what
	 * people can do
	 */
	bool connectionPossible(ConnectionId connectionId) const override;

	void addConnection(ConnectionId connectionId) override;

	bool nodeExists(NodeId nodeId) const override;

	QVariant nodeData(NodeId nodeId, NodeRole role) const override;

	bool setNodeData(NodeId nodeId, NodeRole role, QVariant value) override;

	QVariant portData(NodeId nodeId, PortType portType, PortIndex portIndex, PortRole role) const override;

	bool setPortData(NodeId nodeId, PortType portType, PortIndex portIndex, const QVariant& value, PortRole role) override;

	bool deleteConnection(ConnectionId connectionId) override;

	bool deleteNode(NodeId nodeId) override;

	QJsonObject saveNode(NodeId nodeId) const override;

	void loadNode(const QJsonObject& nodeJson) override;

	NodeId newNodeId() override;

	void clear();

private:
	std::unordered_set<NodeId> nodeIds;
	NodeId nextNodeId = 0;

	/// Contains the graph connectivity information in both directions, i.e. from Node1 to Node2 and from Node2 to Node1
	/// This one is user-defined and can be changed
	std::unordered_set<ConnectionId> connectivity;

	/// Node data
	mutable std::unordered_map<NodeId, NodeData> nodes;
};
