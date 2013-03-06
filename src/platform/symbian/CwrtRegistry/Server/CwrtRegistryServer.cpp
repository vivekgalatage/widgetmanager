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

#include <e32base.h>
#include <w32std.h>
#include <apgcli.h>
#include "CwrtRegistryServer.h"
#include "CwrtRegistrySession.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCwrtRegistryServer::CCwrtRegistryServer
// C++ default constructor can NOT contain any code, that
// might leave.
//
// -----------------------------------------------------------------------------
//
CCwrtRegistryServer::CCwrtRegistryServer() :
    CPolicyServer(0,widgetRegistryPolicy,ESharableSessions)
    {
    }

// -----------------------------------------------------------------------------
// CCwrtRegistryServer::ConstructL
// Symbian 2nd phase constructor can leave.
//
// -----------------------------------------------------------------------------
//
void CCwrtRegistryServer::ConstructL()
    {
    StartL( KWACRegistryName );
    iShutDown.ConstructL();
    iShutDown.Start();
    }

CCwrtRegistryServer::~CCwrtRegistryServer()
    {
    }
// -----------------------------------------------------------------------------
// CCwrtRegistryServer::NewL
// Two-phased constructor.
//
// -----------------------------------------------------------------------------
//
 CServer2* CCwrtRegistryServer::NewLC()
    {
    CCwrtRegistryServer* self = new ( ELeave ) CCwrtRegistryServer;
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CCwrtRegistryServer::NewSessionL
// Creates a new widgetregistry server session
//
// -----------------------------------------------------------------------------
//
CSession2* CCwrtRegistryServer::NewSessionL(
    const TVersion& /*aVersion*/,
    const RMessage2& aMessage ) const
    {
    CCwrtRegistryServerSession* session = CCwrtRegistryServerSession::NewL(
        const_cast<CCwrtRegistryServer&>( *this ),aMessage );
    return session;
    }


// -----------------------------------------------------------------------------
// CCwrtRegistryServer::AddSession
// Cancel the shutdown timer now, the new session is connected
//
// -----------------------------------------------------------------------------
//
void CCwrtRegistryServer::AddSession()
    {
    ++iSessionCount;
    iShutDown.Cancel();  // Cancel any outstanding shutdown timer
    }

// -----------------------------------------------------------------------------
// CCwrtRegistryServer::RemoveSession
// Decrement the session counter and start the shutdown timer if the last client
// has disconnected
// -----------------------------------------------------------------------------
//
void CCwrtRegistryServer::RemoveSession()
    {
    if ( --iSessionCount == 0 )
        {
        iShutDown.Start();
        }
    }

// -----------------------------------------------------------------------------
// CCwrtRegistryServer::RunServerL
// Initialize and run the server
//
// -----------------------------------------------------------------------------
//
void CCwrtRegistryServer::RunServerL()
    {
    // First create and install the active scheduler
    CActiveScheduler* scheduler = new ( ELeave ) CActiveScheduler;
    CleanupStack::PushL(scheduler);

    CActiveScheduler::Install(scheduler);

    // create the server
    CCwrtRegistryServer::NewLC();

    // Naming the server thread after the server helps to debug panics
    User::LeaveIfError( RThread().RenameMe( KWACRegistryName ) );

    RProcess::Rendezvous(KErrNone);

    // Enter the wait loop
    CActiveScheduler::Start();

    // Exited - cleanup the server and scheduler

    CleanupStack::PopAndDestroy( 2, scheduler );
    }

// -----------------------------------------------------------------------------
// Server process entry-point
//
// -----------------------------------------------------------------------------
//
TInt E32Main()
    {
//    __UHEAP_MARK; // Heap checking

    CTrapCleanup* cleanup=CTrapCleanup::New();
    TInt r = KErrNoMemory;

    if ( cleanup )
        {
        TRAP( r, CCwrtRegistryServer::RunServerL() );
        delete cleanup;
        }

//    __UHEAP_MARKEND;
    return r;
    }

// -----------------------------------------------------------------------------
// CShutdown::RunServerL
// Initiates server exit when the timer expires
//
// -----------------------------------------------------------------------------
//
void CShutdown::RunL()
    {
    CActiveScheduler::Stop();
    }

// End of File
