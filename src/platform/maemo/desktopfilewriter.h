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

#ifndef DESKTOPFILEWRITER_H
#define DESKTOPFILEWRITER_H

#include <QString>
#include <QMap>

class SuperWidget;


/*!
  \class DesktopFileWriter
  \brief A class used to create desktop files for widgets.
  */
class DesktopFileWriter
{
    Q_DISABLE_COPY(DesktopFileWriter)

public:
    /*!
     Construct an object to write the desktop files. The root of the unzipped widgets is given by \a rootPath, and the widgets are supposed to be
     installed to \installationPath, while the unique ID is given by \a appId.
     */
    explicit DesktopFileWriter(const QString& rootPath, const QString& installationPath, const QString& appId);

    /*!
      Destroys the object.
    */
    ~DesktopFileWriter();

    /*!
     Set the application runner as \a appRunner, which is used by Ovi.
     */
    void setAppRunner(const QString& appRunner);

    void setAppName(const QString& appName);
    void setAppIcon(const QString& appIcon);
    void setApplicationViewMode(const QString& applicationViewMode);
    void setHomescreenViewMode(const QString& homescreenViewMode);
    void setHidden(const bool isHidden);
    void setPackageName(const QString& packageName);

    /*!
     Write the desktop files. Note that all the above attributes should be set before calling this, except application runner.
     */
    bool write(SuperWidget* widget);

    QString desktopFilePath();

private:
    enum Type {
        Application,
        Homescreen
    };

    bool writeDesktopFile(Type type);
    void addSystemSection(Type type, QString& content);
    void addWRTSection(QString& content);
    void addWidgetSection(Type type, QString& content);
    void setWidgetNameTranslations(SuperWidget* widget);

    bool m_isHidden;
    QString m_rootPath;
    QString m_installationPath;
    QString m_appId;
    QString m_appRunner;
    QString m_appName;
    QMap<QString, QString> m_appNameTranslations;
    QString m_appIcon;
    QString m_packageName;
    QString m_applicationViewMode;
    QString m_homescreenViewMode;
};

#endif // DESKTOPFILEWRITER_H
