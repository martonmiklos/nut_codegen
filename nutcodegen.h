#ifndef NUTCODEGEN_H
#define NUTCODEGEN_H

#include <QDir>
#include <QString>
#include <QSqlDatabase>

#include "fieldqttypelookup.h"
#include "libkode/class.h"
#include "libkode/printer.h"
#include "table.h"

class NutCodeGen
{
public:
    NutCodeGen(const QString &database,
               const QString &host,
               const QString &username,
               const QString &password,
               const QString &workingDirectory, FieldQtTypeLookup::DbType databaseType);
    ~NutCodeGen();

    bool readTables();
    bool readTableFields();
    bool readRelations();

    QString errorString() const;

    bool generateFiles();

    void setGenerateCloneMethods(bool generateCopyConstructor);
    void setGenerateEqualsOperator(bool generateEqualsOperator);

private:
    bool readTablesMySql();
    bool readTableFieldsMySql();
    bool readRelationsMySql();

    bool readTablesSqlite();
    bool readTableFieldsSqlite();
    bool readRelationsSqlite();

    QSqlDatabase m_db;
    QString m_database;
    QString m_host = QStringLiteral("localhost");
    QString m_username;
    QString m_password;
    QString m_workingDir;
    bool m_generateCloneMethod = false;
    bool m_generateEqualsOperator = false;
    QString m_errorString;

    QList <Table*> m_tables;

    bool generateDatabaseClass();
    bool generateTableClasses();
    bool generatePriFile();

    KODE::Printer m_printer;

    void addCloneMethod(const Table *table, KODE::Class *class_);
    void addEqualsOperator(const Table *table, KODE::Class *class_);

    FieldQtTypeLookup::DbType m_databaseType;
    QString m_fnBase;
};

#endif // NUTCODEGEN_H
