//license-placeholder 2018-5-17 Tao Cheng

#include "enumtodebug.h"
#include "includesorter.h"

int main(int argc, char *argv[])
{
    QTextStream cout(stdout);
    QTextStream cin(stdin);

    auto showHelp = [&cout]() {
        cout << "usage: cpptool [[-si <dir>] | [-ed <enum>] | [t <function>]]           " << endl;
        cout << "                                                                       " << endl;
        cout << "   -si <dir>          sort include area of h/cpp files in <folder>     " << endl;
        cout << "   -ed <enum>         generate QDebug operator<<() from enum definition" << endl;
        cout << "   -t <function>      test function                                    " << endl;
        cout.flush();
    };

    if (argc < 2) { showHelp(); return 0; }

    if (QLatin1Literal(argv[1]) == QLatin1Literal("-si")) {
        if (argc < 3) { showHelp(); return 0; }

        IncludeSorter ir;
        ir.setProFile(argv[2]);

        ir.read();
    }
    else if (QLatin1Literal(argv[1]) == QLatin1Literal("-ed")) {
        QString result;
        QString rawCode = cin.readAll();

        EnumToDebug e;
        e.fromCode(rawCode, result);

        cout << result;
        cout.flush();
    }
    else if (QLatin1Literal(argv[1]) == QLatin1Literal("-t")) {
        if (argc < 3) { showHelp(); return 0; }

        if (QLatin1Literal(argv[2]) == QLatin1Literal("ed")) {
            QString result;
            EnumToDebug e;
            e.fromCode(QStringLiteral(""), result);

            cout << result;
            cout.flush();
        }
        else {

        }
    }
    return 0;
}
