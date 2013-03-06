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

#include "packageutils.h"
#include "desktopfilewriter.h"
#include "wacSuperWidget.h"
#include "wacwidgetmanagerconstants.h"
#include "wacw3csettingskeys.h"
#include "wrtsettings.h"
#include "zzip.h"

#include <QDesktopServices>
#include <QFile>


PackageUtils::PackageUtils(SuperWidget* widget, const QString packageName, const QString& sourcePath, const QString &installationPath, const QString &appId)
    : m_widget(widget), m_packageName(packageName), m_sourcePath(sourcePath), m_installationPath(installationPath), m_appId(appId)
{
    m_packageRootPath = m_sourcePath.left(m_sourcePath.lastIndexOf(QDir::separator()));
    m_installedSize = 0;
}

PackageUtils::~PackageUtils()
{
    // do nothing
}

QString PackageUtils::packageFilePath() const
{
    return m_packageFilePath;
}

QString PackageUtils::packageFileName() const
{
    return m_packageFileName;
}

bool PackageUtils::preCreatePackage()
{
    bool result = setAttributes() && prepareWidget() && writeDesktopFile() && writeInstallationScripts() && writeRemovalScripts();

#if defined(Q_OS_MAEMO6)
    result = result && writeBackupConfigFile() && writeBackupRestoreScripts() && writeSecsessionFile();
#endif

    return result;
}

bool PackageUtils::setAttributes()
{
    m_properties = m_widget->getProperties();

    // application runner
    if (m_properties->type() == WIDGET_PACKAGE_FORMAT_OVIAPP)
        m_appRunner = WAC::WrtSettings::createWrtSettings()->valueAsString("OviRunner") + " " + m_appId;

    // name
    m_appName = m_widget->value(W3CSettingsKey::WIDGET_NAME);
    if (m_appName.isEmpty())
        m_appName = QFileInfo(m_widget->widgetBundlePath()).completeBaseName();

    // version
    m_appVersion = m_widget->value(W3CSettingsKey::WIDGET_VERSION);
    if (m_appVersion.isEmpty())
       m_appVersion = DEFAULT_VERSION;

    // description
    m_appDescription = m_widget->value(W3CSettingsKey::WIDGET_DESCRIPTION);
    if (m_appDescription.isEmpty())
        m_appDescription = DEFAULT_DESCRIPTION;

    // author
    m_authorEmail = m_widget->value(W3CSettingsKey::WIDGET_AUTHOR, "email");
    m_authorName = m_widget->value(W3CSettingsKey::WIDGET_AUTHOR);

    // license
#if defined(Q_OS_MAEMO5)
    m_license = m_widget->value(W3CSettingsKey::WIDGET_LICENSE);
#endif

    // view modes
    QStringList viewModes = m_widget->value(W3CSettingsKey::WIDGET_VIEWMODES).split(" ");
    viewModes.removeDuplicates();

    foreach (QString viewMode, viewModes) {
        if (!SUPPORTED_HOMESCREEN_VIEW_MODES.contains(viewMode) && !SUPPORTED_APPLICATION_VIEW_MODES.contains(viewMode))
            viewModes.removeOne(viewMode);
    }
    if (viewModes.isEmpty())
        viewModes << DEFAULT_VIEW_MODE;

    foreach (QString viewMode, viewModes) {
        if (SUPPORTED_HOMESCREEN_VIEW_MODES.contains(viewMode)) {
            m_homescreenViewModes << viewMode;
        } else {
            m_applicationViewModes << viewMode;
        }
    }

    // icon
    m_iconPath = m_properties->iconPath();

    if (m_iconPath.right(4) == ".png" && !m_iconPath.endsWith(":/resource/default_widget_icon.png")) {
        m_iconName = m_appId;
#if !defined(Q_OS_MAEMO6)
        m_iconSymbolicLinkPath = ICON_SYMBOLIC_LINK_DIR + m_iconName + ".png";
#endif
    } else {
        m_iconName = DEFAULT_ICON;
    }

    return true;
}

