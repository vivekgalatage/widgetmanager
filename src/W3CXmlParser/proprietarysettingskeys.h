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

#ifndef __PROPRIETARYSETTINGSKEYS_H
#define __PROPRIETARYSETTINGSKEYS_H

namespace ProprietarySettingsKey   {

   //Shared Library
   const  QString WIDGET_SHARED_LIBRARY ("widget/NOKIA:sharedlibrary");

   //folder name
   const  QString WIDGET_SHARED_LIBRARY_FOLDER("widget/NOKIA:sharedlibrary/NOKIA:folder");

   //is widget
   const  QString WIDGET_SHARED_LIBRARY_WIDGET("widget/NOKIA:sharedlibrary/NOKIA:widget");

   //widgettype
   const QString WIDGET_TYPE("widgettype");

   //Background Timers
   // <NOKIA:timer>neversuspend</NOKIA:timer>
   const QString WIDGET_BACKGROUND_TIMERS("widget/NOKIA:timer");

   //widget is hidden
   const QString WIDGET_HIDDEN("widget/NOKIA:hidden");

   //widget is preinstalled
   const QString WIDGET_PREINSTALL("widget/NOKIA:preinstall");

   const QString WIDGET_VIEWMODE_SETTINGS("widget/NOKIA:viewmodes");
   const QString WIDGET_VIEWMODE_MINIMIZED_SETTINGS("widget/NOKIA:viewmodes/NOKIA:minimized");
}
#endif
