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
#ifndef EMBEDWIDGETINSTALLER_H_
#define EMBEDWIDGETINSTALLER_H_


#include <e32base.h>
#include <e32property.h>
#include <QEventLoop>
#include <QSettings>
#include "wacSuperWidget.h"
#include "wacwidgetmanagerconstants.h"
#include <apgcli.h>

#if defined(QTWRT_USE_USIF)
#include <usif/scr/scr.h>
#include <usif/sif/sif.h>
#endif

#include "SCRConstants.h"


/**
 * MProcessMonitorObserver
 * Process observer
 */
class MProcessMonitorObserver
{
public:
    /**
     * ProcessDied
     * Notifies the observer about the reason of the termination
     * of the process being monitored.
     */
    virtual void ProcessDied( TInt aReason ) = 0;

};


/**
 * CProcessMonitor
 * Active object to monitor a process
 */
class CProcessMonitor : public CActive
{
public:
    // Cancel and destroy
    ~CProcessMonitor();

    // Two-phased constructor.
    static CProcessMonitor* NewL( RProcess& aProcess, MProcessMonitorObserver &aObserver );

    // Two-phased constructor.
    static CProcessMonitor* NewLC( RProcess& aProcess, MProcessMonitorObserver &aObserver  );

public: // New functions
    /**
     * MonitorProcess
     * Function for making the initial request
     * @param The observer
     */
    void MonitorProcess( );

    /**
     * CancelMonitorProcess()
     * Function for cancelling the monitoring. Should be
     * called before deleting this object.
     */
    void CancelMonitorProcess();
private:
    // C++ constructor
    CProcessMonitor( RProcess& aProcess, MProcessMonitorObserver &aObserver  );

    // Second-phase constructor
    void ConstructL();

    /**
     * Internal function used to call the observer
     */
    void CallObserver( TInt& aReason );

private: // From CActive
    // Handle completion
    void RunL();

    // How to cancel
    void DoCancel();

    // Override to handle leaves from RunL(). Default implementation causes
    // the active scheduler to panic.
    TInt RunError( TInt aError );

private:
    /**
     * m_process, the handle to the process to be monitored
     */
    RProcess& m_process;
    /**
     * m_observer, the observer monitor
     */
    MProcessMonitorObserver& m_observer;

};


class EmbedWidgetInstallerPrivate
{
public:
    EmbedWidgetInstallerPrivate();
    virtual ~EmbedWidgetInstallerPrivate();

    TUint32 getProcUid() const;
    int getSymbianUidFromWidgetUniqueIdL(QString& widgetId);

public:
    // Embedded widget install related variables
    RProperty           m_property;
    TUid                m_widgetComponentIdProperty;
    CProcessMonitor*    m_processMonitor;
    RProcess            m_newProc;
    QEventLoop          m_eLoop;
    WidgetInstallError  m_installError;
    WidgetUninstallError m_uninstallError;

#if defined(QTWRT_USE_USIF)
    Usif::RSoftwareComponentRegistry m_SCR;
#endif

};


class WIDGETUTILS_EXPORT EmbedWidgetInstaller: public QObject, public MProcessMonitorObserver
{
    Q_OBJECT

public:
    EmbedWidgetInstaller();
    virtual ~EmbedWidgetInstaller();
    // The caller of the installL API must pass its own ProcessUId in the third param
    WidgetInstallError installL(QString& bundlePath, QString& widgetId);
    WidgetUninstallError uninstallL(QString& widgetId);
    void ProcessDied(TInt& aReason);

private:
    EmbedWidgetInstallerPrivate* d;
};

#endif /* EMBEDWIDGETINSTALLER_H_ */