bool PackageUtils::prepareWidget()
{
    QString widgetInstallPath(m_packageRootPath + m_installationPath);
    while (widgetInstallPath.endsWith(QDir::separator()))
        widgetInstallPath.chop(1);

    QDir dir;
    if (!dir.mkpath(widgetInstallPath.left(widgetInstallPath.lastIndexOf(QDir::separator()))))
        return false;

#if defined(Q_OS_MAEMO5)
    // dpkg on M5 doesn't support long path name, thus unzip the .wgt in postinst script
    m_installedSize += QFileInfo(m_widget->widgetBundlePath()).size();
    if (!dir.mkpath(widgetInstallPath)
        || !QFile::copy(m_widget->widgetBundlePath(), widgetInstallPath + QDir::separator() + QFileInfo(m_widget->widgetBundlePath()).fileName()))
        return false;
#else
    // move the unzipped widget to the temp folder
    m_installedSize += Zzip::uncompressedSize(m_widget->widgetBundlePath());
    if (!dir.rename(m_sourcePath, widgetInstallPath + QDir::separator()))
        return false;

    QDir baseRootDir(m_packageRootPath);
    baseRootDir.mkpath("etc/secure/s/" + m_packageName);
    baseRootDir.mkpath(m_properties->resourcePath());
#endif

#ifdef Q_OS_MAEMO6
    // keep the original .wgt for backup
    m_installedSize += QFileInfo(m_widget->widgetBundlePath()).size();
    QString wgtFilePath(m_packageRootPath + BACKUP_SCRIPTS_DIR + "/install/");
    if (!dir.mkpath(wgtFilePath) || !QFile::copy(m_widget->widgetBundlePath(), wgtFilePath + m_appId + ".wgt"))
        return false;
#endif

    return true;
}

bool PackageUtils::writeDesktopFile()
{
    if (!m_properties->isSharedLibrary() || m_properties->isSharedLibraryWidget()) {
        DesktopFileWriter desktopFileWriter(m_packageRootPath, m_installationPath, m_appId);
#if defined(Q_OS_MAEMO6)
        desktopFileWriter.setAppRunner(m_appRunner);
#endif
        desktopFileWriter.setAppName(m_appName);
#if defined(Q_OS_MAEMO6)
        if (m_iconName.compare(DEFAULT_ICON) != 0)
        {
            //if there is widget icon - set absolute path into desktop file
            desktopFileWriter.setAppIcon(m_iconPath);
        }
        else
        {
            desktopFileWriter.setAppIcon(m_iconName);
        }
#else
        desktopFileWriter.setAppIcon(m_iconName);
#endif
        desktopFileWriter.setHidden(m_properties->isHiddenWidget());
        desktopFileWriter.setPackageName(m_packageName);
        if (!m_applicationViewModes.isEmpty())
            desktopFileWriter.setApplicationViewMode(m_applicationViewModes.first());
        if (!m_homescreenViewModes.isEmpty())
            desktopFileWriter.setHomescreenViewMode(m_homescreenViewModes.first());

        if (!desktopFileWriter.write(m_widget))
            return false;

        m_nativeId = desktopFileWriter.desktopFilePath();
    }

    return true;
}

