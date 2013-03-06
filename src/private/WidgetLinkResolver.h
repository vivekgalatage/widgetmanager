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

#ifndef _WIDGET_LINK_RESOLVER_H_
#define _WIDGET_LINK_RESOLVER_H_

#include "wacWidgetUtils.h"

class WidgetLinkResolver
{
  public:
    WIDGETUTILS_EXPORT static const QString resolveLink (const QString& dirPath, const QString& fileName);
    WIDGETUTILS_EXPORT static const QString installedLocation();
    WIDGETUTILS_EXPORT static const QString preInstalledLocation();
    WIDGETUTILS_EXPORT static const QString dataStorageLocation();
};

#endif
