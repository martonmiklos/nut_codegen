#include "nutcodegen.h"

#include <QDebug>
#include <QFile>
#include <QObject>
#include <QSqlError>
#include <QSqlQuery>
#include <QRegularExpression>

#include "inflector.h"
#include "fieldqttypelookup.h"
#include "libkode/class.h"
#include "libkode/code.h"
#include "libkode/namer.h"

using namespace KODE;

NutCodeGen::NutCodeGen(const QString &database,
                       const QString &host,
                       const QString &username,
                       const QString &password,
                       const QString &wd) :
    m_database(database),
    m_username(username),
    m_password(password)
{
    if (!host.isEmpty())
        m_host = host;

    if (wd.isEmpty())
        m_workingDir = QDir::currentPath();
    else
        m_workingDir = wd;
    m_printer.setOutputDirectory(m_workingDir);
}

NutCodeGen::~NutCodeGen()
{
    qDeleteAll(m_tables);
}

bool NutCodeGen::readTables()
{
    m_db = QSqlDatabase::addDatabase("QMYSQL");
    m_db.setHostName(m_host);
    m_db.setDatabaseName(m_database);
    m_db.setUserName(m_username);
    m_db.setPassword(m_password);
    if (!m_db.open()) {
        m_errorString = QObject::tr("Unable to open the %1 database on host %2\n"
                                    "Error details:\n"
                                    "%3")
                .arg(m_database)
                .arg(m_host)
                .arg(m_db.lastError().text());
        return false;
    }

    QSqlQuery showTablesQuery;
    showTablesQuery.prepare("SHOW TABLES;");

    if (!showTablesQuery.exec()) {
        m_errorString = QObject::tr("Unable to execute the %1 query\n"
                                    "Error details:\n"
                                    "%2")
                .arg(showTablesQuery.lastQuery())
                .arg(showTablesQuery.lastError().text());
        return false;
    }

    while (showTablesQuery.next()) {
        if (showTablesQuery.value(0).toString() != ""
                && showTablesQuery.value(0).toString().toLower() != "__change_logs")
            m_tables << new Table(showTablesQuery.value(0).toString());
    }

    return true;
}

bool NutCodeGen::readTableFields()
{
    QSqlQuery fieldsQuery;
    for (auto table : m_tables) {
        if (!fieldsQuery.exec(QString("SHOW COLUMNS FROM %1").arg(table->m_name))) {
            m_errorString = QObject::tr("Unable to execute the %1 query\n"
                                        "Error details:\n"
                                        "%2")
                    .arg(fieldsQuery.lastQuery())
                    .arg(fieldsQuery.lastError().text());
            return false;
        }

        while (fieldsQuery.next()) {
            TableField field(fieldsQuery.value("Field").toString(),
                             fieldsQuery.value("Type").toString(),
                             fieldsQuery.value("Key") == QStringLiteral("PRI"));
            field.setAutoIncrement(fieldsQuery.value("Extra") == "auto_increment");
            field.m_isNull = (fieldsQuery.value("Null").toString() != "NO");
            table->m_fields.append(field);
        }
    }
    return true;
}

bool NutCodeGen::readRelations()
{
    for (auto table : m_tables) {
        for (auto field : table->m_fields) {
            QSqlQuery foreignKeyQuery;
            foreignKeyQuery.prepare("SELECT "
                                    "COLUMN_NAME, "
                                    "REFERENCED_TABLE_NAME, "
                                    "REFERENCED_COLUMN_NAME "
                                    "FROM INFORMATION_SCHEMA.KEY_COLUMN_USAGE "
                                    "WHERE "
                                    "REFERENCED_TABLE_SCHEMA = :database AND "
                                    "TABLE_NAME = :table AND "
                                    "COLUMN_NAME = :field;");
            foreignKeyQuery.bindValue(":database", m_database);
            foreignKeyQuery.bindValue(":table", table->m_name);
            foreignKeyQuery.bindValue(":field", field.m_name);
            if (foreignKeyQuery.exec()) {
                while (foreignKeyQuery.next()) {
                    for(auto t2 : m_tables) {
                        if (t2->m_name == foreignKeyQuery.value("REFERENCED_TABLE_NAME").toString()) {
                            qWarning() << m_database << table->m_name << field.m_name << t2->m_name;
                            TableRelation rel;
                            rel.m_type = TableRelation::BelongsTo;
                            rel.destinationTable = t2;
                            rel.fieldName = field.m_name;
                            rel.referencedFieldName = foreignKeyQuery.value("REFERENCED_COLUMN_NAME").toString();
                            rel.optional = field.m_isNull;
                            table->m_relations.append(rel);

                            TableRelation backRel;
                            backRel.m_type = TableRelation::HasMany;
                            backRel.destinationTable = table;
                            t2->m_relations.append(backRel);
                            break;
                        }
                    }
                }
            }
        }
    }
    return true;
}

