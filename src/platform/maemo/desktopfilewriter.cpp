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

#include "desktopfilewriter.h"
#include "wacw3csettingskeys.h"
#include "wacSuperWidget.h"
#include "wrtglobal.h"

#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QDebug>

#if defined(Q_OS_MAEMO5)
static const QString SYSTEM_APPLICATION_DESKTOP_FILE_PATH("/usr/share/applications/hildon/");
static const QString SYSTEM_HOMESCREEN_DESKTOP_FILE_PATH("/usr/share/applications/hildon-home/");
#elif defined(Q_OS_MAEMO6)
static const QString SYSTEM_APPLICATION_DESKTOP_FILE_PATH("/usr/share/applications/");
static const QString SYSTEM_HOMESCREEN_DESKTOP_FILE_PATH("");
#elif defined(Q_OS_MEEGO)
static const QString SYSTEM_APPLICATION_DESKTOP_FILE_PATH("/usr/share/applications/meego/");
static const QString SYSTEM_HOMESCREEN_DESKTOP_FILE_PATH("/usr/share/applications/meego-home/");
#else
static const QString SYSTEM_APPLICATION_DESKTOP_FILE_PATH("");
static const QString SYSTEM_HOMESCREEN_DESKTOP_FILE_PATH("");
#endif

#if defined(Q_OS_MAEMO6)
static const QString SYSTEM_HOMESCREEN_DESKTOP_TYPE("X-MeeGoApplet");
#else
static const QString SYSTEM_HOMESCREEN_DESKTOP_TYPE("W3CWidget");
#endif

static const QString WRT_DESKTOP_FILE_PATH("/usr/share/webwidgets/applications/");


DesktopFileWriter::DesktopFileWriter(const QString& rootPath, const QString& installationPath, const QString& appId)
    : m_isHidden(false), m_rootPath(rootPath), m_installationPath(installationPath), m_appId(appId)
{
    // do nothing
}

DesktopFileWriter::~DesktopFileWriter()
{
    // do nothing
}

void DesktopFileWriter::setAppRunner(const QString& appRunner)
{
    m_appRunner = appRunner;
}

void DesktopFileWriter::setAppName(const QString& appName)
{
    m_appName = appName;
}

void DesktopFileWriter::setAppIcon(const QString& appIcon)
{
    m_appIcon = appIcon;
}

void DesktopFileWriter::setApplicationViewMode(const QString& applicationViewMode)
{
    m_applicationViewMode = applicationViewMode;
}

void DesktopFileWriter::setHomescreenViewMode(const QString& homescreenViewMode)
{
    m_homescreenViewMode = homescreenViewMode;
}

void DesktopFileWriter::setHidden(const bool isHidden)
{
    m_isHidden = isHidden;
}

void DesktopFileWriter::setPackageName(const QString& packageName)
{
    m_packageName = packageName;
}

bool DesktopFileWriter::write(SuperWidget* widget)
{
    if (m_appName.isEmpty() || m_appIcon.isEmpty() || m_packageName.isEmpty()
        || m_appId.isEmpty() || m_installationPath.isEmpty() || m_rootPath.isEmpty()
        || (m_homescreenViewMode.isEmpty() && m_applicationViewMode.isEmpty()))
        return false;

    bool result = true;

    if (widget)
        setWidgetNameTranslations(widget);

    if (!m_homescreenViewMode.isEmpty())
        result = writeDesktopFile(Homescreen);

    if (result && !m_applicationViewMode.isEmpty())
        result = writeDesktopFile(Application);

    return result;
}

QString DesktopFileWriter::desktopFilePath()
{

    if (!m_applicationViewMode.isEmpty()) {
        if(m_isHidden){
            return WRT_DESKTOP_FILE_PATH + "hidden/" + m_packageName + ".desktop";
        }
        return WRT_DESKTOP_FILE_PATH + m_packageName + ".desktop";
    }
    return SYSTEM_HOMESCREEN_DESKTOP_FILE_PATH + m_packageName + ".desktop";
}

bool DesktopFileWriter::writeDesktopFile(DesktopFileWriter::Type type)
{
    // set the system specific desktop file path
    QString systemDesktopFilePath;
    if (type == Application) {
        systemDesktopFilePath = m_rootPath + SYSTEM_APPLICATION_DESKTOP_FILE_PATH;
    } else { // if (type == Homescreen)
        systemDesktopFilePath = m_rootPath + SYSTEM_HOMESCREEN_DESKTOP_FILE_PATH;
    }

    if (m_isHidden)
        systemDesktopFilePath.append("hidden/");

    QDir desktopDir(systemDesktopFilePath);
    if (!desktopDir.exists())
        desktopDir.mkpath(systemDesktopFilePath);

    systemDesktopFilePath.append(m_packageName + ".desktop");

    // set the desktop file path for WRT
    QString wrtDesktopFilePath;
    if (type == Application) {
        wrtDesktopFilePath = m_rootPath + WRT_DESKTOP_FILE_PATH;
        if (m_isHidden)
            wrtDesktopFilePath.append("hidden/");

        desktopDir.setPath(wrtDesktopFilePath);
        if (!desktopDir.exists())
            desktopDir.mkpath(wrtDesktopFilePath);

        wrtDesktopFilePath.append(m_packageName + ".desktop");
    }

    // write the desktop file
    QString content;
    addSystemSection(type, content);
    addWidgetSection(type, content);

    QFile desktopFile(systemDesktopFilePath);
    if (!desktopFile.open(QIODevice::ReadWrite | QIODevice::Truncate)
        || -1 == desktopFile.write(content.toUtf8()))
        return false;
    desktopFile.close();

    if (type == Application) {
        content.clear();
        addWRTSection(content);
        addWidgetSection(type, content);

        desktopFile.setFileName(wrtDesktopFilePath);

        if (!desktopFile.open(QIODevice::ReadWrite | QIODevice::Truncate)
            || -1 == desktopFile.write(content.toUtf8()))
            return false;
        desktopFile.close();
    }

    return true;
}

