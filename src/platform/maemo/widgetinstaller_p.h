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


#ifndef WGT_INSTALLER_P_MAEMO_H
#define WGT_INSTALLER_P_MAEMO_H


#include "wacSuperWidget.h"

#include <QString>
#include <QMap>

class PackageUtils;


#if defined(Q_OS_MAEMO6)
#include <QObject>
#include <QMutex>

class PackageManager;
class PackageManagerPendingCallWatcher;
class PackageManagerPendingFetchWatcher;
#endif


class WidgetInstallerPrivate
#if defined(Q_OS_MAEMO6)
    // need to listen to the signal from Package Manager
    : public QObject
{
    Q_OBJECT
#else
{
#endif
    Q_DISABLE_COPY(WidgetInstallerPrivate)

public:
    enum InstallationStatus {
        Success = WidgetInstallSuccess,
        PackageCreationFailed,
        PackageInstallationFailed,
        PackageMovingFailed,
        SecureStorageSetupFailed,
        NotEnoughSpace,
#if defined(Q_OS_MAEMO6)
        OperationCancelled
#endif
    };

    explicit WidgetInstallerPrivate(SuperWidget* widget);
    ~WidgetInstallerPrivate();

    InstallationStatus install(QString source, QString target, QString appId);

    void setInstallationAttributes(WAC::InstallationAttributes attributes);

private:
    bool installPackage();
    bool hasEnoughSpace(const QString& file, const QString& path) const;

#if defined(Q_OS_MAEMO6)
public slots:
    void pkgmgrOperationComplete(const QString& operation, const QString& packageName, const QString& version, const QString& result);
    void pkgmgrDbusError(PackageManagerPendingCallWatcher* watcher);
    void pkgmgrOperationProgress(const QString& operation, const QString& packageName, const QString& version, int percentage);
    void pkgmgrOperationAborted(const QString &operation, const QString &packageidentifier, const QString &version, const QString &reason);
    void pkgmgrDataFetched(const QMap<QString, QVariant>& fetcher);
    void installationCancelled();

private:
    PackageManager* m_packageManager;
    QMutex m_pkgmgrMutex;
    bool m_pkgmgrInstallationSuccess;
    int m_pkgmgrInstallationTime;
    bool m_userCancelled;
    bool m_userInformedWait;
    PackageManagerPendingFetchWatcher* m_fetcher;
    bool fetchPkgmgrState();
#endif

    SuperWidget* m_widget;
    PackageUtils* m_packageUtils;
    QString m_appId;
    QString m_packageName;
    QString m_installationPath;
    bool m_noInstall;

    QString m_outputPath;
    QString m_productId;
};

#endif // WGT_INSTALLER_P_MAEMO_H
