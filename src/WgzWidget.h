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

#ifndef _WGZ_WIDGET_H_
#define _WGZ_WIDGET_H_

#include "wacSuperWidget.h"

/*****************************************************************************
 * Wgz specific widget
 *
 * *************************************************************************/
class WgzWidget: public SuperWidget {
public:
    explicit WgzWidget(QString& rootDirectory);
    virtual ~WgzWidget() {};

    WidgetInstallError install(const bool update=false);
    QString launcherPath(const QString &pkgPath);
    void writeManifest(const QString& path="");
    QString value(const QString& key, const QString & attribute = QString(""));

    /**
     * Returns true if the key with the attribute exists in widget metadata; otherwise false.
     * @param key a key in plist metadata
     * @attribute the attribute is an optional argument. With attribute you can test whether a value
     * of given attribute within specified key exists in plist metadata.
     */
    bool contains( const QString & key, const QString & attribute = QString(""));

    bool unZipBundle(const QString& path){return SuperWidget::unZipBundle(path);};
    bool parseManifest(const QString& path="", const bool force=false);
    bool allowRemovableInstallation();

protected:
    void initialize(QString& rootDirectory) {SuperWidget::initialize(rootDirectory);};
    WidgetProperties* widgetProperties(bool forceUpdate = false, bool minimal = false);
    bool findStartFile(QString& startFile,const QString& path="");
    bool findIcons(QStringList& icons, const QString& path="");
    bool findFeatures(WidgetFeatures& features, const QString& widgetPath="");

    bool setWidgetRootPath(const QString& path="");
};
#endif
