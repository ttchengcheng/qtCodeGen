//license-placeholder 2018-5-17 Tao Cheng

#include "includesorter.h"

IncludeSorter::IncludeSorter() : mPchName(QLatin1Literal("pch.h"))
{
    readPch();
}

QString IncludeSorter::proFile() const
{
    return mProFile;
}

void IncludeSorter::setProFile(const QString &path)
{
    qDebug() << "pro file path: " << path;
    mProFile = path;
}

bool IncludeSorter::readHeaders()
{
    QDir dir = QFileInfo(mProFile).absoluteDir().filePath(QStringLiteral("src"));
    QDirIterator it(dir, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString path = it.next();
        if (!path.endsWith(QLatin1String(".h"))) { continue; }
        auto before = QFileInfo(path).fileName();
        auto after = dir.relativeFilePath(path);
        if (before != after) {
            mHeaders.insert(before, after);
        }
    }
    for (const auto &pair : mHeaders.toStdMap()) {
        qDebug() << pair.first << " -- " << pair.second;
    }
    return true;
}

bool IncludeSorter::handleFiles()
{
    QDir dir = QFileInfo(mProFile).absoluteDir().filePath(QStringLiteral("src"));
    QDirIterator it(dir, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString path = it.next();
        if (!handleFile(path)) { return false; }
    }
    return true;
}

bool IncludeSorter::checkLicense(const QString &line)
{
    if (line.startsWith(QLatin1Literal("//license-placeholder"))) {
        if (mStatus.mLicenseHolder.isEmpty()) {
            mStatus.mLicenseHolder = line;
            qDebug() << "[" << mStatus.lineNo << "]" << "LicenseHolder: " << mStatus.mLicenseHolder;
        }
        else {
            setError(LicenseDuplicated);
        }
        return true;
    }
    return false;
}

bool IncludeSorter::read()
{
    try {
        parseProFile();
        readHeaders();
        handleFiles();
    }
    catch (...) {
        return false;
    }
    return true;
}

bool IncludeSorter::hasError() const
{
    return mStatus.mErrorCode != ErrorCode::Ok;
}

bool IncludeSorter::checkInclude(const QString &line)
{
    bool isQuoteType = true;
    auto match = QRegularExpression(QStringLiteral("\\s*#include\\s*\"(.*)\"\\s*")).match(line);
    if (!match.hasMatch()) {
        match = QRegularExpression(QStringLiteral("\\s*#include\\s*<(.*)>\\s*")).match(line);
        isQuoteType = false;
    }

    if (!match.hasMatch()) { return false; }

    QString item = match.captured(1);
    if (mPchItems.contains(item)) { return false; }
    if (!isQuoteType) {
        qDebug() << "WARNING: not included in pch: [" << line << "]";
    }

    auto index = item.lastIndexOf(QLatin1Char('/'));
    auto fileName = index >= 0 ? item.mid(index + 1) : item;

    // ignore #inlcude "xxxxx.h" for "xxxxx.cpp"
    if (!mStatus.isHeader && fileName == mStatus.currentFile.baseName() + ".h") {
        return true;
    }

    auto it = mHeaders.find(fileName);
    if (it == mHeaders.end() || item == it.value()) {
        mStatus.mIncludes.push_back(line);
        qDebug() << "[" << mStatus.lineNo << "]" << "keep: " << line;
        return true;
    }

    auto newLine = line;
    newLine.replace(
        isQuoteType ? QRegularExpression(QStringLiteral("#include\\s*\"(.*)\"\\s*"))
        : QRegularExpression(QStringLiteral("#include\\s*<(.*)>\\s*")),
        QString(QLatin1Literal("#include \"%1\"")).arg(it.value()));

    qDebug() << "[" << mStatus.lineNo << "]" << "replace: " << line << " to " << newLine;
    mStatus.mIncludes.push_back(newLine);

    return true;
}
bool IncludeSorter::checkPreprocessingDirective(const QString &line)
{
    Q_ASSERT(!mStatus.outOfIncludeArea);
    if (mStatus.outOfIncludeArea) { return true; }

    bool hasContinueSign = line.endsWith(QLatin1Char('\\'));
    if (mStatus.defineContinue) {
        mStatus.preprocessings.push_back(line);
        if (!hasContinueSign) {
            mStatus.defineContinue = false;
            if (!line.trimmed().isEmpty()) { mStatus.preprocessings.push_back(QLatin1Literal("")); }
        }
        return true;
    }
    if (hasContinueSign) {
        if (QRegularExpression(QLatin1Literal("\\s*#\\s*define\\s+.*")).match(line).hasMatch()) {
            mStatus.defineContinue = true;
            mStatus.preprocessings.push_back(line);
            return true;
        }
        else {
            setError(ContinueSignInvalid);
            return false;
        }
    }

    if (line.trimmed().isEmpty()) {
        qDebug() << "[" << mStatus.lineNo << "]" << "Empty line";
        return true;
    }

    if (mStatus.isHeader) {
        if (QRegularExpression(QLatin1Literal("\\s*#\\s*ifndef.+_H")).match(line).hasMatch()) {
            if (!mStatus.mGuardIfDef.isEmpty()) { setError(GuardIfDefDuplicated); return false; }
            mStatus.mGuardIfDef = QString(QStringLiteral("#ifndef %1_H")).arg(mStatus.currentFile.baseName().toUpper());
            return true;
        }
        if (QRegularExpression(QLatin1Literal("\\s*#\\s*define.+_H")).match(line).hasMatch()) {
            if (mStatus.mGuardIfDef.isEmpty()) { setError(GuardIfDefInvalid); return false; }
            if (!mStatus.mGuardDefine.isEmpty()) { setError(GuardDefineDuplicated); return false; }
            mStatus.mGuardDefine = QString(QStringLiteral("#define %1_H")).arg(mStatus.currentFile.baseName().toUpper());
            return true;
        }

        if (mStatus.mGuardIfDef.isEmpty()) { setError(GuardIfDefInvalid); return false; }
        if (mStatus.mGuardDefine.isEmpty()) { setError(GuardDefineInvalid); return false; }
    }

    if (QRegularExpression(QLatin1Literal("\\s*#\\s*if.+")).match(line).hasMatch()) {
        mStatus.ifLevel++;
        mStatus.preprocessings.push_back(line);
        return true;
    }
    if (QRegularExpression(QLatin1Literal("\\s*#\\s*endif\\s*")).match(line).hasMatch()) {
        mStatus.ifLevel--;
        mStatus.preprocessings.push_back(line);

        if (mStatus.ifLevel == 0) {
            mStatus.preprocessings.push_back(QLatin1Literal(""));
        }
        return true;
    }

    return checkInclude(line);
}

