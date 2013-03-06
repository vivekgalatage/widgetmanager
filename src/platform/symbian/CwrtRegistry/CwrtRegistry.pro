# Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
#
# This file is part of Qt Web Runtime.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# version 2.1 as published by the Free Software Foundation.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
#
#

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    Server \
    Client

symbian {
    # Export headers on symbian
    BLD_INF_RULES.prj_exports += \
        "./Client/wacRegistryClient.h           $$MW_LAYER_PUBLIC_EXPORT_PATH(wac/wacRegistryClient.h)" \
        "./common/wacRegistry.h                 $$MW_LAYER_PUBLIC_EXPORT_PATH(wac/wacRegistry.h)"
}
