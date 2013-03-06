#include <QApplication>
#include "sifnotificationmonitor.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    SifNotificationMonitor notifier;
    notifier.start();

    app.exec();

    return 0;
}