bool IncludeSorter::checkOutOfIncludeArea(const QString &line)
{
    if (mStatus.outOfIncludeArea) { return true; }
    if (mStatus.defineContinue || mStatus.ifLevel > 0) { return false; }
    if (line.trimmed().isEmpty()) { return false; }

    if (!line.startsWith("//license") &&
            !QRegularExpression(QLatin1Literal("\\s*#.*")).match(line).hasMatch()) {
        mStatus.outOfIncludeArea = true;
        qDebug() << "[" << mStatus.lineNo << "]" << "Out of Include Area";
        return true;
    }
    return false;
}

void IncludeSorter::setError(IncludeSorter::ErrorCode err)
{
    mStatus.mErrorCode = err;
    qDebug() << "[" << mStatus.lineNo << "]" << "Error: " << mStatus.mErrorCode;

    throw mStatus.mErrorCode;
}

void IncludeSorter::setPchName(const QString &pchName)
{
    mPchName = pchName;
}

bool IncludeSorter::parseProFile()
{
    mPchName.clear();
    if (!mProFile.endsWith(QStringLiteral(".pro"))) { setError(ProFileInvalid); return false; }

    QFile file(mProFile);
    if (!file.open(QFile::ReadOnly)) { setError(ProFileReadFailed); return false; }

    QTextStream ts(&file);
    while (!ts.atEnd()) {
        const auto &line = ts.readLine().trimmed();
        if (line.startsWith(QStringLiteral("PRECOMPILED_HEADER"))) {
            auto parts = line.split(QLatin1Char('='));
            if (parts.length() == 2) {
                mPchName = parts[1].trimmed();
                auto index = mPchName.lastIndexOf(QLatin1Char('/'));
                if (index >= 0) {
                    mPchName = mPchName.mid(index + 1);
                }
            }
            if (mPchName.isEmpty()) {
                qDebug() << "WARNING: invalid PRECOMPILED_HEADER setting in " << mProFile;
            }
            else {
                qDebug() << "pch is: " << mPchName;
            }
            break;
        }
    }
    file.close();
    return true;
}

