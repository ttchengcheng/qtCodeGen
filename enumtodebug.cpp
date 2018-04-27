#include "enumtodebug.h"
#include <QDebug>
#include <QList>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QString>
#include <QTextStream>

bool EnumToDebug::fromCode(const QString &rawCode, QString &result)
{
    static const QString TEST_STRING = R"(
       enum MouseButton {
           NoButton         = 0x00000000,
           LeftButton       = 0x00000001,
           RightButton      = 0x00000002,
           MidButton        = 0x00000004, // ### Qt 6: remove me
           MiddleButton     = MidButton,
           BackButton       = 0x00000008,
           XButton1         = BackButton,
           ExtraButton1     = XButton1,
           ForwardButton    = 0x00000010,
           XButton2         = ForwardButton,
           ExtraButton2     = ForwardButton,
           TaskButton       = 0x00000020,
           ExtraButton3     = TaskButton,
           ExtraButton4     = 0x00000040,
           ExtraButton5     = 0x00000080,
           ExtraButton6     = 0x00000100,
           ExtraButton7     = 0x00000200,
           ExtraButton8     = 0x00000400,
           ExtraButton9     = 0x00000800,
           ExtraButton10    = 0x00001000,
           ExtraButton11    = 0x00002000,
           ExtraButton12    = 0x00004000,
           ExtraButton13    = 0x00008000,
           ExtraButton14    = 0x00010000,
           ExtraButton15    = 0x00020000,
           ExtraButton16    = 0x00040000,
           ExtraButton17    = 0x00080000,
           ExtraButton18    = 0x00100000,
           ExtraButton19    = 0x00200000,
           ExtraButton20    = 0x00400000,
           ExtraButton21    = 0x00800000,
           ExtraButton22    = 0x01000000,
           ExtraButton23    = 0x02000000,
           ExtraButton24    = 0x04000000,
           AllButtons       = 0x07ffffff,
           MaxMouseButton   = ExtraButton24,
           // 4 high-order bits remain available for future use (0x08000000 through 0x40000000).
           MouseButtonMask  = 0xffffffff
        };
    )";

    clear();
    if (!parse(rawCode.isEmpty() ? TEST_STRING : rawCode)) { return false; }
    if (!generate(result)) { return false; }

    qInfo("\n%s", qPrintable(result));
    return true;
}

void EnumToDebug::clear()
{
    mName.clear();
    mType.clear();
    mItems.clear();
}

bool EnumToDebug::parse(const QString &rawCode)
{
    QTextStream err(stderr);

    auto code = rawCode;
    code
    .replace(QRegularExpression("\\r"), "\n")           // \r to \n
    .replace(QRegularExpression("\\s+//.*\\n"), "")     // remove line comments
    .replace(QRegularExpression("/\\*.*\\*/"), "")      // remove block comments
    .replace(QRegularExpression("\\s"), " ")            // all white spaces to space
    .replace(QRegularExpression(" {2,}"), " ");         // continuous spaces to one space

    QRegularExpression re("\\s?enum [class]?\\s?(\\b\\w+\\b)\\s?:?(.*)\\{(.+)\\}\\s?;\\s");
    auto match = re.match(code);
    if (!match.hasMatch()) {
        err << "whole input code is not valid" << endl;
        return false;
    }

    mName = match.captured(1);
    mType = match.captured(2).trimmed();
    QString list = match.captured(3);

    qlonglong value = 0;
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
                auto it = dict.find(valueString);
                if (it != dict.end()) {
                    value = it.value();
                }
                else {
                    qlonglong composedValue = 0;
                    auto composedList = valueString.split('|');
                    if (composedList.isEmpty()) {

                        return false;
                    }
                    for (auto otherItem : composedList) {
                        otherItem = otherItem.trimmed();
                        auto it = dict.find(otherItem);
                        if (it == dict.end()) {
                            err << "can't find definition of ["
                                << otherItem
                                << "] for ["
                                << item
                                << "]"
                                << endl;
                            return false;
                        }
                        composedValue |= it.value();
                    }
                    value = composedValue;
                }
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

        mItems.insertMulti(value, item);
        dict[item] = value;
        qDebug() << item << " = " << value << endl;
    }
    return true;
}

bool EnumToDebug::generate(QString &result)
{
    static const QString FUNCTION_TEMPLATE =
        "#ifndef QT_NO_DEBUG_STREAM\n" \
        "QDebug operator<<(QDebug dbg, const %1 item)\n" \
        "{ \n" \
        "    QDebugStateSaver saver(dbg);\n" \
        "    dbg.nospace();\n" \
        "    dbg << \"%1[\";\n" \
        "    switch (item) {\n" \
        "%2\n" \
        "    }\n" \
        "    dbg << '(' << static_cast<%3>(item) << ')' << ']';\n" \
        "    return dbg;\n" \
        "}\n" \
        "#endif\n";

    static const QString ITEM_TEMPLATE =
        "    case %1::%2:\n" \
        "        dbg << \"%2\"; break;\n";

    QString items;
    for (auto k : mItems.keys()) {
        for (const auto &v : mItems.values(k)) {
            items.append(ITEM_TEMPLATE.arg(mName).arg(v));
        }
    }
    if (mType.isEmpty()) { mType = "int64_t"; }
    result = FUNCTION_TEMPLATE.arg(mName).arg(items).arg(mType);

    return true;
}

