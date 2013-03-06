#include "sifnotificationmonitor.h"
#include <usif/usifcommon.h>
#include <QString>
#include <QFile>

SifNotificationMonitor::SifNotificationMonitor() {
    m_sifOperationsNotifier = 0;

    Log("--- SifNotificationMonitor started ---", true);
}

SifNotificationMonitor::~SifNotificationMonitor() {
    if (m_sifOperationsNotifier)
        delete m_sifOperationsNotifier;
}

void SifNotificationMonitor::start() {
    TRAP_IGNORE(m_sifOperationsNotifier =
                CSifOperationsNotifier::NewL(*this));
}

void SifNotificationMonitor::Log(QString str, bool replace) {
    QIODevice::OpenMode mode = QIODevice::WriteOnly;
    if (!replace)
        mode = mode | QIODevice::Append;

    QFile file("c:\\sifnotification.log");

    file.open(mode);

    file.write(str.toAscii());
    file.write("\n");

    file.close();
}

QString SifNotificationMonitor::HBufCToQString(const HBufC& buf) {
    QString qs = QString::fromUtf16(buf.Ptr(), buf.Length());

    return qs;
}

void SifNotificationMonitor::StartOperationHandler(TUint aKey, const CSifOperationStartData& aStartData) {
    // Ignore start operations for non-widget installs
    if (aStartData.SoftwareType().Compare(KSoftwareTypeWidget) != 0)
        return;

    // Register for progress events for the installation
    TRAP_IGNORE(m_sifOperationsNotifier->SubscribeL(aKey));

    // Log the start of the installation/uninstallation
    switch (aStartData.OperationPhase()) {
    case Usif::EInstalling:
        Log("*** Installation start:");
        break;
    case Usif::EUninstalling:
        Log("*** Uninstallation start:");
        break;
    case Usif::EUpgrading:
        Log("*** Upgrading start:");
        break;
    default:
        break;
    }
    Log(QString("    GlobalComponentId: ") + HBufCToQString(aStartData.GlobalComponentId()));
    Log(QString("        ComponentName: ") + HBufCToQString(aStartData.ComponentName()));
}

void SifNotificationMonitor::EndOperationHandler(const CSifOperationEndData& aEndData) {
    Log("*** Operation complete:");
    Log(QString("    GlobalComponentId: ") + HBufCToQString(aEndData.GlobalComponentId()));
    Log(QString("        ErrorCategory: ") + QString::number((int)aEndData.ErrorCategory()));
    Log(QString("            ErrorCode: ") + QString::number(aEndData.ErrorCode()));
}


void SifNotificationMonitor::ProgressOperationHandler(const CSifOperationProgressData& aProgressData) {
    Log("*** Operation progress:");
    Log(QString("    GlobalComponentId: ") + HBufCToQString(aProgressData.GlobalComponentId()));
    Log(QString("                Phase: ") + QString::number((int)aProgressData.Phase()));
    Log(QString("            Sub-Phase: ") + QString::number((int)aProgressData.SubPhase()));
    Log(QString("             Progress: ") + QString::number(aProgressData.CurrentProgress()) + " / " +
        QString::number(aProgressData.Total()));

}
