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
#ifndef WACCWRTREGISTRYCLIENT_H
#define WACCWRTREGISTRYCLIENT_H

#include <QString>
#include <QVariant>
#include "wacAttributeMap.h"

class RWACRegistryClientSession : public RSessionBase
    {
public: // API
    /**
    * Constructor
    */
    IMPORT_C RWACRegistryClientSession();

    /**
    * Connect to wrtregistry server
    */
    IMPORT_C TInt Connect();

    /**
    * Returns the version
    */
    IMPORT_C TVersion Version() const;

    /**
    * Disconnect from wrtregistry server
    */
    IMPORT_C TInt Disconnect();

    /**
    * Registers the widget
    */
    IMPORT_C bool RegisterWidgetL( const QString& appId, const QString& appTitle,
                                   const QString& appPath, const QString& iconPath,
                                   const AttributeMap& attributes, const QString& type,
                                   unsigned long size, const QString& startPath );

    /**
    * Deregister the widget
    */
    IMPORT_C bool DeRegisterWidgetL( const QString& appId, bool update = false );

    /**
    *  Returns the attribute value for the widget
    */
    IMPORT_C QVariant getAttributeL( const QString& appId,
                                     const QString& attribute,
                                     const QVariant& defaultValue = QVariant() );

    /**
    *  Sets the attribute value for the widget
    */
    IMPORT_C bool setAttributeL(const QString& appId,
                                const QString& attribute,
                                const QVariant& value);

    /**
    * Returns the AppId
    */
    IMPORT_C QString nativeIdToAppIdL( const QString& Id );

private:
    TIpcArgs iMesgArgs;
    };

#endif
