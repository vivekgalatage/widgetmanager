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

#ifndef CWRTREGISTRYSERVER_H
#define CWRTREGISTRYSERVER_H

// INCLUDES
#include <wacRegistry.h>

// FORWARD DECLARATIONS

// ----------------------------------------------------------------------------------------
// Server's policy here
// ----------------------------------------------------------------------------------------

//Total number of ranges
const TUint widgetRegistryCount = 3;

//Definition of the ranges of IPC numbers
const TInt widgetRegistryRanges[widgetRegistryCount] =
        {
        EOpCodeRegisterWidget,          // 0 ; EOpCodeRegisterWidget and EOpCodeDeRegisterWidget
        EOpCodeDeRegisterWidget + 1,    // 2 4 ; EOpCodeWidgetRegistryDisconnect
        EOpCodeNotSupported
        };

//Policy to implement for each of the above ranges
const TUint8 widgetRegistryElementsIndex[widgetRegistryCount] =
        {
        0,
        1,
        CPolicyServer::ENotSupported  //applies to 3rd range (out of range IPC)
        };

//Specific capability checks
const CPolicyServer::TPolicyElement widgetRegistryElements[] =
        {
        {_INIT_SECURITY_POLICY_C1(ECapabilityWriteDeviceData), CPolicyServer::EFailClient},
        {_INIT_SECURITY_POLICY_C1(ECapabilityReadDeviceData), CPolicyServer::EFailClient}
        };

//Package all the above together into a policy
const CPolicyServer::TPolicy widgetRegistryPolicy =
        {
        CPolicyServer::EAlwaysPass, //Allows clients to connect
        widgetRegistryCount,        //number of ranges
        widgetRegistryRanges,       //ranges array
        widgetRegistryElementsIndex,//elements<->ranges index
        widgetRegistryElements,     //array of elements
        };

// FORWARD DECLARATIONS

/**
*
*  This class defines shut down timer usage for the WRT registry server
*/
class CShutdown : public CTimer
    {

public:
    /**
    * Constructor
    */
    inline CShutdown():CTimer( -1 )
        {
        CActiveScheduler::Add( this );
        }

    /**
    * 2-phase constructor
    */
    inline void ConstructL()
        {
        CTimer::ConstructL();
        }

    /**
    * Start timer
    */
    inline void Start()
        {
        After( KShutdownDelay );
        }
    ~CShutdown()
        {
        }

private:
    /*
    * From CActive, see base class header.
    */
    void RunL();

    };


/**
*
*  This class defines the WRT registry server
*/
class CCwrtRegistryServer : public CPolicyServer
    {
public:

    /**
    * Returns singleton of factory.
    */
    static CServer2* NewLC();

    /**
    * Initialize and run the server.
    */
    static void RunServerL();

    /**
    * Cancel the shutdown timer, the new session is connected
    */
    void AddSession();

    /**
    * Decrement the session counter and start the shutdown timer if the last client
    * has disconnected
    */
    void RemoveSession();

    /**
    * Return session count
    */
    TInt SessionCount(){ return iSessionCount; }

private:
    /**
    * Constructor
    */
    CCwrtRegistryServer();

    /**
    * Destructor.
    */
    ~CCwrtRegistryServer();

    /**
    * 2-phase constructor
    */
    void ConstructL();

    /**
    * From CServer2
    */
    virtual CSession2* NewSessionL(
        const TVersion& aVersion, const RMessage2& aMessage ) const;

private: // Data members
    TInt        iSessionCount;// number of open sessions
    CShutdown   iShutDown;// shut down timer
    };

#endif
