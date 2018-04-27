#ifndef ENUMTODEBUG_H
#define ENUMTODEBUG_H

#include <QString>
#include <QMultiMap>

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