QString NutCodeGen::errorString() const
{
    return m_errorString;
}

bool NutCodeGen::generateFiles()
{
    m_db.close();
    Class base("Nut::Table");
    foreach (auto table, m_tables) {
        Class tableClass(Namer::getClassName(table->m_name));
        tableClass.addHeaderInclude("Table");
        tableClass.addHeaderInclude("TableSet");
        tableClass.addBaseClass(base);
        tableClass.addDeclarationMacro("Q_OBJECT");
        tableClass.setDeclareMetatype();

        Function constructor(Namer::getClassName(tableClass.name()));
        Function::Argument parent("QObject *parent", "nullptr");
        constructor.addArgument(parent);
        constructor.setPreReturnTypeDeclarationMacro("Q_INVOKABLE");

        for (auto field : table->m_fields) {
            if (field.m_databaseType.startsWith("enum")) {
                QRegularExpression re("enum\\(([^\\)]*)\\)");
                QRegularExpressionMatch matches = re.match(field.m_databaseType);
                QString enumItemsString = matches.captured(1).remove('\'');
                QStringList enumItems = enumItemsString.split(",", QString::SplitBehavior::SkipEmptyParts);
                // MySQL indexes enums from 1
                enumItems.prepend(QStringLiteral("_empty"));
                Enum fieldTypeEnum(Namer::getClassName(field.m_name) + "_Enum", enumItems);
                tableClass.addEnum(fieldTypeEnum);
                tableClass.addDeclarationMacro(QString("NUT_ENUM(%1)").arg(field.m_name), false);
            }

            if (field.m_isPrimary) {
                if (field.m_autoIncrement) {
                    tableClass.addDeclarationMacro(QString("NUT_PRIMARY_AUTO_INCREMENT(%1)")
                                                   .arg(field.m_name), false);
                } else {
                    tableClass.addDeclarationMacro(QString("NUT_PRIMARY(%1)").arg(field.m_name), false);
                }
            }

            if (!field.m_isNull) {
                tableClass.addDeclarationMacro(QString("NUT_NOT_NULL(%1)").arg(field.m_name), false);
            }

            if (field.m_length) {
                tableClass.addDeclarationMacro(QString("NUT_LEN(%1, %2)")
                                               .arg(field.m_name)
                                               .arg(field.m_length), false);
            }

            // NUT_DECLARE_FIELD has to be skipped if it is a foreign key
            bool skipFieldDeclaration = false;
            for (auto rel : table->m_relations) {
                if (rel.m_type == TableRelation::BelongsTo && rel.fieldName == field.m_name) {
                    skipFieldDeclaration = true;
                    qWarning() << "NUT_DECLARE_FIELD skipped for" << field.m_name;
                    break;
                }
            }
            if (!skipFieldDeclaration) {
                tableClass.addDeclarationMacro(QString("NUT_DECLARE_FIELD(%1, %2, %2, set%3)")
                                               .arg(FieldQtTypeLookup::getQtType(field.m_databaseType, FieldQtTypeLookup::MySQL))
                                               .arg(field.m_name)
                                               .arg(Style::upperFirst(field.m_name)));
            }
        }
        constructor.addInitializer("Nut::Table(parent)");

        Code postImplemntationCode;
        for (auto rel : table->m_relations) {
            if (rel.m_type == TableRelation::HasMany) {
                qWarning() << table->m_name << "HasMany" << rel.destinationTable->m_name;
                tableClass.addDeclarationMacro(QString("NUT_DECLARE_CHILD_TABLE(%1, %2)")
                                               .arg(Namer::getClassName(rel.destinationTable->m_name))
                                               .arg(rel.destinationTable->m_name));
                tableClass.addHeaderInclude("\"" + rel.destinationTable->m_name + ".h\"");
                constructor.addInitializer(QString("m_%1(new Nut::TableSet<%2>(this))")
                                           .arg(rel.destinationTable->m_name)
                                           .arg(Namer::getClassName(rel.destinationTable->m_name)));
                postImplemntationCode.addLine(QString("NUT_IMPLEMENT_CHILD_TABLE(%1, %2, %3)")
                                        .arg(tableClass.name(), Namer::getClassName(rel.destinationTable->m_name), rel.destinationTable->m_name));
            } else if (rel.m_type == TableRelation::BelongsTo) {
                qWarning() << table->m_name << "BelongsTo" << rel.destinationTable->m_name;
                // NUT_FOREIGN_KEY_DECLARE(type, keytype, keyname, keywrite, name, read, write, optional, related_field_name)
                tableClass.addDeclarationMacro(QString("NUT_FOREIGN_KEY_DECLARE(%1, int, %2, set%3, %4, %4, set%5, %6, %7)")
                                               .arg(Namer::getClassName(rel.destinationTable->m_name)) // type
                                               .arg(rel.fieldName) // keyname
                                               .arg(Inflector::upperFirst(rel.fieldName)) // keywrite
                                               .arg(Namer::singularize(rel.destinationTable->m_name)) // name, read
                                               .arg(Inflector::upperFirst(Namer::singularize(rel.destinationTable->m_name))) // write
                                               .arg(rel.optional ? "true" : "false") // optional
                                               .arg(rel.referencedFieldName) //
                                               );
                tableClass.addIncludes(QStringList(), QStringList() << Namer::getClassName(rel.destinationTable->m_name));

                // NUT_FOREIGN_KEY_IMPLEMENT(class, type, keytype, keyname, keywrite, name, read, write)
                postImplemntationCode.addLine(QString("NUT_FOREIGN_KEY_IMPLEMENT(%1, %2, int, %3, set%4, %5, %5, set%6)")
                                              .arg(Namer::getClassName(table->m_name)) // class
                                              .arg(Namer::getClassName(rel.destinationTable->m_name)) // type
                                              .arg(rel.fieldName) // keyname
                                              .arg(Inflector::upperFirst(rel.fieldName)) // keywrite
                                              .arg(Namer::singularize(rel.destinationTable->m_name)) // name, read
                                              .arg(Inflector::upperFirst(Namer::singularize(rel.destinationTable->m_name))) // write
                                              );
                tableClass.addInclude(QString("\"%1.h\"").arg(rel.destinationTable->m_name));
            }
        }
        tableClass.setPostImplementationCode(postImplemntationCode);
        tableClass.addFunction(constructor);

        if (m_generateCloneMethod)
            addCloneMethod(table, &tableClass);

        if (m_generateEqualsOperator)
            addEqualsOperator(table, &tableClass);

        File file;
        file.setFilename(table->m_name);
        Code code;
        code.addLine("#ifdef NUT_NAMESPACE");
        code.addLine("using namespace NUT_NAMESPACE;");
        code.addLine("#endif");
        file.addFileCode(code);
        file.clearCode();
        file.insertClass(tableClass);
        m_printer.printHeader(file);
        m_printer.printImplementation(file);
    }

    if (!generateDatabaseClass())
        return false;

    return generatePriFile();
}

