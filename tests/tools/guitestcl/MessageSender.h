/*
 * nonguiwindow.h
 *
 *  Created on: Dec 21, 2010
 *      Author: administrator
 */

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

#ifndef MESSAGESENDER_H
#define MESSAGESENDER_H


#include <QDir>

#include <e32std.h>
#include <e32cons.h>
#include <s32mem.h>
#include <bacline.h>
#include <apgcli.h>
#include <apacmdln.h>
#include <APGTASK.H>
#include <wacWebAppRegistry.h>
#include <wacw3csettingskeys.h>

#ifdef QTWRT_USE_USIF

#include <usif/scr/scr.h>
#include <usif/scr/screntries.h>
#include <usif/scr/appreginfo.h>
#include <usif/usiferror.h>
#include <usif/sif/sifutils.h>
#include "../../../src/platform/symbian/SCRConstants.h"
using namespace Usif;

#else

#include <WidgetRegistryClient.h> 

#endif  // QTWRT_USE_USIF TENONE

class MessageSender: public QObject 
{
	Q_OBJECT
	public:
		 MessageSender ();
		 void sendMessage(const QString& appId, const QString& msg1, const QString& msg2);
	private:
     void sendL(const QString& appId, const QString& msg1, const QString& msg2);
     TUid getWidgetIdL(const QString &appID, QString &uniqueID);

#ifdef QTWRT_USE_USIF
  	 TBool isWidgetRunningL(QString &uniqueID, TUid &uiUid);  
  	 RSoftwareComponentRegistry m_SCRClient;
#else
  	 RWidgetRegistryClientSession *rwidgetclientsession; 
  
#endif  //QTWRT_USE_USIF
  
};

#endif
