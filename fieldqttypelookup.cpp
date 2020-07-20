#include "fieldqttypelookup.h"

#include <QRegularExpression>

QString FieldQtTypeLookup::getQtType(const QString &fieldType, const FieldQtTypeLookup::DbType dbType)
{
    switch (dbType) {
    case FieldQtTypeLookup::Sqlite:
        break;
    case FieldQtTypeLookup::MySQL:
        return getMySQLFieldQtType(fieldType);
    case FieldQtTypeLookup::PgSQL:
        break;
    case FieldQtTypeLookup::MsSQL:
        break;
    }
    return QString();
}

QString FieldQtTypeLookup::getInitializationValue(const QString &fieldType)
{
    if (fieldType == "bool")
        return "false";
    else if (fieldType.startsWith("Q"))
        return fieldType + "()";
    else if (fieldType == "qreal"
             || fieldType == "double")
        return "0.0";
    else if (fieldType == "float")
        return "0.0f";
    else if (fieldType == "qint8"
             || fieldType == "short"
             || fieldType == "ushort"
             || fieldType == "int"
             ||fieldType == "long"
             || fieldType == "qlonglong")
        return "0";
    Q_UNREACHABLE();
    return QString();
}

QString FieldQtTypeLookup::getMySQLFieldQtType(const QString &databaseType)
{
    QString baseType = databaseType.toUpper();
    if (baseType == "CHAR(1)")
        return "QChar";
    if (baseType == "BIT(1)" || baseType == "TINYINT(1)")
        return "bool";

    QRegularExpression re("([a-zA-Z]*)\\([0-9]*.*");
    QRegularExpressionMatch match = re.match(databaseType);
    if (match.hasMatch()) {
        baseType = match.captured(1).toUpper();
    }

    if (baseType == "ENUM") return "QString";
    if (baseType == "BOOLEAN") return "bool";
    if (baseType == "VARBINARY") return "QBitArray";
    if (baseType == "BLOB") return "QByteArray";
    if (baseType == "DATE") return "QDate";
    if (baseType == "DATETIME") return "QDateTime";
    if (baseType == "TIME") return "QTime";
    if (baseType == "REAL") return "double";
    if (baseType == "FLOAT") return "float";
    if (baseType == "TINYINT") return "qint8";
    if (baseType == "SMALLINT") return "short";
    if (baseType == "SMALLINT") return "ushort";
    if (baseType == "INT") return "int";
    if (baseType == "BIGINT") return "long";
    if (baseType == "BIGINT") return "qlonglong";
    if (baseType == "TEXT") return "QString";
    if (baseType == "VARCHAR") return "QString";
    if (baseType == "DECIMAL") return "qreal";
    return databaseType;
}
