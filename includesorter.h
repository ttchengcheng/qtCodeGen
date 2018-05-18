//license-placeholder 2018-5-17 Tao Cheng

#ifndef INCLUDESORTER_H
#define INCLUDESORTER_H

#include "pch.h"

class IncludeSorter : public QObject
{
    Q_OBJECT
public:
    enum ErrorCode {
        Ok = 0,
        ReadFileFailed,
        ReadDirFailed,
        WriteFileFailed,

        LicenseDuplicated,
        GuardIfDefInvalid,
        GuardIfDefDuplicated,
        GuardDefineInvalid,
        GuardDefineDuplicated,
    };
    Q_ENUM(ErrorCode)

    IncludeSorter();

    QString path() const;
    void setPath(const QString &path);

    bool read();
    bool hasError() const;


private:
    bool handleFile(const QString &filePath);
    bool parseLine(const QString &line);
    bool readHeaders();
    bool handleFiles();
    bool readPch();

    bool checkLicense(const QString &line);
    bool checkInclude(const QString &line);
    bool checkPreprocessingDirective(const QString &line);
    bool checkOutOfIncludeArea(const QString &line);

    void setError(ErrorCode err);

    QDir mRoot;
    QMap<QString, QString> mHeaders;
    QSet<QString> mPchItems;
    struct HeaderStatus
    {
        QString mLicenseHolder;
        QString mGuardIfDef;
        QString mGuardDefine;
        QStringList mIncludes;
        QStringList mLines;
        int lineNo;
        bool isHeader;
        bool outOfIncludeArea;
        ErrorCode mErrorCode;

        void clear();
    } mStatus;
};

#endif // INCLUDESORTER_H
