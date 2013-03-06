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

#include "debianutils.h"
#include "desktopfilewriter.h"
#include "wacSuperWidget.h"
#include "archiver.h"
#include "unzip.h"
#include "wacw3csettingskeys.h"
#include "wbenchmark.h"
#include "wrtsettings.h"
#include "zzip.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QProcess>
#include <QStringList>
#include <QXmlStreamWriter>
#include <sys/stat.h>

#if ENABLE(AEGIS_LOCALSIG)

/*!
 Creates a local signature file to given path. The signature file is name _localsig and contains
 a crypted hash created from all the data going into the .deb package and the security token
 owned by the running process.
  
 @param path path where the _localsig will be created
 */ 

bool DebianUtils::createLocalSignature(QString path){
    QDir current = QDir::current();
    QDir::setCurrent(path);

    //Need to make it here because otherwise the checksum will be different
    QFile debian_binary(path+ "/debian-binary");
    if(!debian_binary.open(QIODevice::WriteOnly)){
        qDebug()<<"Could not create temporary debian_binary"<<Q_FUNC_INFO;
        return false;
    }
    debian_binary.write("2.0\n");
    debian_binary.close();

    QByteArray combined;
    for(int i = 0; i < m_debfiles.size();++i){
        QFile tmp(m_debfiles.at(i));
        if(!tmp.open(QIODevice::ReadOnly)){
            qDebug()<<"could not read"<<m_debfiles.at(i)<<"in"<<Q_FUNC_INFO;
            QDir::setCurrent(current.path());
            return false;
        }
        combined += tmp.readAll();
        tmp.close();
    }

    //FIXME!! This could perhaps be done in memory but as this is not done so often the extra overhead
    //might not be that bad --lörre
    QFile combinedFile("combined");
    if(!combinedFile.open(QIODevice::ReadWrite)){
        qDebug()<<"Could not open file: 'combined' in"<<QDir::current().path()<<Q_FUNC_INFO;
        QDir::setCurrent(current.path());
        return false;
    }
    combinedFile.write(combined);

    QFile _sig(path+"/_localsig");
    if(!_sig.open(QIODevice::WriteOnly)){
        qDebug()<<"Coul not open _localsig file in "<<QDir::current().path()<<Q_FUNC_INFO;
        QDir::setCurrent(current.path());
        return false;
    }
    //Move the file position to beginning because the signing functions
    //will use more low level apis
    _sig.seek(0);
    combinedFile.seek(0);

    if(!createSignature(combinedFile.handle(), _sig.handle(), "LocalSourceOrigin\0")){
        QDir::setCurrent(current.path());
        return false;
    }
    QDir::setCurrent(current.path());
    return true;
}

/*!
 Marks the package to be coming from trusted origin. Such widgets will be packaged
 to debian file that contains _localsig file containing the signature calculated over
 the data of the .deb
 
 @param isTrusted defines if the origin is trusted
*/  

void DebianUtils::installFromTrustedOrigin(bool isTrusted){
    m_addLocalSignature = isTrusted;
}

#endif //AEGIS_LOCALSIG

DebianUtils::DebianUtils(SuperWidget* widget, const
                         QString packageName,
                         const QString& sourcePath,
                         const QString& installationPath,
                         const QString& appId)

    : PackageUtils(widget, packageName, sourcePath, installationPath, appId)
#if ENABLE(AEGIS_LOCALSIG)
    ,m_addLocalSignature(false)
#endif
{
    m_packageFileName = m_packageName + ".deb";
    m_packageFilePath = m_packageRootPath + QDir::separator() + m_packageFileName;
}

