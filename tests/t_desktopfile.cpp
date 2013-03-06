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

#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QTextStream>

#include "wacWidgetManager.h"
#include "wacw3csettingskeys.h"

#include "t_desktopfile.h"
#include "apitestconstants.h"
#include "testhelpers.h"

const QRegExp desktopExp("^Name\\s*(\\[(.*)\\]){0,1}\\s*=\\s*(.*)");
const QRegExp xmlExp("/widget/name$");

using namespace WAC;

extern "C" WrtDesktopFileTest* createDesktopFileTests()
{
    return new WrtDesktopFileTest();
}

WrtDesktopFileTest::WrtDesktopFileTest(QWidget *parent)
    : QObject(parent)
{
}

 WrtDesktopFileTest::~WrtDesktopFileTest()
{
}

void WrtDesktopFileTest::initTestCase()
{
    __countTestsHelper__(this);
}

void WrtDesktopFileTest::cleanupTestCase()
{
    QCOMPARE (0, twidgetUnInstall(m_widgetId));
}

void WrtDesktopFileTest::WidgetDesktopFileTest()
{
    QString pkgpath = APITEST_WIDGETBASE + QString("widgets/lwidgetname.wgt");

    // Install localized widget
    m_widgetId = twidgetInstall(pkgpath);
    m_appPath = "/usr/share/wrt/data/widgets_21D_4C7/" + m_widgetId;
    m_widgetId +=  "-wrt-widget";
    QString desktopFilePath = "/usr/share/applications/" + m_widgetId + ".desktop";
    QFile file(desktopFilePath);
    QCOMPARE(true, file.open(QIODevice::ReadOnly));
    QTextStream stream(&file);
    QStringList widgetNames;
    QString line;
    while (!stream.atEnd()) {
        line = stream.readLine();
        if (line.contains(desktopExp))
            widgetNames.append(line);
    }
    file.close();
    QCOMPARE(true, checkDesktopFile(widgetNames));
}

QString WrtDesktopFileTest::twidgetInstall(QString &pkgpath)
{
    WidgetManager widgetInstaller(NULL);
    QString widgetId;
    pkgpath = QDir::toNativeSeparators(pkgpath);
    WidgetInstallError errCode = WidgetInstallSuccess;
    errCode = widgetInstaller.install(pkgpath, widgetId, true, false);

    switch (errCode) {
    case WidgetInstallSuccess:
        QWARN("WidgetInstallSuccess");
        return widgetId;
    case WidgetValidSignature:
        QWARN("WidgetValidSignature");
        return widgetId;
    case WidgetUnZipBundleFailed:
        QWARN("WidgetUnZipBundleFailed");
        return widgetId;
    case WidgetFindSignatureFailed:
        QWARN("WidgetFindSignatureFailed");
        return widgetId;
    case WidgetSignatureParsingFailed:
        QWARN("WidgetSignatureParsingFailed");
        return widgetId;
    case WidgetSignatureOrSignedInfoMissing:
        QWARN("WidgetSignatureOrSignedInfoMissing");
        return widgetId;
    case WidgetSignatureRefExistFailed:
        QWARN("WidgetSignatureRefExistFailed");
        return widgetId;
    case WidgetSignatureRefValidationFailed:
        QWARN("WidgetSignatureRefValidationFailed");
        return widgetId;
    case WidgetCertValidationFailed:
        QWARN("WidgetCertValidationFailed");
        return widgetId;
    case WidgetSignatureValidationFailed:
        QWARN("WidgetSignatureValidationFailed");
        return widgetId;
    case WidgetParseManifestFailed:
        QWARN("WidgetParseManifestFailed");
        return widgetId;
    case WidgetRegistrationFailed:
        QWARN("WidgetRegistrationFailed");
        return widgetId;
    case WidgetReplaceFailed:
        QWARN("WidgetReplaceFailed");
        return widgetId;
    case WidgetRmDirFailed:
        QWARN("WidgetRmDirFailed");
        return widgetId;
    case WidgetCapabilityNotAllowed:
        QWARN("WidgetCapabilityNotAllowed");
        return widgetId;
    case WidgetPlatformSpecificInstallFailed:
        QWARN("WidgetPlatformSpecificInstallFailed");
        return widgetId;
    case WidgetCorrupted:
        QWARN("WidgetCorrupted");
        return widgetId;
    case WidgetSharedLibraryNotSigned:
        QWARN("WidgetSharedLibraryNotSigned");
        return widgetId;
    case WidgetDriveLetterValidationFailed:
        QWARN("WidgetDriveLetterValidationFailed");
        return widgetId;
    case WidgetTypeValidationFailed:
        QWARN("WidgetTypeValidationFailed");
        return widgetId;
    case WidgetSystemError:
        QWARN("WidgetSystemError");
        return widgetId;
    case WidgetInstallPermissionFailed:
        QWARN("WidgetInstallPermissionFailed");
        return widgetId;
    case WidgetUpdateFailed:
        QWARN("WidgetUpdateFailed");
        return widgetId;
    case WidgetUpdateVersionCheckFailed:
        QWARN("WidgetUpdateVersionCheckFailed");
        return widgetId;
    case WidgetUserConfirmFailed:
        QWARN("WidgetUserConfirmFailed");
        return widgetId;
    case WidgetInstallFailed:
        QWARN("WidgetInstallFailed");
        return widgetId;
    case WidgetStartFileNotFound:
        QWARN("WidgetStartFileNotFound");
    default:
        QWARN("Unknown Error");
        return widgetId;
    }
}

