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

#ifndef __WIDGETMANAGER_P_H__
#define __WIDGETMANAGER_P_H__

#include "wacWidgetUtils.h"
#include "wacSuperWidget.h"

#ifdef OPTIMIZE_INSTALLER

#include "lifecycle/widgetinformation.h"

#include <QList>
class WidgetContext;

#endif //OPTIMIZE_INSTALLER

class QEventLoop;

class HbAction;
class WidgetProperties;
class W3cXmlPlugin;
class CSifUi;
class CSifUiAppInfo;

namespace Usif {
    class CPublishSifOperationInfo;
}

class WidgetManagerPrivate : public QObject
{
    Q_OBJECT

public:
    enum WidgetManagerPromptButtons {
        WidgetManagerPromptButtonYes,
        WidgetManagerPromptButtonNo,
        WidgetManagerPromptButtonOkay,
        WidgetManagerPromptButtonCancel
    };

public:
    WidgetManagerPrivate(QWidget *parent, WidgetManager*);
    ~WidgetManagerPrivate();

    WidgetProperties* getWidgetProperties(const QString &pkgPath);
    SuperWidget* createWidget(const QString &pkgPath);
    QString setDefaultRootDirectory(const QString& rootDirectory);
#ifdef Q_OS_SYMBIAN
    QString setS60RootDirectory(const QChar& driveLetter = NULL, 
            const QString& rootDirectory = NULL);
#endif  // Q_OS_SYMBIAN
    bool validateDriveLetter(const QString& rootDirectory = NULL);
    bool checkCallerPermissions();
    bool showQuestion(const QString& title,
                      const QString& msg,
                      const WidgetManagerPromptButtons& primaryButton,
                      const WidgetManagerPromptButtons& secondaryButton);
    void showInformation(const QString& title, const QString& msg);
#if defined(QTWRT_USE_USIF) && defined (Q_OS_SYMBIAN)
    bool ShowQuestionL(const QString& msg, const QString& primary, const QString& secondary);
#endif
    void uninstallComplete(const WidgetUninstallError& errorCode);

#ifdef OPTIMIZE_INSTALLER
    int installWidget( const QString& pkgPath, const QMap<QString, QVariant>& configDetails );

    void cancel( int trxId );

#endif // OPTIMIZE_INSTALLER
    
    
#ifdef OPTIMIZE_INSTALLER
#if  defined(Q_OS_SYMBIAN) && defined(USE_NATIVE_DIALOG)
public:
    void loadSymbianResourceFile();
    TInt iResourceFileOffset;

#endif // defined(Q_OS_SYMBIAN) && defined(USE_NATIVE_DIALOG)
#endif //OPTIMIZE_INSTALLER
    

public Q_SLOTS:

#ifdef Q_OS_SYMBIAN
    void handleConfirmation();
#endif

    void handleInstallProgress(int p);
    void handleInstallError(WidgetInstallError error);
    void handleDestinationSelection(unsigned long spaceRequired,
                                    bool allowRemovable);
    void handleWidgetReplacement(QString title);
    void handleFeatureInstallation(QList<QString> capList);
    void handleUntrustedWidgetInstallation(WidgetInstallError errorCode);
    void handleInstallationSuccess();

#ifdef QTWRT_USE_ORBIT
    void informationCallback(HbAction*);
    void confirmCallback(HbAction*);
#endif

#ifdef OPTIMIZE_INSTALLER
    void handleProgress( int percentCompleted, WidgetOperationType operationType );

    void handleCompleted();

    void handleAborted( WidgetErrorType errCode );

    void handleInteractiveRequest( QMap< WidgetPropertyType, QVariant> &properties );
#endif // OPT_INSTALLER


private:
#ifdef OPTIMIZE_INSTALLER    
    int transactionId( const QObject* sender );
#endif // OPTIMIZE_INSTALLER

public:
    void * m_data;
    W3cXmlPlugin *m_w3property;
    WidgetType m_widgetType;
    QString m_contentType;
    SuperWidget* m_installingWidget;
    QString m_rootDirectory;
    QHash<QString, SuperWidget*> m_widgetCache;

    bool m_silent;
    bool m_disableUnsignWidgetSignCheck;
    WAC::InstallationAttributes m_attributes;
    QWidget *m_parent;

#if defined(Q_OS_SYMBIAN) && defined(QTWRT_USE_USIF)
    CSifUi *iSifUi;
    CSifUiAppInfo *iAppInfo;
    unsigned long iAppSize;
    int iProgressValue;
    bool iProgressDisplayed;
    Usif::CPublishSifOperationInfo *iSifProgressPublisher;
    HBufC16 *iSifProgressGlobalComponentId;
#endif

public:
    struct DialogResult {
        bool m_dialogAccepted;
        QString m_resultText;
    };

    QEventLoop* m_eventLoop;
    QString m_acceptText;
    DialogResult m_dialogResult;
    
signals:
#ifdef OPTIMIZE_INSTALLER
    void progress( int transId, int percentCompleted, WidgetOperationType operationType );

    void aborted( int transId, WidgetErrorType errCode );

    void interactiveRequest( int transId, QMap<WidgetPropertyType, QVariant> & );
    
    void completed( int transId );
#endif //OPTIMIZE_INSTALLER    

private:
    WidgetManager* q; // not owned
    
#ifdef OPTIMIZE_INSTALLER
    QList< WidgetContext * > m_WidgetContexts;
#endif //OPTIMIZE_INSTALLER

};

#endif
