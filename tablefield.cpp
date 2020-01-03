#include "tablefield.h"


TableField::TableField(const QString &name, QString databaseType, bool isPrimary) :
    m_name(name),
    m_databaseType(databaseType),
    m_isPrimary(isPrimary)
{

}

bool TableField::autoIncrement() const
{
    return m_autoIncrement;
}

void TableField::setAutoIncrement(bool autoIncrement)
{
    m_autoIncrement = autoIncrement;
}
