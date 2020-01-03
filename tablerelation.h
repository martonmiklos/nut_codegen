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
    Table *tableA = nullptr, *tableB = nullptr;
};

#endif // TABLERELATION_H
