#include "EntityGraphModel.h"

std::unordered_set<NodeId> EntityGraphModel::allNodeIds() const {
	return this->nodeIds;
}

std::unordered_set<ConnectionId> EntityGraphModel::allConnectionIds(const NodeId nodeId) const {
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

bool EntityGraphModel::connectionExists(const ConnectionId connectionId) const {
	return this->connectivity.find(connectionId) != this->connectivity.end();
}

NodeId EntityGraphModel::addNode(const QString nodeType) {
	const auto newId = this->newNodeId();
	this->nodeIds.insert(newId);
	Q_EMIT this->nodeCreated(newId);
	return newId;
}

bool EntityGraphModel::connectionPossible(const ConnectionId connectionId) const {
	return this->connectivity.find(connectionId) == this->connectivity.end();
}

void EntityGraphModel::addConnection(const ConnectionId connectionId) {
	this->connectivity.insert(connectionId);
	Q_EMIT this->connectionCreated(connectionId);
}

bool EntityGraphModel::nodeExists(const NodeId nodeId) const {
	return this->nodeIds.find(nodeId) != this->nodeIds.end();
}

QVariant EntityGraphModel::nodeData(NodeId nodeId, NodeRole role) const {
	QVariant result;
	switch (role) {
		case NodeRole::Type:
			result = QString("Default Node Type");
			break;
		case NodeRole::Position:
			result = this->nodeGeometryData[nodeId].pos;
			break;
		case NodeRole::Size:
			result = this->nodeGeometryData[nodeId].size;
			break;
		case NodeRole::CaptionVisible:
			result = true;
			break;
		case NodeRole::Caption:
			result = QString("Node");
			break;
		case NodeRole::Style: {
			auto style = StyleCollection::nodeStyle();
			result = style.toJson().toVariantMap();
			break;
		}
		case NodeRole::InternalData:
			break;
		case NodeRole::InPortCount:
			result = 1u;
			break;
		case NodeRole::OutPortCount:
			result = 1u;
			break;
		case NodeRole::Widget:
			result = QVariant();
			break;
	}
	return result;
}

bool EntityGraphModel::setNodeData(NodeId nodeId, NodeRole role, QVariant value) {
	bool result = false;
	switch (role) {
		case NodeRole::Type:
			break;
		case NodeRole::Position: {
			this->nodeGeometryData[nodeId].pos = value.value<QPointF>();
			Q_EMIT this->nodePositionUpdated(nodeId);
			result = true;
			break;
		}
		case NodeRole::Size: {
			this->nodeGeometryData[nodeId].size = value.value<QSize>();
			result = true;
			break;
		}
		case NodeRole::CaptionVisible:
			break;
		case NodeRole::Caption:
			break;
		case NodeRole::Style:
			break;
		case NodeRole::InternalData:
			break;
		case NodeRole::InPortCount:
			break;
		case NodeRole::OutPortCount:
			break;
		case NodeRole::Widget:
			break;
	}
	return result;
}

QVariant EntityGraphModel::portData(NodeId nodeId, PortType portType, PortIndex portIndex, PortRole role) const {
	switch (role) {
		case PortRole::Data:
		case PortRole::DataType:
			return {};
		case PortRole::ConnectionPolicyRole:
			return QVariant::fromValue(ConnectionPolicy::One);
		case PortRole::CaptionVisible:
			return true;
		case PortRole::Caption:
			if (portType == PortType::In)
				return QString::fromUtf8("Port In");
			else
				return QString::fromUtf8("Port Out");
	}
	return {};
}

bool EntityGraphModel::setPortData(NodeId nodeId, PortType portType, PortIndex portIndex, const QVariant& value, PortRole role) {
	// todo: what does this do
	return false;
}

bool EntityGraphModel::deleteConnection(const ConnectionId connectionId) {
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

bool EntityGraphModel::deleteNode(const NodeId nodeId) {
	// Delete connections to this node first.
	auto connectionIds = this->allConnectionIds(nodeId);
	for (auto &cId : connectionIds) {
		this->deleteConnection(cId);
	}
	this->nodeIds.erase(nodeId);
	this->nodeGeometryData.erase(nodeId);
	Q_EMIT this->nodeDeleted(nodeId);
	return true;
}

QJsonObject EntityGraphModel::saveNode(const NodeId nodeId) const {
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