void NutCodeGen::setGenerateCloneMethods(bool generateCopyConstructor)
{
    m_generateCloneMethod = generateCopyConstructor;
}

void NutCodeGen::setGenerateEqualsOperator(bool generateEqualsOperator)
{
    m_generateEqualsOperator = generateEqualsOperator;
}

bool NutCodeGen::generateDatabaseClass()
{
    Class dbClass(Namer::getClassName(m_database));
    Class base("Nut::Database");

    dbClass.addHeaderInclude("Database");
    dbClass.addHeaderInclude("Query");
    dbClass.addInclude("", "Database");
    dbClass.addInclude("TableSet", "TableSet");
    dbClass.addDeclarationMacro("Q_OBJECT");
    dbClass.addDeclarationMacro("NUT_DB_VERSION(1)");
    dbClass.addBaseClass(base);

    Function constructor(Namer::getClassName(m_database));
    constructor.addInitializer("Database()");

    for (auto table : m_tables) {
        dbClass.addHeaderInclude("\""+table->m_name + ".h\"");
        dbClass.addDeclarationMacro(
                    QString("NUT_DECLARE_TABLE(%1, %2)")
                    .arg(Namer::getClassName(table->m_name))
                    .arg(Inflector::underScore(table->m_name).toLower()), false
                    );
        constructor.addInitializer(QString("m_%1(new Nut::TableSet<%2>(this))")
                                   .arg(Inflector::underScore(table->m_name).toLower())
                                   .arg(Namer::getClassName(table->m_name)));

        constructor.addBodyLine(QString("qRegisterMetaType<%1*>();").arg(Namer::getClassName(table->m_name)));
        constructor.setPreReturnTypeDeclarationMacro("Q_INVOKABLE");
    }
    dbClass.addFunction(constructor);


    File file;
    file.setFilename(m_database);
    Code code;
    code.addLine("#ifdef NUT_NAMESPACE");
    code.addLine("using namespace NUT_NAMESPACE;");
    code.addLine("#endif");
    file.addFileCode(code);
    file.clearCode();
    file.insertClass(dbClass);
    m_printer.printHeader(file);
    m_printer.printImplementation(file);
    return true;
}

