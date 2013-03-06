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

#ifndef WIDGETINSTALLER_H_
#define WIDGETINSTALLER_H_

#include <QDir>
#include "wacSuperWidget.h"

namespace WAC {
    class Storage;
}

class WidgetInstallerPrivate;

/**
 * Interface for handling W3C Widget installation.
 */
class WidgetInstaller {

public:

    explicit WidgetInstaller(SuperWidget * webWidget, WAC::Storage * storage);
    ~WidgetInstaller();

    /**
     * Installs source directory in to a target directory
     * @param source path to source directory
     * @param target path to destination directory
     * @param appId application identifier of installed application
     */
    bool install(const QDir& source, const QDir& target, const QString& appId);

    /**
     * Updates source directory in to a target directory
     * @param source path to source directory
     * @param target path to destination directory
     * @param appId application identifier of installed application
     */
    bool update(const QDir& source, const QDir& target, const QString& appId);

    /**
     * Uninstalls the widget with given application identified and path
     * @param applicationPath path to installed application
     * @param appId application identifier of installed application
     */
    static bool uninstall(const QString& applicationPath, const QString& appId);

    void setInstallationAttributes(const WAC::InstallationAttributes& attributes);

private:
    Q_DISABLE_COPY(WidgetInstaller)
    WidgetInstallerPrivate * d;
};

#endif /* WIDGETINSTALLER_H_ */
