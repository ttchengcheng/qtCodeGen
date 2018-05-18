//license-placeholder 2018-5-17 Tao Cheng

#ifndef ENUMTODEBUG_H
#define ENUMTODEBUG_H

#include "pch.h"

class EnumToDebug
{
public:
    bool fromCode(const QString &code, QString &result);

private:
    void clear();
    bool parse(const QString &rawCode);
    bool generate(QString &result);

    QString mType;
    QString mName;
    QMultiMap<qlonglong, QString> mItems;
};

#endif // ENUMTODEBUG_H
