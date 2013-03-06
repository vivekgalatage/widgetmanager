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

#ifndef WACCWRTREGISTRY_H
#define WACCWRTREGISTRY_H

#include <e32cmn.h>

const TUid KWACRegistryServerUid = { 0x2003DE23 };

const TInt KWACRegistryClientVersionMajor = 0;
const TInt KWACRegistryClientVersionMinor = 1;
const TInt KWACRegistryClientVersionBuild = 1;
const TInt KWACRegistryServerStartupAttempts = 2;
const TInt KWACRegistryServerAsynchronousSlotCount = 4;
const TInt KShutdownDelay = 200000;
const TInt KWACRegistryVal = KMaxFileName;

_LIT( KWACRegistryName, "!wacRegistry" ); // name to connect to
_LIT( KWACRegistryImage, "wacRegistry" ); // DLL/EXE name

// Enumerations
enum TWACRegistryClientOpCodes
    {
    EOpCodeRegisterWidget = 0,
    EOpCodeDeRegisterWidget = 1,
    EOpCodeGetWebAttribute,
    EOpCodeSetWebAttribute,
    EOpCodeNativeIdToAppId,
    EOpCodeWidgetRegistryDisconnect,
    EOpCodeNotSupported // must be last
    };

#endif