bool NutCodeGen::generatePriFile()
{
    QFile file(m_workingDir + QDir::separator() + m_database + ".pri");
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream ts(&file);
        ts << "HEADERS += \\\n";
        ts << "    $$PWD/" << m_database << (m_tables.count() ? ".h \\" : ".h") << "\n";
        for (auto tbl : m_tables) {
            ts << "    $$PWD/" << tbl->m_name << (tbl == m_tables.last() ? ".h" : ".h \\") << "\n";
        }
        ts << "\n";
        ts << "SOURCES += \\\n";
        ts << "    $$PWD/" << m_database << (m_tables.count() ? ".cpp \\" : ".cpp") << "\n";
        for (auto tbl : m_tables) {
            ts << "    $$PWD/" << tbl->m_name << (tbl == m_tables.last() ? ".cpp" : ".cpp \\") << "\n";
        }
        file.close();
        return true;
    }
    m_errorString = QObject::tr("Unable to open the %1 file for writing")
            .arg(file.fileName());
    return  false;
}

void NutCodeGen::addCloneMethod(const Table *table, KODE::Class *class_)
{
    Function copyConstructor;
    copyConstructor.setReturnType("void");
    copyConstructor.setName("clone");
    copyConstructor.addArgument("const " + Namer::getClassName(table->m_name) + " * other");

    for (auto field : table->m_fields)
        copyConstructor.addBodyLine("set" + Namer::upperFirst(field.m_name) + "(other->" + field.m_name + "());");
    class_->addFunction(copyConstructor);
}

void NutCodeGen::addEqualsOperator(const Table *table, Class *class_)
{
    Function equalsOperator;
    equalsOperator.setReturnType("bool");
    equalsOperator.setName("operator==");
    equalsOperator.addArgument("const " + Namer::getClassName(table->m_name) + " &other");

    equalsOperator.addBodyLine("return (");
    int i = 0;
    for (auto field : table->m_fields) {
        QString line = "(" + field.m_name + "() == other." + field.m_name + "())";
        if (i)
            line.prepend("&& ");
        equalsOperator.addBodyLine(line);
        i++;
    }
    equalsOperator.addBodyLine(");");
    class_->addFunction(equalsOperator);
}
