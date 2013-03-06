/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This file is part of Qt Web Runtime.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "widgetinstaller.h"
#include "private/widgetinstaller_p.h"
#include "private/WidgetUtilsLogs.h"
#include "wacSuperWidget.h"
#include "wacstorage.h"

#ifdef Q_OS_SYMBIAN
#include <bautils.h>
#endif

WidgetInstaller::WidgetInstaller(SuperWidget * webWidget, WAC::Storage * storage) {
    d = new WidgetInstallerPrivate(webWidget, storage);
}

WidgetInstaller::~WidgetInstaller() {
    delete d;
}

bool WidgetInstaller::install(const QDir& source, const QDir& target, const QString& /*appId*/) {
    if (source != target) {
        return d->renameDir(source, target);
    }
    return true;
}

bool WidgetInstaller::update(const QDir& source, const QDir& target, const QString& appId) {
    Q_UNUSED(appId)
    return d->cpDir(source, target, true);
}

bool WidgetInstaller::uninstall(const QString& applicationPath, const QString& appId) {
    Q_UNUSED(appId)
    bool status = false;
    QDir currentDir(applicationPath);
    status = SuperWidget::rmDir(applicationPath);
    currentDir.rmdir(applicationPath);
    return status;
}

WidgetInstallerPrivate::WidgetInstallerPrivate(SuperWidget * webWidget,
        WAC::Storage * storage) :
    m_webWidget(webWidget), m_storage(storage), m_firstCopy(true) {
}

WidgetInstallerPrivate::~WidgetInstallerPrivate(){
}



//
// function to copy all files in a directory
// parameters:
//     srcDir    path to source directory
//     dstDir    path to destination directory
//     force     if true force copy
//     secure    if true add to secure storage
//
// Used to copy entire directory
//
bool WidgetInstallerPrivate::cpDir(const QDir &srcDir,const QDir &dstDir,const bool force, 
        const bool secure)
{
    QString srcAbsPathOrg = srcDir.absolutePath();
    QString srcAbsPath =
        !srcAbsPathOrg.endsWith(QDir::separator())?
        srcAbsPathOrg+QDir::separator() : srcAbsPathOrg;
    QString dstAbsPathOrg = dstDir.absolutePath();
    QString dstAbsPath =
        !dstAbsPathOrg.endsWith(QDir::separator())?
        dstAbsPathOrg+QDir::separator() : dstAbsPathOrg;

    // Create destination folder first
    // Do check existance first, b/c it is faster than mkdir of existing folder
    if (!dstDir.exists()) {
        if (!dstDir.mkdir(dstAbsPathOrg)) {
            return false;
        }
    }

    // Get all files and folders in the current folder
    QFileInfoList allEntries =
        srcDir.entryInfoList(QDir::AllEntries|QDir::Hidden|QDir::NoDotAndDotDot);
    QFileInfoList::iterator it;

    // for each file and folder...
    for (it = allEntries.begin(); it != allEntries.end(); it++) {
        if (it->isFile()) {
            // copy/rename files
            QString dstFileName = dstAbsPath + it->fileName();

            if (!QFile::exists(dstFileName) || force) {
                QFile dstFile(dstFileName);
                dstFile.remove();
                QDir p;
                if (!p.rename(srcAbsPath + it->fileName(), dstFileName)) {
                    LOG("error: cp failed");
                    return false;
                }
#ifdef CWRT_WIDGET_FILES_IN_SECURE_STORAGE
                else if (secure) {
                    m_storage->add(dstFileName);
                }
#else
                (void)secure;
#endif
            }
        }
        else {
            // recursevly move folder content
            QDir newSrc(srcAbsPath + it->fileName() );
            QDir newDst(dstAbsPath + it->fileName() );
            if (!cpDir(newSrc,newDst,force,secure)) {
                return false;
            }
        }
    }
    return true;
}

// function to rename directory
// parameters:
//     srcDir    path to source directory
//     dstDir    path to destination directory
//
// Used to rename entire directory
//
bool WidgetInstallerPrivate::renameDir(const QDir& srcDir, const QDir& dstDir) {
    QDir widgetPath(m_webWidget->widgetInstallPath());
    QString srcPath = srcDir.absolutePath();
    QString dstPath = dstDir.absolutePath();

    if (dstDir.exists()) {
        SuperWidget::rmDir(dstPath);
        if (!dstDir.rmdir(dstPath))
            return false;
    }
    // make sure the full path exists with the exception of the directory so the rename will work
    else if (!widgetPath.mkpath(dstPath) || !widgetPath.rmdir(dstPath))
            return false;

#ifdef Q_OS_SYMBIAN
    //If the srcPath and dstPath are not on the same drive then files should be copied from srcPath to dstPath.
    //The unzip path of widget can be e: drive while restoring widgets from backup-restore process.
    //    if (!srcPath.startsWith("c:",Qt::CaseInsensitive) ) {
    if (srcPath.left(1).compare(dstPath.left(1), Qt::CaseInsensitive)) {
        LOG("Root directory do not match");
        srcPath = srcPath + '\\';
        srcPath = QDir::toNativeSeparators(srcPath);
        dstPath = QDir::toNativeSeparators(dstPath);
        TRAPD( err, moveDirL(srcPath, dstPath) );
        if (err != KErrNone)
            return false;
        else
            return true;
    }
#endif
    return widgetPath.rename(srcPath, dstPath);
}

void WidgetInstaller::setInstallationAttributes(const WAC::InstallationAttributes& attributes) {
    Q_UNUSED(attributes);
    // TODO : It is able to be implemented in future for other than MAEMO
}


#ifdef Q_OS_SYMBIAN
// function to copy directory
// parameters:
//     srcPath    path to source directory
//     dstPath    path to destination directory
//
// Used to move entire directory
//
void WidgetInstallerPrivate::moveDirL(const QString& srcPath,const QString& dstPath)
{
    LOG("WidgetInstallerPrivate::moveDirL"<<srcPath<<dstPath);

    RFs aFs;
    User::LeaveIfError( aFs.Connect() );
    CFileMan* fileManager = CFileMan::NewL( aFs );
    CleanupStack::PushL( fileManager );

    TPtrC16 srcDomainPath(reinterpret_cast<const TUint16*>(srcPath.utf16()));
    TPtrC16 desDomainPath(reinterpret_cast<const TUint16*>(dstPath.utf16()));

    TInt err = fileManager->Move(srcDomainPath, desDomainPath, CFileMan::ERecurse | 
            CFileMan::EOverWrite );
    LOG("WidgetInstallerPrivate::moveDirL() using fileManager->Move()"<<err);

    User::LeaveIfError( err );
    CleanupStack::PopAndDestroy();  // fileMananger

    aFs.Close();
}
#endif
