#pragma once

#include <QList>
#include <QString>

#include <vmfpp/VMF.h>

struct EntityConnectionKV {
	QString output;
	QString targetname;
	QString input;
	QString parameter;
	QString delay;
	int fireAmount;
};

struct EntityKV {
	int id;
	QString classname;
	QString targetname;
	QList<EntityConnectionKV> connections;
};

class EntityKVParser {
public:
	explicit EntityKVParser(const QString& contents);

	[[nodiscard]] bool isValid() const;

	[[nodiscard]] explicit operator bool() const;

	[[nodiscard]] QList<EntityKV> getEntities() const;

private:
	vmfpp::Root root;
	bool valid;
};
