/*
 * MessageSender.cpp
 *
 *  Created on: Dec 21, 2010
 *      Author: administrator
 */

#include "messagesender.h"
#include <QString>



HBufC* qt_QString2HBufCNewL(const QString& aString)
{
    HBufC *buffer;
#ifdef QT_NO_UNICODE
    TPtrC8 ptr(reinterpret_cast<const TUint8*>(aString.toLocal8Bit().constData()));
    buffer = HBufC8::NewL(ptr.Length());
    buffer->Des().Copy(ptr);
#else
    TPtrC16 ptr(reinterpret_cast<const TUint16*>(aString.utf16()));
    buffer = HBufC16::NewL(ptr.Length());
    buffer->Des().Copy(ptr);
#endif
    return buffer;
}

MessageSender::MessageSender()
    {
    
    
    }


void MessageSender::sendMessage(const QString& appId, const QString& msg1, const QString& msg2)
{
    TRAP_IGNORE(sendL(appId, msg1, msg2));
}

void MessageSender::sendL(const QString& appId, const QString& msg1, const QString& msg2)
{
    TInt error = KErrNone;

    TUid widgetUid(KNullUid);   
    QString uniqueID = "";
    
    // Find widgetUid & uniqueID from webAppRegistry using user entered appID
    TRAP_IGNORE(widgetUid = getWidgetIdL(appId, uniqueID));

#ifdef QTWRT_USE_USIF
    TBool isWidgetRunning;
    TUid uiUid(KNullUid);
    // Use usif/scr to determine if widget running & get uiUid (for widget UI app)
    TRAP_IGNORE(isWidgetRunning = isWidgetRunningL(uniqueID, uiUid));
    if(!widgetUid.iUid || uniqueID.isEmpty() || !uiUid.iUid )
        return;

#else  // ninetwo
    TUid uiUid = TUid::Uid(0x200267C0); // for widget UI app
    if (!widgetUid.iUid) {
        return;
    }
#endif // ninetwo

    TPtrC16 message(reinterpret_cast<const TUint16*>(msg2.utf16()));
    TInt messageLength = msg2.length();
    HBufC8* b8 = HBufC8::NewLC(messageLength);
    TPtr8 b8Ptr(b8->Des());
    b8Ptr.Copy(message);

    RApaLsSession apaLsSession;
    User::LeaveIfError(apaLsSession.Connect());
    
    TApaAppInfo appInfo;
    User::LeaveIfError(apaLsSession.GetAppInfo(appInfo, widgetUid));
      
    RWsSession wsSession;
    User::LeaveIfError(wsSession.Connect());
    CleanupClosePushL(wsSession);

    TApaTaskList taskList(wsSession);
    TApaTask task = taskList.FindApp(uiUid);

    if (task.Exists() && (QString::compare(msg1,"open")==0)) {
        // 32-bits as base10 is 10 digits range without sign
        const TInt numberLength = 10;
        // message length = characters to represent:
        // version and uid and miniviewOperation
        // plus length of "TailEnd"
        TInt v1charCount =  numberLength * 3;
        // Additional command line params in truncatedCommandLine
        // descriptor Length() is number of chars not bytes
        TInt v2charCount = v1charCount + numberLength
            + messageLength;

        TInt l = (messageLength ? v2charCount : v1charCount);
        HBufC8* message8 = HBufC8::NewLC(l);
        TPtr8 desMessage8(message8->Des());
        RDesWriteStream stream(desMessage8);
        CleanupClosePushL(stream);
        stream.WriteUint32L(2); // version
        stream.WriteUint32L(widgetUid.iUid);
        stream.WriteInt32L(0); // operation open
        stream.WriteInt32L(messageLength);
        stream.WriteL(*b8);
        CleanupStack::PopAndDestroy(); // stream
        const TInt KCustomMessageId = 0x000001;
        TUid messageUid = TUid::Uid(KCustomMessageId);
        task.SendMessage(messageUid, desMessage8);
        task.BringToForeground();
        CleanupStack::PopAndDestroy(); // message8
        
    }else if (QString::compare(msg1,"open")==0){ //incase user passed in erroneous msg2 (or close even).
        CApaCommandLine* commandLine = CApaCommandLine::NewLC();
        commandLine->SetExecutableNameL(appInfo.iFullName);

        // TODO get this from registry
        HBufC* bc = HBufC::NewLC(256);
        unsigned char mainHTML[] = "mainHTML";
        for (unsigned char* p = &mainHTML[0]; *p; p++) {
            bc->Des().Append(TChar(*p));
        }
        commandLine->SetDocumentNameL(*bc);

        commandLine->SetTailEndL(*b8);
        
        TInt err = apaLsSession.StartApp(*commandLine);

        CleanupStack::PopAndDestroy(2); // bc, commandLine  
    }
    
    
    if (task.Exists() && QString::compare(msg1,"close")==0 ) {
        // 32-bits as base10 is 10 digits range without sign
        const TInt numberLength = 10;
        // message length = characters to represent:
        // version and uid and miniviewOperation
        // plus length of "TailEnd"
        TInt v1charCount =  numberLength * 3;
        // Additional command line params in truncatedCommandLine
        // descriptor Length() is number of chars not bytes
        TInt v2charCount = v1charCount + numberLength
            + messageLength;

        TInt l = (messageLength ? v2charCount : v1charCount);
        HBufC8* message8 = HBufC8::NewLC(l);
        TPtr8 desMessage8(message8->Des());
        RDesWriteStream stream(desMessage8);
        CleanupClosePushL(stream);
        
        stream.WriteUint32L(2); // version
        stream.WriteUint32L(widgetUid.iUid);
        stream.WriteInt32L(2); // operation close
        stream.WriteInt32L(messageLength);
        stream.WriteL(*b8);
        
        CleanupStack::PopAndDestroy(); // stream
        const TInt KCustomMessageId = 0x000001;
        TUid messageUid = TUid::Uid(KCustomMessageId);
        task.SendMessage(messageUid, desMessage8);
        
        //task.BringToForeground();  Don't call this in "close" case.

        CleanupStack::PopAndDestroy(); // message8
    }
    CleanupStack::PopAndDestroy(2);
    // wsSession, b8
    
}

