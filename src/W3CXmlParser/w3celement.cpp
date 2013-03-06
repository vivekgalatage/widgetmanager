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

#include <QString>
#include <QXmlStreamAttributes>
#include "wacw3celement.h"

W3CElement::W3CElement()
{
}

W3CElement::W3CElement(const QString &elementName)
{
    m_elementName = elementName;
}

void W3CElement::setAttributes(const QXmlStreamAttributes &aAttributes)
{
    m_attributes = aAttributes;
}


void W3CElement::setnamespaceUri(const QString &aString)
{
    m_nsUri = aString;
}

void W3CElement::setText(const QString &aString)
{
    m_readElementText = aString ;
}

QString W3CElement::attributeValue(const QString &aString) const
{
    return m_attributes.value(aString).toString();
}
void  W3CElement::addAttribute(const QString &name, const QString &value)
{
    QXmlStreamAttribute xmlAttr (name, value);
    m_attributes.append(xmlAttr);
}

