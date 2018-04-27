#include <QCoreApplication>
#include <QDebug>
#include "enumtodebug.h"
#include <QTextStream>

int main(int /*argc*/, char * /*argv*/[])
{
    QTextStream cout(stdout);
    QString rawCode, str;

    bool isTest = false;
    if (!isTest) {
        QTextStream cin(stdin);
        rawCode = cin.readAll();
    }

    EnumToDebug e;
    e.fromCode(rawCode, str);

    cout << str;
    cout.flush();

    return 0;
}
