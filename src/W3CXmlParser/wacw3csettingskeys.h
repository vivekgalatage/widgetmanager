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

#ifndef __WACW3CSETTINGSKEYS_H
#define __WACW3CSETTINGSKEYS_H

namespace W3CSettingsKey   {
   //widget Headers
   const  QString WIDGET_ID("widget/id");
   const  QString WIDGET_VERSION("widget/version");
   const  QString WIDGET_HEIGHT("widget/height");
   const  QString WIDGET_WIDTH("widget/width");
   const  QString WIDGET_NAMESPACE("widget/namespace");
   const  QString WIDGET_LANG("widget/lang");
   const  QString WIDGET_VIEWMODES("widget/viewmodes");

   //name
   const  QString  WIDGET_NAME ("widget/name");

   //description
   const  QString WIDGET_DESCRIPTION("widget/description");

   //author
   const  QString WIDGET_AUTHOR("widget/author");

   //license
   const  QString WIDGET_LICENSE("widget/license");

   //icon
   const  QString WIDGET_ICON("widget/icon/%1");

   //server
   const  QString WIDGET_SERVER("widget/server");

   //delta
   const  QString WIDGET_DELTA("widget/delta");

   //content
   const  QString WIDGET_CONTENT("widget/content");

   //access
   const  QString WIDGET_ACCESS("widget/access/%1");

   //feature
   const  QString WIDGET_FEATURE("widget/feature/%1");

   //param
   const  QString WIDGET_FEATURE_PARAM("widget/feature/%1/param/%2");

   //preference
   const  QString WIDGET_PREFERENCE("widget/preference/%1");

   //catgories
   const  QString WIDGET_CATEGORIES("widget/categories");

   // Default language key
   const QString WIDGET_LANGUAGE_KEY("default");
}

#endif
