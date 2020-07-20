#ifndef FIELDQTTYPELOOKUP_H
#define FIELDQTTYPELOOKUP_H

#include <QString>
#include <QObject>

class FieldQtTypeLookup
{
public:
    enum DbType {
        Sqlite,
        MySQL,
        PgSQL,
        MsSQL
    };

    static QString getQtType(const QString &fieldType, const DbType dbType = MySQL);
    static QString getInitializationValue(const QString &fieldType);
private:
    static QString getMySQLFieldQtType(const QString &databaseType);
};

#endif // FIELDQTTYPELOOKUP_H
