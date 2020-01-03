#ifndef TABLEFIELD_H
#define TABLEFIELD_H

#include <QString>

class TableField
{
public:
    TableField(const QString &name, QString databaseType, bool isPrimary = false);

    QString m_name;
    QString m_qtType;
    QString m_databaseType;

    bool m_isPrimary = false;
    bool m_autoIncrement = false;
    int m_length = 0;
    bool autoIncrement() const;
    void setAutoIncrement(bool autoIncrement);
    bool m_isNull = false;
};

#endif // TABLEFIELD_H