bool PackageUtils::writeInstallationScripts()
{
    // preinst script
#ifdef Q_OS_MAEMO5
    // TODO: find a similar way to pop up license on other platform
    if (!m_license.isEmpty()) {
        m_preinstScript.append("#!/bin/sh\n"
                               "cat > /tmp/" + m_packageName + " <<EOF\n"
                               + m_license + "\n"
                               "EOF\n"
                               "maemo-confirm-text /tmp/" + m_packageName + "\n"
                               "retval=$?\n"
                               "rm /tmp/" + m_packageName + "\n"
                               "exit $retval");
    }
#endif

    // postinst script
    m_postinstScript.append("#!/bin/sh\n\n");
#if Q_OS_MAEMO5
    m_postinstScript.append("unzip '" + m_installationPath + QDir::separator() + QFileInfo(m_widget->widgetBundlePath()).fileName() + "' -d " + m_properties->installPath() + "\n");
#endif

#if !defined(Q_OS_MAEMO6)
    if (!m_iconSymbolicLinkPath.isEmpty()) {
        m_postinstScript.append("mkdir -p " + ICON_SYMBOLIC_LINK_DIR + "\n"
                                "if [ ! -f " + m_iconSymbolicLinkPath + " ]; then\n"
                                "ln -s " + m_iconPath + " " + m_iconSymbolicLinkPath + "\n"
                                "fi\n");
    }
#endif

    QString certificateAki = m_properties->certificateAki();
    if (certificateAki.isEmpty())
        certificateAki = "NONE";

#ifdef __ARMEL__
    // XXX: On armel, webappregisterer stores some paths based on HOME variable
    // which is expected to be from regular user. Since the app runs as root, we
    // need to force HOME to be /home/user when calling webappregisterer.
    m_postinstScript.append("`HOME=/home/user exec /usr/bin/webappregisterer " + m_properties->installPath() + " " + m_properties->id() + " " + m_nativeId + " " + certificateAki + " register`\n"
                            "ret_val=$?\n");
#else
    m_postinstScript.append("`exec /usr/bin/webappregisterer " + m_properties->installPath() + " " + m_properties->id() + " " + m_nativeId + " " + certificateAki + " register`\n"
                            "ret_val=$?\n");
#endif

#ifdef Q_OS_MAEMO6
    m_postinstScript.append("if [ -f \"/usr/share/wrt/data/registry/webapp_registry.db\" ] ; then\n"
                            "    mkdir -p \"/var/lib/aegis/ps/.s/S/wrt.registry\"\n"
                            "    cp \"/usr/share/wrt/data/registry/webapp_registry.db\" \"/var/lib/aegis/ps/.s/S/wrt.registry/webapp_registry.db\"\n"
                            "    if [ -f \"/var/lib/aegis/ps/Ss/wrt.registry\" ] ; then\n"
                            "        rm \"/var/lib/aegis/ps/Ss/wrt.registry\"\n"
                            "    fi\n"
                            "    apscli -x -s wrt.registry:Ss -a \"/var/lib/aegis/ps/.s/S/wrt.registry/webapp_registry.db\"\n"
                            "fi\n");
#endif

    m_postinstScript.append("exit $ret_val\n");
    return true;
}

bool PackageUtils::writeRemovalScripts()
{
    // prerm script
    QString widgetService = QString("com.nokia.webwidgetrunner%1wrtwidgetdesktop").arg(m_properties->id());
    QString widgetObject("/wrt/webwidgetrunner");
    QString widgetCloseMessage("local.WRT.Maemo.WebListener.Close");

    m_prermScript.append("#!/bin/sh\n\n");
    m_prermScript.append(QString("dbus-send --session --dest=%1 --type=method_call %2 %3\n").arg(widgetService, widgetObject, widgetCloseMessage));
    m_prermScript.append("exit 0\n");

    // postrm script
    m_postrmScript.append("#!/bin/sh\n\n");
#ifdef Q_OS_MEEGO
    m_postrmScript.append("rm /etc/secure/s/" + m_packageName + "\n");
    m_postrmScript.append("rm -rf " + m_properties->resourcePath() + "\n");
#endif

#ifdef Q_OS_MAEMO5
    // remove unzipped files
    m_postrmScript.append("rm -rf /usr/share/wrt/data/widgets_21D_4C7/" + m_properties->id() + "\n");
#endif

#if !defined(Q_OS_MAEMO6)
    if (!m_iconSymbolicLinkPath.isEmpty())
        m_postrmScript.append("rm -f " + m_iconSymbolicLinkPath + "\n");
#endif

    m_postrmScript.append("exec /usr/bin/webappregisterer " + m_properties->id() + " unregister\n\n"
                          "exit 0\n");

    return true;
}

