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

#ifndef CWRTREGISTRYSESSION_H
#define CWRTREGISTRYSESSION_H

// INCLUDES

// FORWARD DECLARATIONS

class CCwrtRegistryServer;

class CCwrtRegistryServerSession : public CSession2
    {
public:
    /**
    * Returns singleton of factory.
    * @param aServer CwrtRegistry server
    * @return CCwrtRegistryServerSession object
    */
    static CCwrtRegistryServerSession* NewL(
        CCwrtRegistryServer& aServer,
        const RMessage2& aMessage );

    /**
    * Destructor.
    */
    ~CCwrtRegistryServerSession();

    /**
    * Called by the CServer framework.
    * @return none
    */
    void CreateL();


private:
    /**
    * C++ constructor.
    */
    CCwrtRegistryServerSession();

    /**
    * 2-phased constructor.
    * @param aServer CwrtRegistry server
    * @param aMessage message from client
    * @return none
    */
    void ConstructL( CCwrtRegistryServer& aServer,
        const RMessage2& aMessage );

private:
    /**
    * From CSession2, handles messages from client
    * @param aMessage message from client
    * @return none
    */
    void ServiceL( const RMessage2& aMessage );

    /**
    * From CSession2, handles leaves from ServiceL()
    * @param aMessage message from client
    * @param aError
    * @return none
    */
    void ServiceError(const RMessage2& aMessage,TInt& aError);

private: // Op Codes
    /**
    * Register widget
    * @param aMessage message from client
    * @return Error code
    */
    bool OpRegisterWidgetL( const RMessage2& aMessage );

    /**
    * Deregister widget
    * @param aMessage message from client
    * @return Error code
    */
    bool OpDeRegisterWidgetL( const RMessage2& aMessage );

    /**
    * Returns the attribute value for the widget
    * @param aMessage message from client
    * @return Error code
    */
    TInt OpGetWebAttributeL( const RMessage2& aMessage );

    /**
    * Sets the attribute value for the widget
    * @param aMessage message from client
    * @return Error code
    */
    bool OpSetWebAttributeL( const RMessage2& aMessage );

    /**
    * Returns the appId of the widget
    * @param aMessage message from client
    * @return Error code
    */
    TInt OpNativeIdToAppIdL( const RMessage2& aMessage );

    /**
    * Disconnect from client
    * @param aMessage message from client
    * @return Error code
    */
    TInt OpDisconnect( const RMessage2& aMessage );

    /**
    * Get CwrtRegistry server object as reference
    * @return CwrtRegistry server object
    */
    CCwrtRegistryServer& CwrtRegistryServer();

    };

#endif
