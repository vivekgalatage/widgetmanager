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

#ifndef DEBIANUTILS_H
#define DEBIANUTILS_H


#include "packageutils.h"
#include "wrtglobal.h"
#define OVISTORE_WEBAPPS_PACKAGE_NAME_PREFIX "ovi-webapp-"

class SuperWidget;
class WidgetProperties;

typedef struct  {
    QString packageName;
    QString authorName;
    QString appName;
    QString appVersion;
    QString appDescription;
    unsigned long installedSize;
} DebianPackageMetadata;


/*!
  \class DebianUtils
  \brief A class used to create Debian packages for widgets.
  */
class DebianUtils : public PackageUtils
{
    Q_DISABLE_COPY(DebianUtils)

public:
    /*!
     Constructs an object that can be used to create a Debian package named \a packageName. The unzipped widget files locate at \a sourcePath, and they are
     supposed to be installed to \a installationPath. The attributes of widgets can be seen from \a widget, while the unique ID is given by \a appId.
     */
    explicit DebianUtils(SuperWidget* widget, const QString packageName, const QString& sourcePath, const QString& installationPath, const QString& appId);

    static QString generateControlFile(const DebianPackageMetadata& packageMetadata);
    /*!
      Create the package.
     */
    virtual bool createPackage();
    
    /*!
      Create the error package.
     */
    virtual bool createErrorPackage();

    /*!
     Mark the widget as a trusted origin widget. This will
     generate a local signature file into the .deb
     */
#if ENABLE(AEGIS_LOCALSIG)
    void installFromTrustedOrigin(bool isTrustedOrigin);
#endif

private:
    bool writeControlFile();
#if ENABLE(AEGIS_LOCALSIG)
    bool createLocalSignature(QString path);
    bool m_addLocalSignature;
#endif
   
#if ENABLE(AEGIS_MANIFEST)
   bool writeManifest(QString rootPath);
#endif   
   
   
    // to generate a valid Debian version number from the version given in config.xml
    static QString generateValidDebianVersion(const QString& originalVersion);
    static bool isValidDebianVersion(const QString& version);
    static bool hasValidDebianEpoch(const QString& version);
    static bool hasValidDebianUpstreamVersion(const QString& version);
    static bool hasValidDebianRevision(const QString& version);
    static QString convertToAlphaNumeric(const QString& original);

    QByteArray m_controlFile;
    //Member that holds needed deb files to be added for creating local sig
    QStringList m_debfiles;
};

#endif // DEBIANUTILS_H
