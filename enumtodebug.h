#ifndef ENUMTODEBUG_H
#define ENUMTODEBUG_H

#include <QString>

class EnumToDebug
{
public:
    EnumToDebug() = default;
    bool fromCode(const QString &code, QString &result);

private:
    QString mType;
    QString mName;
};

#endif // ENUMTODEBUG_H
