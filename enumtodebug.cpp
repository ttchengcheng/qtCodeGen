#include "enumtodebug.h"
#include <QString>
#include <Qlist>
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QMultiMap>

EnumToDebug::EnumToDebug()
{

}

bool EnumToDebug::fromCode(const QString &rawCode, QString &result)
{
    static const QString TEST_STRING = R"(
                                       enum Corner {
                                           TopLeftCorner = 0x00000,
                                           TopRightCorner = 0x00001,
                                           BottomLeftCorner = 0x00002,
                                           BottomRightCorner = 0x00003
                                       };
                                         )";

    auto code = TEST_STRING;
    code.replace(QRegularExpression("\\r"), "\n")
            .replace(QRegularExpression("\\s+//.*\\n"), "")
            .replace(QRegularExpression("/\\*.*\\*/"), "")
            .replace(QRegularExpression("\\s"), " ")
            .replace(QRegularExpression(" {2,}"), " ");

    QRegularExpression re("\\s?enum [class]?\\s?(\\b\\w+\\b)\\s?:?(.*)\\{(.+)\\}\\s?;\\s");
    auto match = re.match(code);
    if (!match.hasMatch()) { return false; }

    QString name = match.captured(1);
    QString type = match.captured(2).trimmed();
    QString list = match.captured(3);

    qDebug() << endl;
    qDebug() << "name" << name;
    qDebug() << "type" << type;

    int value = 0;
    QMultiMap<int, QString> items;
    QRegularExpression lineRe1("\\s*(\\b\\w+\\b)\\s?=(\\S+)\\s?");
    QRegularExpression lineRe2("\\s*(\\b\\w+\\b)\\s?");
    QString item, valueString;
    bool ok;
    for (const auto &line : list.split(',')) {
        if (line.trimmed().isEmpty()) { continue; }
        match = lineRe1.match(line);
        if (match.hasMatch()) {
            item = match.captured(1);
            valueString = match.captured(2).trimmed();
            if (valueString.startsWith("0x"), Qt::CaseInsensitive) {
                value = valueString.toInt(&ok, 16);
            }
            else {
                value = valueString.toInt(&ok, 10);
            }
            if (!ok) { return false; }
        }
        else {
            match = lineRe2.match(line);
            if (match.hasMatch()) {
                item = match.captured(1);
                value = value + 1;
            }
            else {
                return false;
            }
        }

        items.insertMulti(value, item);
    }
    for (auto k : items.keys()) {
        for (const auto &v : items.values(k)) {
            qDebug() << "[" << k << "] : " << v;
        }
    }
    return true;
}
