#ifndef __SIFNOTIFICATIONMONITOR_H__
#define __SIFNOTIFICATIONMONITOR_H__

#include <usif/sif/sifnotification.h>

class QString;

using namespace Usif;

class SifNotificationMonitor : public MSifOperationsHandler {
 public:
    SifNotificationMonitor();
    ~SifNotificationMonitor();

    void start();

    virtual void StartOperationHandler(TUint aKey, const CSifOperationStartData& aStartData);
    virtual void EndOperationHandler(const CSifOperationEndData& aEndData);
    virtual void ProgressOperationHandler(const CSifOperationProgressData& aProgressData);

 private:
    CSifOperationsNotifier *m_sifOperationsNotifier;
    void Log(QString str, bool replace=false);
    QString HBufCToQString(const HBufC& buf);
};

#endif // __SIFNOTIFICATIONMONITOR_H__