bool IncludeSorter::handleFile(const QString &filePath)
{
    if (!mPchName.isEmpty() && filePath.endsWith(mPchName)) { return true; }

    mStatus.clear();
    mStatus.currentFile.setFile(filePath);

    bool isHeader = filePath.endsWith(QStringLiteral(".h"));
    bool isCpp = filePath.endsWith(QStringLiteral(".cpp"));
    if (!isHeader && !isCpp) { return true; }

    qDebug() << endl << "start parsing file [" << filePath << "]...";
    mStatus.isHeader = isHeader;

    // read lines
    {
        QFile file(filePath);
        if (!file.open(QFile::ReadOnly)) { setError(ReadFileFailed); return false; }
        QTextStream ts(&file);
        for (; !ts.atEnd(); ++mStatus.lineNo) {
            parseLine(ts.readLine());
        }
    }

    if (hasError()) { return false; }

    if (mStatus.mLicenseHolder.isEmpty()) {
        mStatus.mLicenseHolder = QStringLiteral("//license-placeholder 2018-5-17 Tao Cheng");
    }
    std::sort(mStatus.mIncludes.begin(), mStatus.mIncludes.end());

    // write
    {
        QFile file(filePath);
        if (!file.open(QFile::WriteOnly)) { setError(WriteFileFailed); return false; }

        QTextStream ts(&file);
        ts << mStatus.mLicenseHolder << endl;
        ts << endl;
        if (mStatus.isHeader) {
            ts << mStatus.mGuardIfDef << endl;
            ts << mStatus.mGuardDefine << endl;
            ts << endl;
        }
        if (mStatus.isHeader) {
            ts << "#include \"" << mPchName << "\"" << endl;
        }
        else {
            ts << "#include \"" << mStatus.currentFile.baseName() + ".h\"" << endl;
        }
        for (const auto &line : mStatus.mIncludes) {
            ts << line << endl;
        }
        ts << endl;
        for (const auto &line : mStatus.preprocessings) {
            ts << line << endl;
        }
        if (!mStatus.preprocessings.isEmpty() && !mStatus.preprocessings.last().isEmpty()) ts << endl;
        for (const auto &line : mStatus.mLines) {
            ts << line << endl;
        }
    }

    if (hasError()) { return false; }

    return true;
}

bool IncludeSorter::parseLine(const QString &line)
{
    if (checkOutOfIncludeArea(line)) {
        mStatus.mLines.push_back(line);
        return true;
    }
    else {
        if (checkLicense(line)) { return true; }
        if (checkPreprocessingDirective(line)) { return true; }
        return false;
    }
}

void IncludeSorter::HeaderStatus::clear()
{
    currentFile.setFile(QStringLiteral(""));
    mLicenseHolder.clear();
    mGuardIfDef.clear();
    mGuardDefine.clear();
    mIncludes.clear();
    preprocessings.clear();
    mLines.clear();
    outOfIncludeArea = false;
    ifLevel = 0;
    defineContinue = false;
    isHeader = true;
    mErrorCode = Ok;
    lineNo = 0;
}

