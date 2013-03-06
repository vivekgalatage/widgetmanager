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

#ifndef WIDGETUTILS_EXPORT
/**This file should be included in all header files that make symbols available to the user
 * of the WidgetUtils library.
 * All symbols exported from the library should be prefixed by
 * WIDGET_UTILS_EXPORT in order to be correctly exported on all supported platforms.
*/
#ifdef BUILDING_WIDGETUTILS_LIB  
#define WIDGETUTILS_EXPORT Q_DECL_EXPORT
#else
#define WIDGETUTILS_EXPORT Q_DECL_IMPORT
#endif
#endif