TUid MessageSender::getWidgetIdL(const QString &appID, QString &uniqueID) {

    TUid widgetUid(KNullUid);

    WebAppRegistry* myWebAppReg   = WebAppRegistry::instance();
    QList<WebAppInfo>* webAppList = myWebAppReg->registered();

    for (int i = 0; i < webAppList->size(); ++i) {

        WebAppInfo aWebAppInfo = (WebAppInfo) webAppList->at(i);
        QList<W3CElement*> listElements
            = aWebAppInfo.getElement(W3CSettingsKey::WIDGET_ID); //WIDGET_ID is appID;
  
        if(!listElements.isEmpty()){
            W3CElement* firstElement = listElements.first();
            if (firstElement != NULL) {
                if (firstElement->readElementText() == appID)  {
                    widgetUid.iUid = aWebAppInfo.uid();
                    uniqueID = aWebAppInfo.appId();  // appID is unique ID;
                    break;
              }
          }
        }
    }
    return widgetUid;
}

#ifdef QTWRT_USE_USIF

TBool Window::isWidgetRunningL(QString& uniqueID, TUid& uiUid) {
    
    TBool isWidgetRunning = false;
    HBufC *globalId = qt_QString2HBufCNewL(uniqueID);
    CleanupStack::PushL(globalId);
    HBufC *appIDKey = qt_QString2HBufCNewL(SCR_PROP_APPID);
    CleanupStack::PushL(appIDKey);
    HBufC *isActiveKey = qt_QString2HBufCNewL(SCR_PROP_ISACTIVE);
    CleanupStack::PushL(isActiveKey);
    HBufC *isPresentKey = qt_QString2HBufCNewL(SCR_PROP_ISPRESENT);
    CleanupStack::PushL(isPresentKey);
    HBufC *procIDKey = qt_QString2HBufCNewL(SCR_PROP_PROCUID);
    CleanupStack::PushL(procIDKey);

    User::LeaveIfError(m_SCRClient.Connect());

    RArray<TComponentId> aComponentIdList;
    
    // Create & set filter
    CComponentFilter* myFilter = CComponentFilter::NewL();
    myFilter->SetSoftwareTypeL(Usif::KSoftwareTypeWidget);
    myFilter->AddPropertyL(*appIDKey, *globalId);  //appID is unique ID;
     
    m_SCRClient.GetComponentIdsL(aComponentIdList, myFilter);
    
    for(int i = 0; i < aComponentIdList.Count(); i++) {
        TComponentId id = (TComponentId) aComponentIdList[i];
        CComponentEntry* aEntry = CComponentEntry::NewL();
        if (m_SCRClient.GetComponentL(id, *aEntry)) {

            // Get PROCID
            CIntPropertyEntry* procIDPropEntry
                       = (CIntPropertyEntry*) m_SCRClient.GetComponentPropertyL(id, *procIDKey);
            uiUid.iUid = procIDPropEntry->IntValue();

            // Get IsPresent
            CIntPropertyEntry* isPresentPropEntry
                             =  (CIntPropertyEntry*) m_SCRClient.GetComponentPropertyL(id, *isPresentKey);
            if (isPresentPropEntry) {
                // Get IsActive
                CIntPropertyEntry* isActivePropEntry
                                 =  (CIntPropertyEntry*) m_SCRClient.GetComponentPropertyL(id, *isActiveKey);
                if (isActivePropEntry)
                    isWidgetRunning  = (TBool) isActivePropEntry->IntValue();
            }
            break;
        }
    }  
    m_SCRClient.Close();
    
    CleanupStack::PopAndDestroy(procIDKey);
    CleanupStack::PopAndDestroy(isPresentKey);
    CleanupStack::PopAndDestroy(isActiveKey);
    CleanupStack::PopAndDestroy(appIDKey);
    CleanupStack::PopAndDestroy(globalId);

    return isWidgetRunning;
}

#endif  // TENONE
