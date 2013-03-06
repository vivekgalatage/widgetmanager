#ifndef WIDGETLOG_H
#define WIDGETLOG_H

#ifdef ENABLE_LOG
#include <QDebug>
#define LOGIT(param) qDebug() << "OPTINSTALLER:" << param;
#else // !ENABLE_LOG
#define LOGIT(param)
#endif // ENABLE_LOG

#endif // WIDGETLOG_H
