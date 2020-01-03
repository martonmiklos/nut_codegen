#ifndef TABLE_H
#define TABLE_H

#include <QList>

#include "tablefield.h"
#include "tablerelation.h"

class TableRelation;

class Table
{
public:
    Table(const QString &name);

    QString m_name;

    QList<TableField> m_fields;
    QList<TableRelation> m_relations;

    QString m_displayFieldName;
};

#endif // TABLE_H