#if defined(Q_OS_MAEMO6)
bool PackageUtils::writeBackupConfigFile()
{
    QString backupFileDirPath = m_packageRootPath + BACKUP_CONFIG_FILE_DIR;

    QDir backupFileDir(backupFileDirPath);
    if (!backupFileDir.exists())
        backupFileDir.mkpath(backupFileDirPath);

    QString backupFilePath = QString("%1/widget_%2.conf").arg(backupFileDirPath, m_appId);
    QFile backupFile(backupFilePath);
    if (!backupFile.open(QIODevice::WriteOnly))
        return false;

    QString widgetService = QString("com.nokia.webwidgetrunner%1wrtwidgetdesktop").arg(m_appId);

    QXmlStreamWriter backupXmlStream(&backupFile);
    backupXmlStream.setAutoFormatting(true);
    backupXmlStream.writeStartElement("backup-configuration");

    backupXmlStream.writeTextElement("application-type", "nokia");
    backupXmlStream.writeTextElement("application-name", m_appName);

    QString backupScriptsDirPath = BACKUP_SCRIPTS_DIR + "/" + m_appId;
    backupXmlStream.writeTextElement("backup-method", "permanent-backup-files,backup-scripts");
    backupXmlStream.writeStartElement("backup-scripts");
    backupXmlStream.writeTextElement("backup-script-name", QString("%1/%2-backup").arg(backupScriptsDirPath).arg(m_appId));
    backupXmlStream.writeTextElement("restore-script-name", QString("%1/%2-restore").arg(backupScriptsDirPath).arg(m_appId));
    backupXmlStream.writeEndElement(); // backup-scripts

    backupXmlStream.writeStartElement("backup-dbus");
    backupXmlStream.writeTextElement("prestart", "no");
    backupXmlStream.writeStartElement("dbus-service");
    backupXmlStream.writeAttribute("object", "/wrt/webwidgetrunner/backup");
    backupXmlStream.writeCharacters(widgetService);
    backupXmlStream.writeEndElement(); // dbus-service
    backupXmlStream.writeEndElement(); // backup-dbus

    backupXmlStream.writeStartElement("locations");

    backupXmlStream.writeStartElement("location");
    backupXmlStream.writeAttribute("type", "permanent-backup-file");
    backupXmlStream.writeAttribute("category", "settings"); //should be the same as in wrt.conf

    // Removing user-specific home path from resourcePath and prepending
    // $HOME env variable, so it can be flexible enough for backup app.
    QString resourcePath = QDir::cleanPath(m_properties->resourcePath());
    QString homePath = QDir::homePath();
    if (resourcePath.startsWith(homePath)) {
        resourcePath.remove(homePath);
        resourcePath.prepend(QString("$HOME"));
    }
    backupXmlStream.writeCharacters(resourcePath); // widget data path
    backupXmlStream.writeEndElement();

    backupXmlStream.writeStartElement("location");
    backupXmlStream.writeAttribute("type", "file");
    backupXmlStream.writeAttribute("category", "settings");
    QString privateEncryptedStoragePath = QString("/var/tmp/%1_Pe.tar.gz").arg(m_appId);
    backupXmlStream.writeCharacters(privateEncryptedStoragePath);
    backupXmlStream.writeEndElement();

    backupXmlStream.writeEndElement(); // locations

    backupXmlStream.writeEndElement(); // backup-configuration
    backupXmlStream.writeEndDocument();

    backupFile.flush();

    return true;
}

bool PackageUtils::writeBackupRestoreScripts()
{
    return writeBackupScript() && writeRestoreScript();
}