int WrtDesktopFileTest::twidgetUnInstall(QString &id)
{
    WidgetManager widgetInstaller(NULL) ;
    if (widgetInstaller.uninstall(id, true) == WidgetUninstallSuccess)
         return true;
     else
         return false;
}

bool WrtDesktopFileTest::checkDesktopFile(QStringList fileWidgetNames)
{
    WidgetManager wgtMgr(NULL);
    WidgetProperties *WgtProperties = wgtMgr.getProperties(m_appPath);
    QMap<QString, QVariant> pList = WgtProperties->plist();
    QStringList keys = pList.keys();
    for (int i = 0; i < keys.count(); i++) {
        QString key = keys.at(i);
        if (!key.contains(xmlExp)) {
            keys.removeAt(i);
            i--;
        }
    }

    //fill qmap with values from xml file, language is key, widget name is value: "en_us", "EngUS"
    QMap<QString, QString> xmlWidgetNames;
    for (int i = 0; i < keys.count(); i++) {
        QString key = keys.at(i);
        QString name = WgtProperties->plistValue(key).toString();
        if (name.isEmpty())
            name = "lwidgetname";
        xmlWidgetNames.insert(key.remove(xmlExp).replace("-","_"), name);
    }
    qDebug() << "Names in the config.xml file:" << xmlWidgetNames;

    //fill qmap with values from desktop file, language is key, widget name is value: "en_us", "EngUS"
    QMap<QString, QString> desktopWidgetNames;
    for (int i = 0; i < fileWidgetNames.count(); i++) {
        QString nameValue = fileWidgetNames.at(i);
        int pos = 0;
        while ((pos = desktopExp.indexIn(nameValue, pos)) != -1) {
            if (desktopExp.cap(1).isEmpty())
                desktopWidgetNames.insert("default", desktopExp.cap(3));
            else
                desktopWidgetNames.insert(desktopExp.cap(2).toLower(), desktopExp.cap(3));
            pos += desktopExp.matchedLength();
        }
    }
    qDebug() << "Names in the desktop file:" << desktopWidgetNames;

    //validate if values from xml file are present in desktop file
    QMap<QString, QString>::iterator i = xmlWidgetNames.begin();
    while (i != xmlWidgetNames.end()) {
        if (desktopWidgetNames.value(i.key()).isEmpty())
            return false;
        if (desktopWidgetNames.value(i.key()).compare(i.value()))
            return false;
        i++;
    }

    return true;
}
