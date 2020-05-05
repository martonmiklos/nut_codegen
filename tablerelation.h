#ifndef TABLERELATION_H
#define TABLERELATION_H

#include "table.h"

class Table;

class TableRelation
{
public:
    TableRelation() = default;

    enum RelationType {
        HasMany,
        BelongsTo
    };

    RelationType m_type;
    QString fieldName, referencedFieldName;
    Table *destinationTable = nullptr;
    bool optional = false;
};

#endif // TABLERELATION_H
