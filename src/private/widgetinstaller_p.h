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

#ifndef WIDGETINSTALLER_P_H_
#define WIDGETINSTALLER_P_H_

class SuperWidget;
namespace WAC {
    class Storage;
}

/**
 * Interface for handling W3C Widget installation.
 */
class WidgetInstallerPrivate {

public:

    explicit WidgetInstallerPrivate(SuperWidget * webWidget, WAC::Storage * storage);
    ~WidgetInstallerPrivate();

    bool cpDir( const QDir&, const QDir&, const bool force=false,
                const bool secure=
#ifdef CWRT_WIDGET_FILES_IN_SECURE_STORAGE
                true
#else
                false
#endif
        );

    bool renameDir(const QDir&, const QDir&);



private:

#ifdef Q_OS_SYMBIAN
    void moveDirL(const QString& srcPath,const QString& dstPath);
#endif

private:
    SuperWidget * m_webWidget;
    WAC::Storage * m_storage;
    bool m_firstCopy;

    Q_DISABLE_COPY(WidgetInstallerPrivate)

};

#endif /* WIDGETINSTALLER_P_H_ */
