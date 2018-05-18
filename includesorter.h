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
        ProFileInvalid,
        ProFileReadFailed,

        LicenseDuplicated,
        GuardIfDefInvalid,
        GuardIfDefDuplicated,
        GuardDefineInvalid,
        GuardDefineDuplicated,
        ContinueSignInvalid,
    };
    Q_ENUM(ErrorCode)

    IncludeSorter();

    QString proFile() const;
    void setProFile(const QString &proFile);

    bool read();
    bool hasError() const;


    void setPchName(const QString &pchName);

private:
    bool parseProFile();
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

    QString mProFile;
    QString mPchName;
    QMap<QString, QString> mHeaders;
    QSet<QString> mPchItems;
    struct HeaderStatus
    {
        QFileInfo currentFile;
        QString mLicenseHolder;
        QString mGuardIfDef;
        QString mGuardDefine;
        QStringList mIncludes;
        QStringList preprocessings;
        QStringList mLines;
        int lineNo;
        int ifLevel;
        bool isHeader;
        bool outOfIncludeArea;
        bool defineContinue;
        ErrorCode mErrorCode;

        void clear();
    } mStatus;
};

#endif // INCLUDESORTER_H