void DesktopFileWriter::addSystemSection(Type type, QString& content)
{
    content.append("# This file was automatically generated by Qt Web Runtime\n"
                   "# " + QDateTime::currentDateTime().toString() + "\n"
                   "[Desktop Entry]\n");

    if (type == Application) {
        content.append("Type=Application\n");
    } else if (type == Homescreen) {
        content.append("Type=" + SYSTEM_HOMESCREEN_DESKTOP_TYPE + "\n");
    }

    content.append("Name=" + m_appName + "\n");
    QMap<QString, QString>::const_iterator i = m_appNameTranslations.begin();
    while (i != m_appNameTranslations.end()) {
        if (!i.value().isEmpty()){
            content.append("Name[" + i.key() + "]=" + i.value() + "\n");
        }
        i++;
    }
    content.append("Icon=" + m_appIcon + "\n");

#if defined(Q_OS_MEEGO)
    content.append("Categories=Utility;\n");
#endif

#if defined(Q_OS_MAEMO6)
    if (m_isHidden){
        content.append("Hidden=true\n");
    }
    if (type == Application) {
        if (m_appRunner.isEmpty()) {
            content.append("Exec=/usr/bin/webwidgetrunner " + WRT_DESKTOP_FILE_PATH + m_packageName + ".desktop\n");
        } else {
            content.append("Exec=" + m_appRunner + "\n");
        }
        content.append("OnlyShowIn=X-MeeGo;\n"
                       "[X-MeeGoApplet]\n");
    } else { // if (type == Homescreen)
        content.append("Exec=/usr/bin/webwidgetrunner\n"
                       "[X-MeeGoApplet]\n"
                       "Applet=libmaemowrt-applet.so\n");
    }
#else
    if (type == Application) {
        content.append("Exec=webwidgetrunner " + SYSTEM_APPLICATION_DESKTOP_FILE_PATH + m_packageName + ".desktop\n");
    } else { // if (type == Homescreen)
        content.append("X-Path=" + m_appName + "\n");
    }
#endif
}

void DesktopFileWriter::addWidgetSection(Type type, QString& content)
{
    content.append("[WRT]\n"
                   "Type=W3CWidget\n"
                   "StartHtml=file://" + m_installationPath + "\n"
                   "WUID=" + m_packageName + "\n");

    if (type == Application) {
        content.append("ViewMode=" + m_applicationViewMode + "\n");
    } else { // if (type == Homescreen)
        content.append("ViewMode=" + m_homescreenViewMode + "\n");
    }

    content.append("WebAppId=" + m_appId + "\n");
}

void DesktopFileWriter::addWRTSection(QString& content)
{
    content.append("# This file was automatically generated by Qt Web Runtime\n"
                   "# " + QDateTime::currentDateTime().toString() + "\n"
                   "[Desktop Entry]\n"
                   "Type=" + SYSTEM_HOMESCREEN_DESKTOP_TYPE + "\n"
                   "Name=" + m_appName + "\n");
    QMap<QString, QString>::iterator i = m_appNameTranslations.begin();
    while (i != m_appNameTranslations.end()) {
        if (!i.value().isEmpty()){
            content.append("Name[" + i.key() + "]=" + i.value() + "\n");
        }
        i++;
    }
    content.append("Icon=" + m_appIcon + "\n");

#if defined(Q_OS_MAEMO6)
    if (m_appRunner.isEmpty()) {
        content.append("Exec=/usr/bin/webwidgetrunner\n");
    } else {
        content.append("Exec=" + m_appRunner + "\n");
    }
    content.append("[X-MeeGoApplet]\n"
                   "Applet=libmaemowrt-applet.so\n");
#else
    content.append("X-Path=" + m_appName + "\n");
#endif
}

void DesktopFileWriter::setWidgetNameTranslations(SuperWidget* widget)
{
    m_appNameTranslations.clear();

    WidgetManifestInfo *manifestinfo = widget->getWidgetManifestInfo();
    if (!manifestinfo)
        return;

    QSet<QString> languageList = manifestinfo->languages();

    foreach (QString language, languageList) {
        QString widgetName = manifestinfo->value(W3CSettingsKey::WIDGET_NAME, "", language);

        // Replacing language-region like: en-us with en_US according to freedesktop standards
        int pos = language.indexOf("-", 0);
        if (pos > 0) {
            QString region = '_' + language.right(language.length() - pos - 1).toUpper();
            language.replace(pos, region.length(), region);
        }

        if (widgetName.isEmpty())
            widgetName = QFileInfo(widget->widgetBundlePath()).completeBaseName();
        m_appNameTranslations.insert(language, widgetName);
    }
}
