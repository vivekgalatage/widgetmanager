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

#ifndef _WACW3CELEMENT_H
#define _WACW3CELEMENT_H

#include <QXmlStreamAttributes>
#include "wacWidgetUtils.h"
class QString;
class QXmlStreamAttributes;
class WIDGETUTILS_EXPORT W3CElement
{

public :
   W3CElement();
   explicit W3CElement(const QString &elementName);
   QString  attributeValue(const QString &aString) const ;
   QString readElementText() const {return m_readElementText ;}
   void  setText(const QString &aString);
   void  setAttributes(const QXmlStreamAttributes &aAttributes);
   void  addAttribute(const QString &name, const QString &value);
   void  setnamespaceUri(const QString &aString);
   QXmlStreamAttributes  attributes() const { return m_attributes ;}
   QString namespaceUri() { return m_nsUri ; }
   const QString& elementName() const { return m_elementName;};

private :
   W3CElement(W3CElement &aElement);
   W3CElement & operator=(const W3CElement &aElement);

private :
   QXmlStreamAttributes m_attributes;
   QString m_readElementText;
   QString m_nsUri;
   QString m_elementName;
};

#endif