bool DebianUtils::createPackage()
{
    if (!preCreatePackage() || !writeControlFile())
        return false;

    WBM("creating Debian package");
    qDebug() << "Creating deb file" << m_packageName << ".deb";
    bool bRet = true;
    
    // Create control file
    WBM_BEGIN("creating control.tar.gz", control);
    Archiver control(m_packageRootPath + "/control.tar.gz", Archiver::TarGz);
    bRet = bRet && control.isValid();
    bRet = bRet && control.addFileFromBuffer(m_controlFile, "./control", 444);
#if defined(Q_OS_MAEMO5)
    if (!m_preinstScript.isEmpty())
        bRet = bRet && control.addFileFromBuffer(m_preinstScript, "./preinst", 444);
#endif
    bRet = bRet && control.addFileFromBuffer(m_postinstScript, "./postinst", 444);
    bRet = bRet && control.addFileFromBuffer(m_postrmScript, "./postrm", 444);
    bRet = bRet && control.addFileFromBuffer(m_prermScript, "./prerm", 444);
    bRet = bRet && control.flush();
    if (!bRet) {
        qCritical() << "FAIL: creating control.tar.gz";
        return false;
    }
    WBM_END(control);

    // Create data file
    WBM_BEGIN("creating data.tar", data);
    Archiver data(m_packageRootPath + "/data.tar", Archiver::Tar);
    bRet = data.isValid();
    if (bRet) {
        QStringList files = QDir(m_packageRootPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        foreach (QString file, files) {
#if defined(Q_OS_MAEMO5)
            if (file == "unzip")
                continue;
#endif
            if (bRet)
                bRet = data.addDirectoryRecursively(m_packageRootPath + "/" + file, file);
        }
        // add some files here
        bRet = bRet && data.flush();
        if (!bRet) {
            qCritical() << "FAIL: creating data.tar";
            return false;
        }
    }
    WBM_END(data);

    // Create debian file
    WBM_BEGIN("creating ar archive", ar);
    Archiver debian(m_packageRootPath + QDir::separator() + m_packageName + ".deb", Archiver::Ar);
    bRet = data.isValid();
    if (bRet) {
        bRet = bRet && debian.addFileFromBuffer("2.0\n", "debian-binary", 444);
        bRet = bRet && debian.addFile(m_packageRootPath + "/control.tar.gz", "control.tar.gz");
        bRet = bRet && debian.addFile(m_packageRootPath + "/data.tar", "data.tar");

#if ENABLE(AEGIS_MANIFEST)
        bRet = bRet && writeManifest(m_packageRootPath);
        bRet = bRet && debian.addFile(m_packageRootPath + "/wrtinstaller.aegis", "_aegis");
#endif

    m_debfiles<<"debian-binary"<< "control.tar.gz"<< "data.tar";
#if ENABLE(AEGIS_MANIFEST)
    m_debfiles<<"wrtinstaller.aegis";
#endif

#if ENABLE(AEGIS_LOCALSIG)
        if (m_addLocalSignature) {
            //Create the signature and aegis manifest file
            if (createLocalSignature(m_packageRootPath)) {
                //If signature creation succeeded add these too. Not preventing
                //the installation if they fail thou. Widget will be installed as
                //non signed
                bRet = bRet && debian.addFile(m_packageRootPath + "/_localsig", "_localsig");
            }else{
                qCritical() <<"FAIL: creating _localsig";
                return false;
            }
        }
#endif
        bRet = bRet && debian.flush();

        if (!bRet) {
            qCritical() << "FAIL: creating debian file";
            return false;
        }
    }
    WBM_END(ar);

    return true;
}

bool DebianUtils::createErrorPackage()
{

    WBM("creating Error Debian package");
    bool bRet = true;
    bool isTrusted(true);

    QDir md;
    md.mkdir(m_packageRootPath);
    m_appVersion = DEFAULT_VERSION;
    
    if( !writeControlFile())
        return false;
   
    //Error package must exit with error 
    QByteArray preinst = "#!/bin/sh\nexit 1\n";
    
    WBM_BEGIN("creating control.tar.gz", control);
    
    Archiver control(m_packageRootPath + "/control.tar.gz", Archiver::TarGz);
    bRet = bRet && control.isValid();
    bRet = bRet && control.addFileFromBuffer(m_controlFile, "./control", 444);
    bRet = bRet && control.addFileFromBuffer(preinst, "./preinst", 444);

    bRet = bRet && control.flush();
    if (!bRet) {
        qCritical() << "FAIL: creating control.tar.gz";
        return false;
    }
    WBM_END(control);

    // Create debian file
    WBM_BEGIN("creating ar archive", ar);
    Archiver debian(m_packageRootPath + QDir::separator() + m_packageName + ".deb", Archiver::Ar);
    //bRet = data.isValid();
    if (bRet) {
        bRet = bRet && debian.addFileFromBuffer("2.0\n", "debian-binary", 444);
        bRet = bRet && debian.addFile(m_packageRootPath + "/control.tar.gz", "control.tar.gz");
        
    //Add only needed deb files for creating local sig
    m_debfiles<<"debian-binary"<< "control.tar.gz";
#if ENABLE(AEGIS_LOCALSIG)
        if (isTrusted) {
            //Create the signature and aegis manifest file
            if (createLocalSignature(m_packageRootPath)) {
                //If signature creation succeeded add these too. Not preventing
                //the installation if they fail thou. Widget will be installed as
                //non signed
                bRet = bRet && debian.addFile(m_packageRootPath + "/_localsig", "_localsig");
            }else{
                qCritical() <<"FAIL: creating _localsig";
                return false;
            }
        }
#endif
        bRet = bRet && debian.flush();

        if (!bRet) {
            qCritical() << "FAIL: creating debian file";
            return false;
        }
    }
    WBM_END(ar);
    return true;
}



#if ENABLE(AEGIS_MANIFEST)
bool DebianUtils::writeManifest(QString packageRootPath)
{
    
    QString aegisFileName = "wrtinstaller.aegis"; 
    QFile aegis(QString("%1/%2").arg(packageRootPath).arg(aegisFileName));
    if (aegis.open(QIODevice::WriteOnly)) {
        QTextStream aegisStream(&aegis);
        aegisStream<<"<aegis>\n<request context=\"INSTALL\">\n";
        aegisStream<<"<credential name=\"CAP::chown\" />\n";
        aegisStream<<"<credential name=\"CAP::setgid\" />\n";            
        aegisStream<<"<credential name=\"CAP::setuid\" />\n";                    
        aegisStream<<"<credential name=\"CAP::dac_override\"/>\n";
        aegisStream<<"<credentail name=\"GRP::admin\" />\n";
        aegisStream<<"</request>\n</aegis>\n";
        aegis.close();

        return true;
    }
    qCritical() <<"FAIL: Could not create "<<aegisFileName<<".aegis file";
    return false;
}

#endif


bool DebianUtils::writeControlFile()
{
    QDir dir;
    if (!dir.mkpath(m_packageRootPath))
        return false;

    DebianPackageMetadata meta;
    meta.packageName = m_packageName;
    meta.appDescription = m_appDescription;
    meta.appName = m_appName;
    meta.appVersion = m_appVersion;
    meta.authorName = m_authorName;
    if (!m_authorEmail.isEmpty())
    {
         meta.authorName += "<"+ m_authorEmail+ ">";
    }
    meta.installedSize = m_installedSize;

    m_controlFile.append(generateControlFile( meta));

     return true;
}

QString DebianUtils::generateValidDebianVersion(const QString& originalVersion)
{
    QString validVersion = originalVersion;
    validVersion.remove(" ");
    if (!isValidDebianVersion(validVersion)) {
        // If supplied originalVersion string would fail in debian packaging,
        // we replace it with default version string + original version where illegal chars have been removed.
        validVersion = convertToAlphaNumeric(originalVersion);
    }

    return validVersion;
}

QString DebianUtils::convertToAlphaNumeric(const QString& original)
{
    QString converted = original;
    for (int i = 0; i < converted.size(); i++) {
        QChar c = converted.at(i);
        if (!c.isLetterOrNumber())
            converted.replace(i, 1, "_");
    }

    return converted;
}

bool DebianUtils::isValidDebianVersion(const QString& version)
{
    // Version must start with digit
    if (version.size() > 0 && !version.at(0).isDigit())
        return false;

    // If there is hyphen in the version string, debian_revision must exist
    if (version.contains("-") && !hasValidDebianRevision(version))
        return false;

    // If there is colon in the version string, epoch must exist
    if (version.contains(":") && !hasValidDebianEpoch(version))
        return false;

    // Check upstream_veersion.
    if (!hasValidDebianUpstreamVersion(version))
        return false;

    return true;
}

bool DebianUtils::hasValidDebianEpoch(const QString& version)
{
    bool valid = false;

    // If there is hyphen in the version string, debian_revision must exist
    int separatorIndex = version.indexOf(":");
    if ((separatorIndex > 0) && (separatorIndex < version.size())) {
        QString epoch = version.left(separatorIndex);
        epoch.toInt(&valid);
    }

    return valid;
}

bool DebianUtils::hasValidDebianUpstreamVersion(const QString& version)
{
    for (int i = 0; i < version.size(); i++) {
        QChar c = version.at(i);
        if (!c.isLetterOrNumber() && (c != '.') && (c != '+') && (c != '-') && (c != ':') && (c != '~'))
            return false;
    }
    return true;
}

bool DebianUtils::hasValidDebianRevision(const QString& version)
{
    bool valid = false;

    // If there is hyphen in the version string, debian_revision must exist
    int separatorIndex = version.lastIndexOf("-");
    if ((separatorIndex > 0) && (separatorIndex < version.size())) {
        QString revision = version.mid(separatorIndex);
        revision.toInt(&valid);
    }

    return valid;
}

QString DebianUtils::generateControlFile(const DebianPackageMetadata& packageMetadata)
{
    QString controlFile ("Package: " + packageMetadata.packageName + "\n"
                         "Source: " + packageMetadata.packageName + "\n"
                         "Priority: optional\n"
                         "Section: user/desktop\n"
                         "Maintainer: " + packageMetadata.authorName + "\n"
                         "Version: " + generateValidDebianVersion(packageMetadata.appVersion) + "\n"
                         "Architecture: all\n"
                         "Installed-Size: " + QString::number(packageMetadata.installedSize >> 10) + "\n"
                         "Maemo-Display-Name: " + packageMetadata.appName + "\n"
#ifdef ENABLE_WIDGET_DEPS
                         "Depends: qtwrt\n"
#endif
    // TODO: This is likely not working with more that 70 characters.
                         "Description: " + packageMetadata.appName + " (web application)" + "\n " + packageMetadata.appDescription.simplified() + "\n");
    return controlFile;

}
