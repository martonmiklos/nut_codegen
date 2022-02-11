#include <QCoreApplication>

#include <QCommandLineParser>
#include <QDir>
#include <QDebug>

#include "nutcodegen.h"

int main(int argc, char *argv[])
{
    QCoreApplication app( argc, argv );
    QCoreApplication::setApplicationName("nut_codegen");
    QCoreApplication::setOrganizationName("MM");

    QCommandLineParser cmdLine;
    cmdLine.setApplicationDescription("NUT ORM code generator");
    cmdLine.addHelpOption();
    cmdLine.addVersionOption();

    QCommandLineOption dirOption(
                QStringList() << "d" << "directory",
                QCoreApplication::translate("main", "Directory to generate files in"),
                "directory",
                ".");
    cmdLine.addOption(dirOption);

    QCommandLineOption databaseOption(
                QStringList() << "db" << "database",
                QCoreApplication::translate("main", "The name of the database"),
                "database");
    cmdLine.addOption(databaseOption);

    QCommandLineOption hostOption(
                QStringList() << "host",
                QCoreApplication::translate("main", "Server to connect (by default it is localhost)"),
                "host");
    cmdLine.addOption(hostOption);

    QCommandLineOption passwordOption(
                QStringList() << "password",
                QCoreApplication::translate("main", "Password to use when connecting to server"),
                "password");
    cmdLine.addOption(passwordOption);

    QCommandLineOption userOption(
                QStringList() << "user",
                QCoreApplication::translate("main", "MySQL user name to use when connecting to server"),
                "user");
    cmdLine.addOption(userOption);

    QCommandLineOption cloneMethodOption(
                QStringList() << "add-clone-methods",
                QCoreApplication::translate("main", "Generate copy methods for table classes"));
    cmdLine.addOption(cloneMethodOption);

    QCommandLineOption equalsOpOption(
                QStringList() << "add-equals-operator",
                QCoreApplication::translate("main", "Generate copy methods for table classes"));
    cmdLine.addOption(equalsOpOption);

    cmdLine.process(app);
    if (!cmdLine.parse(QCoreApplication::arguments())) {
        qDebug() << cmdLine.errorText();
        return -1;
    }

    NutCodeGen cg(cmdLine.value(databaseOption),
                  cmdLine.value(hostOption),
                  cmdLine.value(userOption),
                  cmdLine.value(passwordOption),
                  cmdLine.value(dirOption),
                  !QFileInfo::exists(cmdLine.value(databaseOption))
                  ? FieldQtTypeLookup::DbType::MySQL : FieldQtTypeLookup::Sqlite);
    if (cmdLine.isSet(cloneMethodOption))
        cg.setGenerateCloneMethods(true);

    if (cmdLine.isSet(equalsOpOption))
        cg.setGenerateEqualsOperator(true);

    if (!cg.readTables()) {
        qDebug() << cg.errorString();
        return -1;
    }

    if (!cg.readTableFields()) {
        qDebug() << cg.errorString();
        return -1;
    }

    if (!cg.readRelations()) {
        qDebug() << cg.errorString();
        return -1;
    }

    cg.generateFiles();
    return 0;
}
