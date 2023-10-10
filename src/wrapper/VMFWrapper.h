#pragma once

#include <QList>
#include <QString>

#include <vmfpp/VMF.h>

struct VMFEntityConnection {
	QString output;
	QString targetname;
	QString input;
	QString parameter;
	QString delay;
	int fireAmount;
};

struct VMFEntity {
	int id;
	QString classname;
	QString targetname;
	QList<VMFEntityConnection> connections;
};

class VMFWrapper {
public:
	explicit VMFWrapper(const QString& filePath_);

	[[nodiscard]] bool isOpen() const;

	[[nodiscard]] explicit operator bool() const;

	[[nodiscard]] QList<VMFEntity> getEntities() const;

private:
	QString filePath;
	vmfpp::Root root;
	bool valid;
};
