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

#ifndef __WIDGETREGISTRATIONS60APPARC_H__
#define __WIDGETREGISTRATIONS60APPARC_H__

// INCLUDES
#include <e32base.h>
#include <f32file.h>

namespace SwiUI
{
/**
* This class handles registration and deregistration during widget
* installation.
*
* @lib WidgetInstallerUI
* @since 3.1
*/
class CWidgetRegistrationS60Apparc: public CBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CWidgetRegistrationS60Apparc* NewL( RFs& aFs );

        /**
        * Destructor.
        */
        ~CWidgetRegistrationS60Apparc();

    public: // new functions

        /**
        * Register installed widget as non native app
        * @since 3.1
        * @param aWidgetEntry: A widget entry to be registered
        * return void
        */
        void RegisterWidgetL( const TDesC& aMainHTML,
                              const TDesC& aBundleDisplayName,
                              const TDesC& aIconPath,
                              const TDesC& aDriveName,
                              const TUid& aUid,
                              bool hideIcon);

        /**
        * deregister installed widget as non native app
        * @since 3.1
        * @param aUid - the UID of widget to be unisntalled
        * @return void
        */
        void DeregisterWidgetL( const TUid& aUid );

        /**
        * Send the command to widget launcher to close the widget if it is open.
        * @since 8.x
        * @param aUid - the UID of widget to be closed
        * @return void
        */
        void closeWidgetL( const TUid& aUid );

    protected:  // constructors

        /**
        * C++ Constructor.
        */
        CWidgetRegistrationS60Apparc( RFs& aFs );

        /**
        * 2nd phase constructor.
        */
        void ConstructL();

    private:

       RFs              iFs; //Not owned
    };
}

#endif
