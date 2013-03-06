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

#ifndef _PROPRIETARYTAGS_H
#define _PROPRIETARYTAGS_H

namespace SharedLibraryTags
{
  const QString SHARED_LIBRARY_TAG("sharedlibrary");
  const QString SHARED_LIBRARY_FOLDER_TAG("folder"); //element under shared library element
  const QString SHARED_LIBRARY_WIDGET_TAG("widget"); //element under shared library element

}
namespace ProprietaryTags
{
  const QString BACKGROUND_TIMER_TAG("timer");
  const QString VIEWMODE_SETTINGS_TAG("viewmodes");
  const QString VIEWMODE_MINIMIZED_SETTINGS_TAG("minimized");
}
namespace HiddenTags
{
  const QString HIDDEN_WIDGET_TAG("hidden");
}

namespace PreinstallTags
{
const QString PREINSTALL_WIDGET_TAG("preinstall");
}

#endif



