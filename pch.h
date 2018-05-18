//license-placeholder 2018-4-13 Tao Cheng
#ifndef PCH_H
#define PCH_H


#if defined __cplusplus

#include <QAbstractItemModel>
#include <QAbstractListModel>
#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLibraryInfo>
#include <QList>
#include <QLocale>
#include <QMap>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMimeData>
#include <QMultiMap>
#include <QObject>
#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QRegExp>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QSettings>
#include <QSignalMapper>
#include <QSize>
#include <QSizeF>
#include <QSortFilterProxyModel>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QTimer>
#include <QUrl>
#include <QVariant>
#include <QVector>
#include <QtMath>
#include <algorithm>
#include <functional>
#include <math.h>
#include <stdio.h>

#endif

// ----------------------misc---------------------------
#define _connect(obj, signal) connect(obj, SIGNAL(signal), SLOT(signal))
#define _icon_path ":/resource/icon/16x16/"

// ----------------------debugging---------------------------
#define _info(exp, ...) \
     qInfo(exp, __VA_ARGS__); \

#define _expect(exp, ...) \
    if (!(exp)) { \
        qWarning(__VA_ARGS__); \
    }

#define _must(exp, ...) \
    if (!(exp)) { \
        qFatal(__VA_ARGS__); \
    }


// ----------------------singleton---------------------------
#define _declare_singleton(clsName) \
public: \
    static clsName *instance();  \
    static void deleteInstance(); \
    \
private:  \
    clsName();  \
    ~clsName(); \
    static clsName *mInstance;

#define _implement_singleton(clsName) \
    clsName *clsName::mInstance = nullptr; \
    clsName *clsName::instance() \
    { \
        return mInstance ? mInstance : (mInstance = new clsName());\
    } \
    void clsName::deleteInstance() \
    { \
        delete mInstance; \
        mInstance = nullptr; \
    }

#endif // PCH_H
