#include "EntityGraphModel.h"

std::unordered_set<NodeId> EntityGraphModel::allNodeIds() const {
	return this->nodeIds;
}

std::unordered_set<ConnectionId> EntityGraphModel::allConnectionIds(NodeId nodeId) const {
	std::unordered_set<ConnectionId> result;
	std::copy_if(this->connectivity.begin(), this->connectivity.end(), std::inserter(result, std::end(result)), [&nodeId](const ConnectionId& cid) {
		return cid.inNodeId == nodeId || cid.outNodeId == nodeId;
	});
	return result;
}

std::unordered_set<ConnectionId> EntityGraphModel::connections(NodeId nodeId, PortType portType, PortIndex portIndex) const {
	std::unordered_set<ConnectionId> result;
	std::copy_if(this->connectivity.begin(), this->connectivity.end(), std::inserter(result, std::end(result)), [&portType, &portIndex, &nodeId](const ConnectionId& cid) {
		return getNodeId(portType, cid) == nodeId && getPortIndex(portType, cid) == portIndex;
	});
	return result;
}

bool EntityGraphModel::connectionExists(ConnectionId connectionId) const {
	return this->connectivity.find(connectionId) != this->connectivity.end();
}

NodeId EntityGraphModel::addNode(QString nodeType) {
	const auto newId = this->newNodeId();
	this->nodeIds.insert(newId);
	this->nodes[newId] = NodeData{};
	this->nodes[newId].type = nodeType;
	Q_EMIT this->nodeCreated(newId);
	return newId;
}

bool EntityGraphModel::connectionPossible(ConnectionId connectionId) const {
	return this->connectivity.find(connectionId) == this->connectivity.end();
}

void EntityGraphModel::addConnection(ConnectionId connectionId) {
	this->connectivity.insert(connectionId);
	Q_EMIT this->connectionCreated(connectionId);
}

bool EntityGraphModel::nodeExists(NodeId nodeId) const {
	return this->nodeIds.find(nodeId) != this->nodeIds.end();
}

QVariant EntityGraphModel::nodeData(NodeId nodeId, NodeRole role) const {
	switch (role) {
		case NodeRole::Type:
			return this->nodes[nodeId].type;
		case NodeRole::Position:
			return this->nodes[nodeId].position;
		case NodeRole::Size:
			return this->nodes[nodeId].size;
		case NodeRole::CaptionVisible:
			return true;
		case NodeRole::Caption:
			return this->nodes[nodeId].caption;
		case NodeRole::Style:
			return StyleCollection::nodeStyle().toJson().toVariantMap();
		case NodeRole::InternalData:
			return {};
		case NodeRole::InPortCount:
			return static_cast<PortIndex>(this->nodes[nodeId].inputs.size());
		case NodeRole::OutPortCount:
			return static_cast<PortIndex>(this->nodes[nodeId].outputs.size());
		case NodeRole::Widget:
			return {};
	}
}

bool EntityGraphModel::setNodeData(NodeId nodeId, NodeRole role, QVariant value) {
	switch (role) {
		case NodeRole::Type:
			this->nodes[nodeId].type = value.value<QString>();
			return true;
		case NodeRole::Position:
			this->nodes[nodeId].position = value.value<QPointF>();
			Q_EMIT this->nodePositionUpdated(nodeId);
			return true;
		case NodeRole::Size:
			this->nodes[nodeId].size = value.value<QSize>();
			return true;
		case NodeRole::CaptionVisible:
			return false;
		case NodeRole::Caption:
			this->nodes[nodeId].caption = value.value<QString>();
			return true;
		case NodeRole::Style:
			return false;
		case NodeRole::InternalData:
			return false;
		case NodeRole::InPortCount:
			this->nodes[nodeId].inputs.resize(value.value<PortIndex>());
			return true;
		case NodeRole::OutPortCount:
			this->nodes[nodeId].outputs.resize(value.value<PortIndex>());
			return true;
		case NodeRole::Widget:
			return false;
	}
	return false;
}

