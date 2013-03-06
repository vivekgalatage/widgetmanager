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

#ifndef _WAC_WIDGET_MANAGER_CONSTANTS_H_
#define _WAC_WIDGET_MANAGER_CONSTANTS_H_

#include <QDir>
#include "wrtglobal.h"

#define CONTENT_TYPE_WGZ "application/x-nokia-widget"
#define CONTENT_TYPE_WGT "application/widget"
#define CONTENT_TYPE_UNKNOWN "Unknown"

#define EXTENSION_WGZ ".wgz"
#define EXTENSION_WGT ".wgt"

#define WIDGET_PACKAGE_FORMAT_WGT "w3c-partial-v1"
#define WIDGET_PACKAGE_FORMAT_WGZ "wgz"
#define WIDGET_PACKAGE_FORMAT_SHARED_LIBRARY "shared-library"
#define WIDGET_PACKAGE_FORMAT_JIL "jil"
#define WIDGET_PACKAGE_FORMAT_OVIAPP "ovi-application"

#define CAPABILITY_OVI "ovi"

const char WIDGET_PACKAGE_MAGIC_NUMBER_WGT[] = {0x50, 0x4B, 0x03, 0x04};

#ifdef CWRT_WIDGET_FILES_IN_SECURE_STORAGE
const QString WIDGET_STORAGE("WidgetsSecureStore");
#endif

const QString WIDGET_FOLDER("widgets_21D_4C7");
const QString SHARED_LIBRARY_FOLDER("lib");

static const char DATA_FOLDER[]        = "data";
static const char SECSESSION_FILE[]         = "secsession";

#if defined(Q_OS_SYMBIAN)
/* The following default locations are true for normal widget installations, using the widget
 * installer. For embedded widgets, the locations below are different and it is under the host
 * application. During installation, we maintain those values in a few variables in superwidget
 * and once installation is done, these locations should be retrieved from the widget registry
 * for embedded widgets, all these locations are relative to the root anyways, so once we get
 * the root directory from the registry, it should be easy to derive others
 */
const QString DEFAULT_ROOT_DIRECTORY("C:\\Private\\2003DE07");
const QString DEFAULT_DIRECTORY("2003DE07");
const QString PRIVATE_DIRECTORY(":/Private");
const QString DATA_PATH(DEFAULT_ROOT_DIRECTORY + QString(DATA_FOLDER) + "\\");
const QString INI_PATH("C:\\Private\\2003DE0D\\multiprocini\\multiprocsettings.ini");
#define BACKUP_RESTORE_UID 0x2003DE0F
#define MEDIA_MANAGER_UID 0x2003DE1D

// This property is used by USIF Installer and Embed Installer for getting the widget uniqueid
enum TWidgetInstallPropertyKeys {
    EWidgetUniqueIdProperty=99
};

#elif defined(Q_OS_MAEMO6) || defined(Q_OS_MAEMO5)

/* Maemo directories:
 *   /home/user/.local/share/data/wrt/<id>   -- widget runtime generated data
 *   /usr/share/wrt/data/widgets_21D_4C7     -- widget installation directory
 *   /usr/share/wrt/data/widgets_21D_4C7/lib -- shared JS libraries
 */
#include <QDesktopServices>

const QString DEFAULT_ROOT_DIRECTORY = QString("/usr/share/wrt/data/");
const QString DATA_PATH = 
        QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "wrt/";

#elif defined(Q_OS_LINUX) || defined(Q_OS_MEEGO)
const QString DEFAULT_ROOT_DIRECTORY = QString("/usr/share/wrt/");
const QString DATA_PATH = DEFAULT_ROOT_DIRECTORY + DATA_FOLDER + QDir::separator();

#else // win32
#include <QDesktopServices>
const QString DEFAULT_ROOT_DIRECTORY = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
const QString DATA_PATH = DEFAULT_ROOT_DIRECTORY + 
                            QDir::separator() + 
                            QString(DATA_FOLDER) + 
                            QDir::separator();
#endif

// Registry location
#if defined(Q_OS_SYMBIAN)
const QString REGISTRY_DB_PATH("C:\\Private\\2003DE23");
const QString WIDGET_INSTALL_PATH = DEFAULT_ROOT_DIRECTORY + 
                                    QDir::separator() + 
                                    WIDGET_FOLDER + 
                                    QDir::separator();
#else
const QString REGISTRY_DB_PATH = DEFAULT_ROOT_DIRECTORY;
const QString WIDGET_INSTALL_PATH = DEFAULT_ROOT_DIRECTORY + 
                                    WIDGET_FOLDER + 
                                    QDir::separator();
#endif

// Widget and Shared library location
const QString SHARED_LIBRARY_PATH = WIDGET_INSTALL_PATH + 
                                    SHARED_LIBRARY_FOLDER + 
                                    QDir::separator();

#if ENABLE(UNIX_CERTIFICATE_MANAGER)
const QString CERTIFICATE_PATH("/etc/certs/codesigning");
#endif


#endif // _WAC_WIDGET_MANAGER_CONSTANTS_H_
