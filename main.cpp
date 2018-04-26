#include <QCoreApplication>
#include "enumtodebug.h"

int main(int argc, char *argv[])
{
    EnumToDebug e;
    QString str;
    e.fromCode("", str);

    QCoreApplication a(argc, argv);

    return a.exec();
}
