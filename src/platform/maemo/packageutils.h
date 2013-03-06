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

#ifndef PACKAGEUTILS_H
#define PACKAGEUTILS_H

#include <QStringList>
#include "wrtglobal.h"
class SuperWidget;
class WidgetProperties;

const QString DEFAULT_VERSION("0.0.1");
const QString DEFAULT_DESCRIPTION("A web application");
const QString DEFAULT_VIEW_MODE("windowed");
const QString DEFAULT_ICON("icon-l-default-application");
const QString DEFAULT_VENDOR("NOKIA");
const QString DEFAULT_OS("linux");
const QString DEFAULT_ARCHITECTURE("i386");
const QString DEFAULT_GROUP("widgets");

#if defined(Q_OS_MAEMO6)
const QStringList SUPPORTED_HOMESCREEN_VIEW_MODES;
const QStringList SUPPORTED_APPLICATION_VIEW_MODES(QStringList() << "windowed" << "fullscreen" << "maximized");
#elif defined(Q_OS_MAEMO5)
const QStringList SUPPORTED_HOMESCREEN_VIEW_MODES(QStringList() << "minimized");
const QStringList SUPPORTED_APPLICATION_VIEW_MODES(QStringList() << "windowed" << "fullscreen");
const QString ICON_SYMBOLIC_LINK_DIR("/usr/share/icons/hicolor/scalable/hildon/");
#else
const QStringList SUPPORTED_HOMESCREEN_VIEW_MODES(QStringList() << "floating" << "minimized");
const QStringList SUPPORTED_APPLICATION_VIEW_MODES(QStringList() << "windowed" << "fullscreen" << "maximized");
// TODO: where to put the icons for reference Linux and MeeGo?
const QString ICON_SYMBOLIC_LINK_DIR("");
#endif

const QString BACKUP_CONFIG_FILE_DIR("/usr/share/backup-framework/applications");
const QString BACKUP_SCRIPTS_DIR("/usr/share/wrt/backup");


/*!
  \class PackageUtils
  \brief The base class used to create packages for widgets.
  */
class PackageUtils
{
    Q_DISABLE_COPY(PackageUtils)

public:
    /*!
     Constructs an object that can be used to create a Debian package named \a packageName. The unzipped widget files locate at \a sourcePath, and they are
     supposed to be installed to \a installationPath. The attributes of widgets can be seen from \a widget, while the unique ID is given by \a appId.
     */
    explicit PackageUtils(SuperWidget* widget, const QString packageName, const QString& sourcePath, const QString& installationPath, const QString& appId);

    /*!
      Destroys the object.
    */
    virtual ~PackageUtils();

    /*!
      Create the package.
     */
    virtual bool createPackage() = 0;

    /*!
      Create the error package in case of installation error and --no-install option.
     */
    virtual bool createErrorPackage() = 0;

    /*!
     The full path of the generated package.
     */
    virtual QString packageFilePath() const;

    /*!
     The file name of the generated package.
     */
    virtual QString packageFileName() const;

protected:
    bool preCreatePackage();

    bool setAttributes();
    bool prepareWidget();
    bool writeDesktopFile();
    bool writeInstallationScripts();
    bool writeRemovalScripts();

#if defined(Q_OS_MAEMO6)
    bool writeBackupConfigFile();
    bool writeBackupRestoreScripts();
    bool writeBackupScript();
    bool writeRestoreScript();
    bool writeSecsessionFile();
#endif
   
#if ENABLE(AEGIS_LOCALSIG)
   static bool createSignature(int ih, int oh, const char* resource_id);
   static int computeDigest(int ih, unsigned char* digest, ssize_t maxdigestlen);
#endif   

    SuperWidget* m_widget;
    WidgetProperties* m_properties;
    QString m_packageName;
    QString m_sourcePath;
    QString m_installationPath;
    QString m_appId;
    QString m_packageRootPath;
    QString m_packageFilePath;
    QString m_packageFileName;
    QString m_nativeId;
    QString m_appRunner;
    QString m_appName;
    QString m_appVersion;
    QString m_appDescription;
    QString m_authorEmail;
    QString m_authorName;
    QString m_iconName;
    QString m_iconPath;
#if !defined(Q_OS_MAEMO6)
    QString m_iconSymbolicLinkPath;
#endif
    QStringList m_applicationViewModes;
    QStringList m_homescreenViewModes;
#if defined(Q_OS_MAEMO5)
    QString m_license;
    QByteArray m_preinstScript;
#endif
    QByteArray m_postinstScript;
    QByteArray m_postrmScript;
    QByteArray m_prermScript;

    unsigned long m_installedSize;
};

#endif // PACKAGEUTILS_H
