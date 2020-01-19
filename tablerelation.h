#ifndef TABLERELATION_H
#define TABLERELATION_H

#include "table.h"

class Table;

class TableRelation
{
public:
    TableRelation();

    enum RelationType {
        HasMany,
        BelongsTo
    };

    RelationType m_type;
    QString fieldName;
    Table *destinationTable = nullptr;
};

#endif // TABLERELATION_H
