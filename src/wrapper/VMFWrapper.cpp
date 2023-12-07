#include "VMFWrapper.h"

// I made the library I'm allowed to do this
#include <vmfpp/detail/StringUtils.h>
#include <vmfpp/Reader.h>

EntityKVParser::EntityKVParser(const QString& contents) {
	vmfpp::Reader reader;
	this->valid = reader.readData(this->root, contents.toStdString());
}

bool EntityKVParser::isValid() const {
	return this->valid;
}

EntityKVParser::operator bool() const {
	return this->isValid();
}

QList<EntityKV> EntityKVParser::getEntities() const {
	QList<EntityKV> entities;
	if (!this->valid || !this->root.hasSection(vmfpp::DEFAULT_SECTIONS::ENTITY)) {
		return entities;
	}
	const auto& entitySection = this->root.getSection(vmfpp::DEFAULT_SECTIONS::ENTITY);
	for (const auto& entity : entitySection) {
		EntityKV entData;
		entData.id = std::stoi(entity.getValue("id").at(0));
		entData.classname = entity.getValue("classname").at(0).data();
		entData.targetname = entity.hasValue("targetname") ? entity.getValue("targetname").at(0).data() : "";
		if (entity.hasChild("connections")) {
			for (const auto& connection : entity.getChild("connections")) {
				for (const auto& [connectionOutput, connectionInfos] : connection.getValues()) {
					EntityConnectionKV entConnectionData;
					entConnectionData.output = connectionOutput.c_str();
					for (const auto& connectionInfo : connectionInfos) {
						std::vector<std::string> infoParts;
						if (std::count(connectionInfo.begin(), connectionInfo.end(), 0x1b) != 0) {
							infoParts = vmfpp::detail::split(connectionInfo, 0x1b);
						} else {
							infoParts = vmfpp::detail::split(connectionInfo, ',');
						}
						entConnectionData.targetname = infoParts[0].c_str();
						entConnectionData.input = infoParts[1].c_str();
						entConnectionData.parameter = infoParts[2].c_str();
						entConnectionData.delay = infoParts[3].c_str();
						entConnectionData.fireAmount = std::stoi(infoParts[4]);
						entData.connections.push_back(entConnectionData);
					}
				}
			}
		}
		entities.push_back(entData);
	}
	return entities;
}