bool PackageUtils::writeBackupScript()
{
    QString backupScriptsDirPath = m_packageRootPath + BACKUP_SCRIPTS_DIR + "/" + m_appId;
    QDir backupScriptsDir(backupScriptsDirPath);
    if (!backupScriptsDir.exists())
        backupScriptsDir.mkpath(backupScriptsDirPath);

    QString backupScriptFilePath = QString("%1/%2-backup").arg(backupScriptsDirPath, m_appId);
    QFile backupScriptFile(backupScriptFilePath);
    if (!backupScriptFile.open(QIODevice::WriteOnly))
        return false;

    QString backupScript;
    backupScript.append("#!/bin/sh\n\n");
    backupScript.append(QString("PE_FILES=`apscli -s %1:Pe | cut -d ':' -f 0`\n").arg(m_appId));
    backupScript.append(QString("PE_FILES=\"$PE_FILES \"`apscli -s %1:Pe -lu`\n").arg(m_appId));
    backupScript.append("PE_VALID_FILES=\n");
    backupScript.append("for f in $PE_FILES\n");
    backupScript.append("do\n");
    backupScript.append("\tif [ -f \"$f\" ]; then\n");
    backupScript.append("\t\tPE_VALID_FILES=$f\" \"$PE_VALID_FILES\n");
    backupScript.append("\tfi\n");
    backupScript.append("done\n\n");
    backupScript.append("if [ -z \"$PE_VALID_FILES\" ]; then\n");
    backupScript.append("\texit 0\n");
    backupScript.append("fi\n\n");
    backupScript.append(QString("tar cfz /var/tmp/%1_Pe.tar.gz $PE_VALID_FILES\n\n").arg(m_appId));
    backupScript.append("exit 0\n");

    backupScriptFile.write(backupScript.toUtf8());
    backupScriptFile.flush();
    backupScriptFile.close();

    backupScriptFile.setPermissions(backupScriptFile.permissions()
            | QFile::ExeUser | QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther);

    return true;
}

bool PackageUtils::writeRestoreScript()
{
    QString backupScriptsDirPath = m_packageRootPath + BACKUP_SCRIPTS_DIR + "/" + m_appId;
    QDir backupScriptsDir(backupScriptsDirPath);
    if (!backupScriptsDir.exists())
        backupScriptsDir.mkpath(backupScriptsDirPath);

    QString restoreScriptFilePath = QString("%1/%2-restore").arg(backupScriptsDirPath, m_appId);
    QFile restoreScriptFile(restoreScriptFilePath);
    if (!restoreScriptFile.open(QIODevice::WriteOnly))
        return false;

    QString restoreScript;
    restoreScript.append("#!/bin/sh\n\n");
    restoreScript.append(QString("if [ ! -f \"/var/tmp/%1_Pe.tar.gz\" ]; then\n").arg(m_appId));
    restoreScript.append("\texit 0\n");
    restoreScript.append("fi\n\n");
    restoreScript.append(QString("tar xfz /var/tmp/%1_Pe.tar.gz -C /\n").arg(m_appId));
    restoreScript.append(QString("rm -f /var/tmp/%1_Pe.tar.gz\n\n").arg(m_appId));
    restoreScript.append("exit 0\n");

    restoreScriptFile.write(restoreScript.toUtf8());
    restoreScriptFile.flush();
    restoreScriptFile.close();
    restoreScriptFile.setPermissions(restoreScriptFile.permissions()
            | QFile::ExeUser | QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther);

    return true;
}

bool PackageUtils::writeSecsessionFile()
{
    QFileInfo secSessionFileInfo(m_packageRootPath + m_properties->secureSessionPath());

    QDir dir = secSessionFileInfo.absoluteDir();
    if (!dir.mkpath(dir.path()))
        return false;

    QFile secSessionFile(secSessionFileInfo.absoluteFilePath());
    if (!secSessionFile.open(QIODevice::ReadWrite | QIODevice::Truncate)
        || -1 == secSessionFile.write(m_properties->secureSessionString().toUtf8()))
        return false;
    secSessionFile.close();

    return true;
}
#endif

