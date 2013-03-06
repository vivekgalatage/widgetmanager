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

#include <s32strm.h>
#include "wacRegistry.h"
#include "CwrtRegistryData.h"

// ============================================================================
// SerializeStringL()
// Writes a string into aStream
//
// ============================================================================
//
void SerializeStringL( RWriteStream& aStream, const QString& aString )
    {
    aStream.WriteInt32L( 3 );
    aStream.WriteL( _L("str"), 3 );
    aStream.WriteInt32L( aString.size() );

    TPtrC16 stringSymbian(reinterpret_cast<const TUint16*>(aString.constData()), aString.size());
    aStream.WriteL( stringSymbian );
    }

// ============================================================================
// SerializeMapL()
// Writes a string into aStream
//
// ============================================================================
//
void SerializeMapL( RWriteStream& aStream, const AttributeMap& aMap )
    {
    aStream.WriteInt32L( 3 );
    aStream.WriteL( _L("map"), 3 );
    aStream.WriteInt32L( aMap.size() );
    AttributeMap::const_iterator i = aMap.constBegin();
    while (i != aMap.constEnd())
        {
        // first serialize the key (a string) and then the value (a variant)
        SerializeStringL( aStream, i.key() );
        QVariant val = i.value();
        switch ( val.type() )
            {
            case QVariant::String:
                SerializeStringL( aStream, val.toString() );
                break;
            case QVariant::Int:
            case QVariant::UInt:
                SerializeIntL( aStream, val.toInt() );
                break;
            case QVariant::Bool:
                SerializeBoolL( aStream, val.toBool() );
                break;
            default:
                break;
            }
        ++i;
        }
    }

// ============================================================================
// SerializeIntL()
// Writes an integer into aStream
//
// ============================================================================
//
void SerializeIntL( RWriteStream& aStream, const TInt& aInt )
    {
    aStream.WriteInt32L( 3 );
    aStream.WriteL( _L("int"), 3 );
    aStream.WriteInt32L( aInt );
    }

// ============================================================================
// SerializeBoolL()
// Writes a boolean into aStream
//
// ============================================================================
//
void SerializeBoolL( RWriteStream& aStream, const TBool& aBool )
    {
    aStream.WriteInt32L( 4 );
    aStream.WriteL( _L("bool"), 4 );
    aStream.WriteInt32L( aBool );
    }

// ============================================================================
// DeserializeStringL()
// Read string from aStream
//
// ============================================================================
//
QString DeserializeStringL( RReadStream& aStream )
    {
    TInt len = 0;
    TBuf<512> tempBuf;

    len = aStream.ReadInt32L();
    aStream.ReadL( tempBuf, len );  // contains "str"

    len = aStream.ReadInt32L();
    aStream.ReadL( tempBuf, len );

    QString qStr( reinterpret_cast<const QChar*>(tempBuf.Ptr()), tempBuf.Length() );

    return qStr;
    }

// ============================================================================
// DeserializeStringL()
// Read string (without the "str" prefix) from aStream
//
// ============================================================================
//
QString DeserializeStringWithoutPrefixL( RReadStream& aStream )
    {
    TInt len = 0;
    TBuf<512> tempBuf;

    len = aStream.ReadInt32L();
    aStream.ReadL( tempBuf, len );

    QString qStr( reinterpret_cast<const QChar*>(tempBuf.Ptr()), tempBuf.Length() );

    return qStr;
    }

// ============================================================================
// DeserializeMapL()
// Read map from aStream
//
// ============================================================================
//
void DeserializeMapL( RReadStream& aStream, AttributeMap& aMap )
    {
    TInt size = 0, len = 0;
    TBuf<10> tempBuf;

    len = aStream.ReadInt32L();
    aStream.ReadL( tempBuf, len );  // contains "map"

    // number of elements in the map
    size = aStream.ReadInt32L();
    for ( int i = 0; i < size; i++ )
        {
        // key is a QString
        QString key = DeserializeStringL( aStream );
        // value is a QVariant
        len = aStream.ReadInt32L();
        aStream.ReadL( tempBuf, len );  // contains "str" or "int" or "bool"
        if ( 0 == tempBuf.Compare( _L("str") ) )
            {
            QString valueStr = DeserializeStringWithoutPrefixL( aStream );
            aMap[key] = QVariant( valueStr );
            }
        else if ( 0 == tempBuf.Compare( _L("int") ) )
            {
            int valueInt = aStream.ReadInt32L();
            aMap[key] = QVariant( valueInt );
            }
        else if ( 0 == tempBuf.Compare( _L("bool") ) )
            {
            bool valueBool = aStream.ReadInt32L();
            aMap[key] = QVariant( valueBool );
            }
        }

    }

// ============================================================================
// DeserializeIntL()
// Read int from aStream
//
// ============================================================================
//
int DeserializeIntL( RReadStream& aStream )
    {
    TInt len = 0;
    TBuf<10> tempBuf;

    len = aStream.ReadInt32L();
    aStream.ReadL( tempBuf, len );  // contains "int"

    int aInt = aStream.ReadInt32L();
    return aInt;
    }