QVariant EntityGraphModel::portData(NodeId nodeId, PortType portType, PortIndex portIndex, PortRole role) const {
	switch (role) {
		case PortRole::Data:
			return {};
		case PortRole::DataType:
			if (portType == PortType::In) {
				return this->nodes[nodeId].inputs.at(portIndex).type;
			} else if (portType == PortType::Out) {
				return this->nodes[nodeId].outputs.at(portIndex).type;
			}
			return {};
		case PortRole::ConnectionPolicyRole:
			if (portType == PortType::In) {
				return QVariant::fromValue(this->nodes[nodeId].inputs.at(portIndex).allowMultipleConnections ? ConnectionPolicy::Many : ConnectionPolicy::One);
			} else if (portType == PortType::Out) {
				return QVariant::fromValue(this->nodes[nodeId].outputs.at(portIndex).allowMultipleConnections ? ConnectionPolicy::Many : ConnectionPolicy::One);
			}
			return {};
		case PortRole::CaptionVisible:
			return true;
		case PortRole::Caption:
			if (portType == PortType::In) {
				return this->nodes[nodeId].inputs.at(portIndex).caption;
			} else if (portType == PortType::Out) {
				return this->nodes[nodeId].outputs.at(portIndex).caption;
			}
			return {};
	}
	return {};
}

bool EntityGraphModel::setPortData(NodeId nodeId, PortType portType, PortIndex portIndex, const QVariant& value, PortRole role) {
	switch (role) {
		case PortRole::Data:
			return false;
		case PortRole::DataType:
			if (portType == PortType::In) {
				this->nodes[nodeId].inputs[portIndex].type = value.value<QString>();
				return true;
			} else if (portType == PortType::Out) {
				this->nodes[nodeId].outputs[portIndex].type = value.value<QString>();
				return true;
			}
			return false;
		case PortRole::ConnectionPolicyRole: {
			bool allowMultipleConnections = value.value<ConnectionPolicy>() == ConnectionPolicy::Many;
			if (portType == PortType::In) {
				this->nodes[nodeId].inputs[portIndex].allowMultipleConnections = allowMultipleConnections;
				return true;
			} else if (portType == PortType::Out) {
				this->nodes[nodeId].outputs[portIndex].allowMultipleConnections = allowMultipleConnections;
				return true;
			}
			return false;
		}
		case PortRole::CaptionVisible:
			return false;
		case PortRole::Caption:
			if (portType == PortType::In) {
				this->nodes[nodeId].inputs[portIndex].caption = value.value<QString>();
				return true;
			} else if (portType == PortType::Out) {
				this->nodes[nodeId].outputs[portIndex].caption = value.value<QString>();
				return true;
			}
			return false;
	}
	return false;
}

bool EntityGraphModel::deleteConnection(ConnectionId connectionId) {
	bool disconnected = false;
	if (auto it = this->connectivity.find(connectionId); it != this->connectivity.end()) {
		disconnected = true;
		this->connectivity.erase(it);
	}
	if (disconnected) {
		Q_EMIT this->connectionDeleted(connectionId);
	}
	return disconnected;
}

bool EntityGraphModel::deleteNode(NodeId nodeId) {
	// Delete connections to this node first.
	auto connectionIds = this->allConnectionIds(nodeId);
	for (auto &cId : connectionIds) {
		this->deleteConnection(cId);
	}
	this->nodeIds.erase(nodeId);
	this->nodes.erase(nodeId);
	Q_EMIT this->nodeDeleted(nodeId);
	return true;
}

QJsonObject EntityGraphModel::saveNode(NodeId nodeId) const {
	QJsonObject nodeJson;

	nodeJson["id"] = static_cast<qint64>(nodeId);

	{
		const auto pos = this->nodeData(nodeId, NodeRole::Position).value<QPointF>();

		QJsonObject posJson;
		posJson["x"] = pos.x();
		posJson["y"] = pos.y();
		nodeJson["position"] = posJson;
	}

	return nodeJson;
}

void EntityGraphModel::loadNode(const QJsonObject& nodeJson) {
	NodeId restoredNodeId = static_cast<NodeId>(nodeJson["id"].toInt());

	// Next NodeId must be larger that any id existing in the graph
	this->nextNodeId = std::max(this->nextNodeId, restoredNodeId + 1);

	// Create new node.
	this->nodeIds.insert(restoredNodeId);
	Q_EMIT this->nodeCreated(restoredNodeId);

	{
		QJsonObject posJson = nodeJson["position"].toObject();
		const QPointF pos{posJson["x"].toDouble(), posJson["y"].toDouble()};
		this->setNodeData(restoredNodeId, NodeRole::Position, pos);
	}
}

NodeId EntityGraphModel::newNodeId() {
	return this->nextNodeId++;
}
