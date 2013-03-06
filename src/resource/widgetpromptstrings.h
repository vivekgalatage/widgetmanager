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

#ifndef _WRT_PROMPT_STRINGS_H_
#define _WRT_PROMPT_STRINGS_H_

#include <QCoreApplication>

// This file contains all the UI strings used in WRT
//template
// QCoreApplication::translate(<context>, QT_TRANSLATE_NOOP(<context>, <string_to_be_localized>) where <context> can be a component name
// to assist proper translation process

#define TR_WM_WIDGET_UNINSTALL            QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Widget uninstall"))
#define TR_WM_WIDGET_REMOVE_QUERY         QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Do you want to remove "))
#define TR_WM_WIDGET_INSTALL              QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Widget install"))
#define TR_WM_WIDGET_INSTALL_QUERY        QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Do you want to install "))
#define TR_WM_WIDGET_INSTALL_FAILED       QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Installation failed."))
#define TR_WM_WIDGET_INSTALL_ERROR        QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Widget install error"))
#define TR_WM_WIDGET_INSTALL_SUCCESS      QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Widget successfully installed."))
#define TR_WM_WIDGET_INSTALL_CANCELLED    QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Installation cancelled."))
#define TR_WM_WIDGET_REPLACE              QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Replace existing widget"))
#define TR_WM_WIDGET_SIGNATURE_INVALID    QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Signature invalid."))
#define TR_WM_WIDGET_UNKNOWN_ERROR        QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Unknown error."))
#define TR_WM_WIDGET_INSTALL_CONT_QUERY   QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", " Do you still want to install?"))
#define TR_WM_WIDGET_INSTALL_ALLOW        QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Allow "))
#define TR_WM_WIDGET_INSTALL_TO_ACCESS    QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", " to access:"))
#define TR_WM_WIDGET_INSTALL_FEATURE_CONTACT    QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Contact"))
#define TR_WM_WIDGET_INSTALL_FEATURE_CALENDAR   QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Calendar"))
#define TR_WM_WIDGET_INSTALL_FEATURE_SYSINFO    QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Sysinfo"))
#define TR_WM_WIDGET_INSTALL_FEATURE_CAMERA     QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Camera"))
#define TR_WM_WIDGET_INSTALL_FEATURE_MESSAGING  QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Messaging"))
#define TR_WM_WIDGET_INSTALL_FEATURE_COMMLOG    QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Communication Logs"))
#define TR_WM_WIDGET_INSTALL_FEATURE_LANDMARK   QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Landmark"))
#define TR_WM_WIDGET_INSTALL_FEATURE_LOCATION   QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Location"))
#define TR_WM_WIDGET_INSTALL_FEATURE_MEDIA      QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Media"))
#define TR_WM_WIDGET_INSTALL_FEATURE_AUDIOPLAYER QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "AudioPlayer"))
#define TR_WM_WIDGET_INSTALL_FEATURE_VIDEOPLAYER QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "VideoPlayer"))
#define TR_WM_WIDGET_INSTALL_FEATURE_FILESYSTEM QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Filesystem"))
#define TR_WM_WIDGET_INSTALL_FEATURE_SENSOR     QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Sensor"))
#define TR_WM_WIDGET_INSTALL_FEATURE_TELEPHONY  QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Telephony"))
#define TR_WM_SECURITY_WARNING            QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Security Warning"))


#define TR_WM_WIDGET_SELECT_DRIVE QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", ""))
#define TR_WM_WIDGET_SELECT_DRIVE_MESSAGE QCoreApplication::translate("WidgetManager", QT_TRANSLATE_NOOP("WidgetManager", "Select installation drive:"))

#endif //_WRT_PROMPT_STRINGS_H_
