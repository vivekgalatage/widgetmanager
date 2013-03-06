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

#include "webapplocalizer.h"
#include "wacsettings.h"
#include "WidgetUtilsLogs.h"
#include <QDir>
#include <QLocale>
#if defined(Q_OS_MAEMO6) || defined(Q_OS_MEEGO)
#include <MGConfItem>
#include <MLocale>
#endif

QHash<QString, QString> WebAppLocalizer::s_widgetExistsCache;

WebAppLocalizer::WebAppLocalizer()
{

}

WebAppLocalizer::~WebAppLocalizer()
{
    s_widgetExistsCache.clear();
}

// function to retrieve localized version of a file
// parameters:
//    file            path to file that needs a localized version
//    absolute        if true provide absolute path, if false relative path
//    root            path to root of widget
// return:
//    QString         path of localized file
//
QString WebAppLocalizer::getLocalizedFile(const QString& file, const bool absolute, 
        const QString& root)
{
    QString userAgentLang = platformLanguage();
    if (userAgentLang.isEmpty() )
        return file;

    QChar sep = QDir::separator();
    QString widgetRoot = QDir::fromNativeSeparators(root) + sep;

    // Find out relative path for with file name, without locales and language
    QString relativeFile = file;
    if (absolute) {
        relativeFile = QDir::fromNativeSeparators(file.mid(widgetRoot.length()));
        if (relativeFile.startsWith("locales/"))
            relativeFile = relativeFile.mid(relativeFile.indexOf('/', 8)+1);
        else
            relativeFile = file.mid(root.length()+1);
    }

    // Cache locales directory if not already done
    QString cacheDir = widgetRoot+"locales/";
    if (!s_widgetExistsCache.contains(cacheDir)) {
        QStringList dirList = QDir(cacheDir).entryList(QDir::CaseSensitive|
                                                        QDir::Dirs|
                                                        QDir::NoDotAndDotDot);
        foreach(const QString &dir, dirList) {
            // Have to cache real path because of case sensitivity
            s_widgetExistsCache.insert(cacheDir.toLower()+dir.toLower()+sep, 
                                        cacheDir+dir+sep);
        }
        // just mark locales directory as cached and add root dir
        s_widgetExistsCache.insert(cacheDir, "cached");
        s_widgetExistsCache.insert(widgetRoot.toLower(), widgetRoot);
    }

    // Create pathlist where to search fallback files
    QStringList pathList(widgetRoot+"locales/"+userAgentLang+sep);
    QString lang(userAgentLang);
    while (lang.contains('-')) {
        lang.truncate(lang.lastIndexOf('-'));
         // add truncated localreglang to search path
        pathList << widgetRoot+"locales/"+lang+sep;
    }
    pathList << widgetRoot; // and widget root directory as a last search path

    // Return orgigal file as default result if localized not found
    QString result = file;

    // Search files
    foreach(const QString &searchPath, pathList) {
        QString cacheKey = searchPath.toLower();
        // Check cache does localized dir exist
        if (s_widgetExistsCache.contains(cacheKey)) {
            // Does the file exist?
            QString searchFile = s_widgetExistsCache.value(cacheKey) + relativeFile;
            if (QFile::exists(searchFile)) {
                if (absolute)
                    result = searchFile;
                else
                    result = searchFile.mid(widgetRoot.length());
                break; // We find it, let's go out from foreach
            }
        }
    }

    return QDir::toNativeSeparators(result);
}

QString WebAppLocalizer::findStartFile(const QString& contentSource, 
        const QString& webAppRootDirectory)
{
    QStringList fileList;
    if (!contentSource.isEmpty()) {
        fileList.append(contentSource);
    }

    // Append default start files to list.
    fileList.append("index.htm");
    fileList.append("index.html");
    fileList.append("index.svg");
    fileList.append("index.xhtml");
    fileList.append("index.xht");

    foreach (const QString &fileName, fileList) {
        QString locFile = WebAppLocalizer::getLocalizedFile(fileName, false, 
                webAppRootDirectory);
        LOG("Localized start file: " << locFile);
        if (!locFile.isEmpty() && QFile::exists(webAppRootDirectory + 
                QDir::separator() + locFile)) {
            LOG("Localized start file: " << locFile);
            if (locFile.startsWith("locales/")) {
                // File name from locales/<lang>/ folder.
                int slashAfterLanguage = locFile.indexOf('/', 8);
                return locFile.mid(slashAfterLanguage + 1);
            } else {
                return locFile;
            }
        }
    }
    return QString();
}

QString WebAppLocalizer::platformLanguage()
{
    WAC::WrtSettings* settings = WAC::WrtSettings::createWrtSettings();
    QString userAgentLang = settings->valueAsString("UserAgentLanguage");
    if (userAgentLang.isEmpty() || userAgentLang.compare(" ") == 0) {
#if defined(Q_OS_MAEMO6) || defined(Q_OS_MEEGO) || defined(Q_OS_MAEMO5)

#if defined(Q_OS_MAEMO6) || defined(Q_OS_MEEGO)
        //Logic is similar to 'Language' control panel applet
        //Meego language is set using GConf
        const QString SettingsLanguage("/meegotouch/i18n/language");
        MGConfItem item (SettingsLanguage);
        QString resultLang = item.value().toString();
        if (resultLang.isEmpty()) {
            resultLang = MLocale().name(); // returning the default
        }
#else
        //Maemo5 language is set using 'setenv'
        QString resultLang = QString(qgetenv("LANG"));
        if (resultLang.isEmpty()) {
            resultLang = QLocale().name();
        }
#endif //#if defined(Q_OS_MAEMO6) || defined(Q_OS_MEEGO)

        // en means GB and not US, See bug: NB#167615
        if (resultLang == "en") {
            userAgentLang = "en_GB";
        } else {
            userAgentLang = QLocale(resultLang).name();
        }
        QLocale::setDefault(userAgentLang);
#else
        // Default to system local on non maemo/meego platforms.
        QLocale language = QLocale::system();
        userAgentLang = language.name();
        QLocale::setDefault(userAgentLang);
#endif
        userAgentLang = QLocale().name().toLower().replace(QString("_"),QString("-"));
    }
    return userAgentLang;
}

