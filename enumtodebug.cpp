#include "enumtodebug.h"
#include <QString>
#include <QList>
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QMultiMap>

bool EnumToDebug::fromCode(const QString & /*rawCode*/, QString & /*result*/)
{
    static const QString TEST_STRING = R"(
       enum InputMethodQuery {
           ImEnabled = 0x1,
           ImCursorRectangle = 0x2,
           ImMicroFocus = 0x2, // deprecated
           ImFont = 0x4,
           ImCursorPosition = 0x8,
           ImSurroundingText = 0x10,
           ImCurrentSelection = 0x20,
           ImMaximumTextLength = 0x40,
           ImAnchorPosition = 0x80,
           ImHints = 0x100,
           ImPreferredLanguage = 0x200,

           ImAbsolutePosition = 0x400,
           ImTextBeforeCursor = 0x800,
           ImTextAfterCursor = 0x1000,
           ImEnterKeyType = 0x2000,
           ImAnchorRectangle = 0x4000,
           ImInputItemClipRectangle = 0x8000,

           ImPlatformData = 0x80000000,
           ImQueryInput = ImCursorRectangle | ImCursorPosition | ImSurroundingText |
                          ImCurrentSelection | ImAnchorRectangle | ImAnchorPosition,
           ImQueryAll = 0xffffffff
       };
    )";
    auto code = TEST_STRING;
    code
    .replace(QRegularExpression("\\r"), "\n")
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

    qlonglong value = 0;
    QMultiMap<qlonglong, QString> items;
    QMap<QString, qlonglong> dict;
    QRegularExpression lineRe1("\\s?(\\b\\w+\\b)\\s?=(.*)\\s?");
    QRegularExpression lineRe2("\\s?(\\b\\w+\\b)\\s?");
    QString item, valueString;
    bool ok;
    for (const auto &line : list.split(',')) {
        if (line.trimmed().isEmpty()) { continue; }
        match = lineRe1.match(line);
        if (match.hasMatch()) {
            item = match.captured(1);
            valueString = match.captured(2).trimmed();
            if (valueString.startsWith("0x", Qt::CaseInsensitive)) {
                value = valueString.toLongLong(&ok, 16);
            }
            else {
                value = valueString.toLongLong(&ok, 10);
            }
            if (!ok) {
                qlonglong composedValue = 0;
                auto composedList = valueString.split('|');
                if (composedList.isEmpty()) { return false; }
                for (auto otherItem : composedList) {
                    otherItem = otherItem.trimmed();
                    auto it = dict.find(otherItem);
                    if (it == dict.end()) { return false; }
                    composedValue |= it.value();
                }
                value = composedValue;
            }
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
        dict[item] = value;
    }
    for (auto k : items.keys()) {
        for (const auto &v : items.values(k)) {
            qDebug() << "[" << k << "] : " << v;
        }
    }
    return true;
}