bool IncludeSorter::readPch()
{
    mPchItems = QSet<QString> {
        QLatin1Literal("QAbstractItemModel"),
        QLatin1Literal("QAbstractListModel"),
        QLatin1Literal("QAction"),
        QLatin1Literal("QApplication"),
        QLatin1Literal("QBoxLayout"),
        QLatin1Literal("QByteArray"),
        QLatin1Literal("QCheckBox"),
        QLatin1Literal("QClipboard"),
        QLatin1Literal("QCloseEvent"),
        QLatin1Literal("QColor"),
        QLatin1Literal("QColorDialog"),
        QLatin1Literal("QComboBox"),
        QLatin1Literal("QCompleter"),
        QLatin1Literal("QContextMenuEvent"),
        QLatin1Literal("QCoreApplication"),
        QLatin1Literal("QCursor"),
        QLatin1Literal("QDataStream"),
        QLatin1Literal("QDate"),
        QLatin1Literal("QDateTime"),
        QLatin1Literal("QDebug"),
        QLatin1Literal("QDesktopServices"),
        QLatin1Literal("QDesktopWidget"),
        QLatin1Literal("QDialog"),
        QLatin1Literal("QDir"),
        QLatin1Literal("QDirIterator"),
        QLatin1Literal("QDockWidget"),
        QLatin1Literal("QEvent"),
        QLatin1Literal("QFile"),
        QLatin1Literal("QFileDialog"),
        QLatin1Literal("QFileInfo"),
        QLatin1Literal("QFocusEvent"),
        QLatin1Literal("QFrame"),
        QLatin1Literal("QGraphicsItem"),
        QLatin1Literal("QGraphicsSceneMouseEvent"),
        QLatin1Literal("QGraphicsView"),
        QLatin1Literal("QGridLayout"),
        QLatin1Literal("QGuiApplication"),
        QLatin1Literal("QHBoxLayout"),
        QLatin1Literal("QHash"),
        QLatin1Literal("QHeaderView"),
        QLatin1Literal("QHelpEvent"),
        QLatin1Literal("QIODevice"),
        QLatin1Literal("QIcon"),
        QLatin1Literal("QImage"),
        QLatin1Literal("QJsonArray"),
        QLatin1Literal("QJsonDocument"),
        QLatin1Literal("QJsonObject"),
        QLatin1Literal("QKeyEvent"),
        QLatin1Literal("QKeySequence"),
        QLatin1Literal("QLabel"),
        QLatin1Literal("QLayout"),
        QLatin1Literal("QLibraryInfo"),
        QLatin1Literal("QLineEdit"),
        QLatin1Literal("QList"),
        QLatin1Literal("QListWidget"),
        QLatin1Literal("QLocale"),
        QLatin1Literal("QMainWindow"),
        QLatin1Literal("QMap"),
        QLatin1Literal("QMenu"),
        QLatin1Literal("QMessageBox"),
        QLatin1Literal("QMetaEnum"),
        QLatin1Literal("QMetaObject"),
        QLatin1Literal("QMimeData"),
        QLatin1Literal("QMouseEvent"),
        QLatin1Literal("QMultiMap"),
        QLatin1Literal("QObject"),
        QLatin1Literal("QOpenGLFunctions"),
        QLatin1Literal("QOpenGLShaderProgram"),
        QLatin1Literal("QOpenGLWidget"),
        QLatin1Literal("QOpenGLWindow"),
        QLatin1Literal("QOpenGLTexture"),
        QLatin1Literal("QPaintEvent"),
        QLatin1Literal("QPainter"),
        QLatin1Literal("QPalette"),
        QLatin1Literal("QPen"),
        QLatin1Literal("QPinchGesture"),
        QLatin1Literal("QPlainTextEdit"),
        QLatin1Literal("QPoint"),
        QLatin1Literal("QPointF"),
        QLatin1Literal("QPushButton"),
        QLatin1Literal("QRect"),
        QLatin1Literal("QRectF"),
        QLatin1Literal("QRegExp"),
        QLatin1Literal("QRegularExpression"),
        QLatin1Literal("QRegion"),
        QLatin1Literal("QResizeEvent"),
        QLatin1Literal("QSaveFile"),
        QLatin1Literal("QScrollArea"),
        QLatin1Literal("QScrollBar"),
        QLatin1Literal("QSet"),
        QLatin1Literal("QSettings"),
        QLatin1Literal("QShortcut"),
        QLatin1Literal("QSignalMapper"),
        QLatin1Literal("QSize"),
        QLatin1Literal("QSizeF"),
        QLatin1Literal("QSortFilterProxyModel"),
        QLatin1Literal("QSplitter"),
        QLatin1Literal("QStandardPaths"),
        QLatin1Literal("QString"),
        QLatin1Literal("QStringList"),
        QLatin1Literal("QStyle"),
        QLatin1Literal("QStyleOption"),
        QLatin1Literal("QStyleOptionGraphicsItem"),
        QLatin1Literal("QTabBar"),
        QLatin1Literal("QTabWidget"),
        QLatin1Literal("QTextBrowser"),
        QLatin1Literal("QTextStream"),
        QLatin1Literal("QTimer"),
        QLatin1Literal("QToolBar"),
        QLatin1Literal("QToolButton"),
        QLatin1Literal("QTranslator"),
        QLatin1Literal("QTreeView"),
        QLatin1Literal("QTreeWidget"),
        QLatin1Literal("QTreeWidgetItem"),
        QLatin1Literal("QUndoCommand"),
        QLatin1Literal("QUndoGroup"),
        QLatin1Literal("QUndoStack"),
        QLatin1Literal("QUndoView"),
        QLatin1Literal("QUrl"),
        QLatin1Literal("QVBoxLayout"),
        QLatin1Literal("QValidator"),
        QLatin1Literal("QVariant"),
        QLatin1Literal("QVector"),
        QLatin1Literal("QWhatsThis"),
        QLatin1Literal("QWheelEvent"),
        QLatin1Literal("QWidget"),
        QLatin1Literal("QXmlStreamAttributes"),
        QLatin1Literal("QXmlStreamReader"),
        QLatin1Literal("QXmlStreamWriter"),
        QLatin1Literal("QtMath"),
        QLatin1Literal("algorithm"),
        QLatin1Literal("functional"),
        QLatin1Literal("math.h"),
        QLatin1Literal("stdio.h"),
        QLatin1Literal("pch.h"),
        QLatin1Literal("commonpch.h"),
        QLatin1Literal("QThread"),
        QLatin1Literal("QMutexLocker"),
        QLatin1Literal("QLatin1String"),
        QLatin1Literal("QMutex"),
    };
    return true;
}
