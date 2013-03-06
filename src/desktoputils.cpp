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

#include "desktoputils.h"
#include "wacSuperWidget.h"
#include "w3cxmlplugin.h"
#include "w3ctags.h"
#include "wacw3csettingskeys.h"
#include "private/wacWidgetInfo.h"

#include <QtGui>
void DesktopUtils::WriteDesktopFile(SuperWidget* widget, const QString& packageName, 
const QString &installationPath, const QString &appId, WidgetInfo* widgetInfo)
{
    // Don't generate desktop files for shared libraries and hidden widgets
    if (widget->getProperties()->isSharedLibraryWidget() || 
	widget->getProperties()->isSharedLibraryWidget())
        return;
    
    W3cXmlPlugin* pluginInfo = static_cast<W3cXmlPlugin*>(widgetInfo);
    QString name;
    for (int i = 0; i < packageName.length(); i++) {
        if (!packageName.at(i).isSpace())
            name.append(packageName.at(i));
    }
    QString filename = "/usr/share/applications/" + name + ".desktop";

    QString str;
    str += "[Desktop Entry]\n";
    str += "Name=" + packageName + '\n';

    str += "Type=Application\n";
    QString description = pluginInfo->value(W3CSettingsKey::WIDGET_DESCRIPTION, "", "");
    if (description.isEmpty())
        description = packageName;
    
    str += "Description=" + description + '\n';

    QString iconFileName = widget->getProperties()->iconPath();

    str += "Icon=" + iconFileName + '\n';

    str += "Exec=webwidgetrunner %k\n";// \"" + filename + "\"\n";
    str += "Terminal=false\n";

    QString categories = pluginInfo->value(W3CSettingsKey::WIDGET_CATEGORIES);
    if (categories.isEmpty())
        categories = "Application;";
    str += "Categories=" + categories + '\n';

    str += '\n';

    str += "[WRT]\n";
    str += "Type=Plain\n";

    QString width = pluginInfo->value(W3CSettingsKey::WIDGET_WIDTH);
    QString height = pluginInfo->value(W3CSettingsKey::WIDGET_HEIGHT);
    if (!width.isEmpty())
        str += "Width=" + width + '\n';

    if (!height.isEmpty())
        str += "Height=" + height + '\n';

    str += "StartHtml=" + installationPath + '\n';

    str += "WUID=" + appId + "-wrt-widget\n";
    str += "WebAppId=" + appId + '\n';

    QFile file;
    file.setFileName(filename);
    file.open(QIODevice::Truncate | QIODevice::WriteOnly);
    file.write(str.toAscii());
    file.close();

// FIXME unrelated here:
// 1. integrate with RPM
// 3. menus callback
    
}