#if ENABLE(AEGIS_LOCALSIG)
#include <errno.h>
#include <QThread>
#include <grp.h>
#include <aegis_common.h>
#include <aegis_crypto.h>
#include <openssl/evp.h>
#define DIGESTTYP   EVP_sha1
#define DIGESTLEN   EVP_MD_size(DIGESTTYP())
#define DIGESTNAME "SHA1"
#define EVPOK 1
/*!
 computes hash over the given data

 @param ih filehandle to data from which the hash is to be calculated
 @param digest array into which the hash is stored. The array must be allocated
 and must contains maxdigestlen bytes
 @param maxdigestlen the length of digest to be calculated. Must equal to the size
 of allocated bytes in digest array.
  
 @return computed hash length
 */

//Copied from aegis-crypto0 bin/accli.c
int PackageUtils::computeDigest(int ih, unsigned char* digest, ssize_t maxdigestlen)
{
    EVP_MD_CTX mdctx;
    unsigned int mdlen;
    unsigned char data[512];
    int rc;
    ssize_t len;

    if (maxdigestlen < DIGESTLEN)
        return(-EINVAL);

    rc = EVP_DigestInit(&mdctx, DIGESTTYP());
    if (EVPOK != rc) {
        qDebug()<<"EVP_DigestInit returns "<< rc;
        return 0;
    }

    while (0 < (len = read(ih, data, sizeof(data)))) {
        rc = EVP_DigestUpdate(&mdctx, data, len);
        if (EVPOK != rc) {
            qDebug()<<"EVP_DigestUpdate returns:"<<rc<<Q_FUNC_INFO;
            return 0;
        }
        if (len < sizeof(data))
           break;
    }

    rc = EVP_DigestFinal(&mdctx, digest, &mdlen);
    if (rc != EVPOK) {
        qDebug()<<"EVP_DigestUpdate returns:"<<rc<<Q_FUNC_INFO;
        return(0);
    }

    EVP_MD_CTX_cleanup(&mdctx);

    return mdlen;
}
/*!

 Create signature for the given data using the resource id and write the signature
 to given file.
 
 @param ih inputfilehandle to file containing the data from which the data is to be read. The file must
 exists and must be opened for reading
 @param oh outputfilehandle to file into which the signature is to be written. The file must exists
 and must be opened for writing
 @param resource_id null terminated array containing the resource_id that will be appended to the
 signature. Executing application must have this resource token
 
 @return true if the generation and writing of signature succeeded. Otherwise false.
 */ 

//Copied from aegis-crypto0 bin/accli.c
bool PackageUtils::createSignature(int ih, int oh, const char* resource_id)
{
    int mdlen;
    unsigned char digest[DIGESTLEN];
    struct aegis_signature_t signature;
    aegis_crypto_result res;

    mdlen = computeDigest(ih, digest, sizeof(digest));
    if (0 == mdlen) {
        qDebug()<<"Could not calculate digest in "<<Q_FUNC_INFO;
        return false;
    }

    res = aegis_crypto_sign(digest, mdlen, resource_id, &signature);
    if (aegis_crypto_ok != res) {
        qDebug()<<"Failed to sign the signature in "<<Q_FUNC_INFO<<aegis_crypto_last_error_str();
        return false;
    }

    char* str_sig = NULL;
    if (0 < aegis_crypto_signature_to_string(&signature,
                                             aegis_as_hexstring,
                                            resource_id,
                                             &str_sig)){
        //signature creation successfull. Write it to file
        ssize_t len = strlen(str_sig);
        ssize_t written = write(oh, str_sig, len);
        if (written < len) {
            qDebug()<<"Can not write outputfile"<<Q_FUNC_INFO;
        }
    }
    //write terminating charactersignature
    write(oh,"\n",1);
    if (NULL != str_sig)
        aegis_crypto_free(str_sig);
    return true;
}

#endif //QTWRT_ENABLE_AEGIS_LOCALSIG

