/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_EXAMPLES_COMPLEXTOOLBARCONTROLS_BINARYSREAMHELPERS_H
#define INCLUDED_EXAMPLES_COMPLEXTOOLBARCONTROLS_BINARYSREAMHELPERS_H

#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>

using namespace css;
using namespace css::beans;
using namespace css::io;
using namespace css::uno;
using namespace css::container;
using namespace rtl;
using namespace std;

class BinaryXInputStream
{
    Reference<XInputStream> mxInputStream;
    Reference<XSeekable> mxSeekable;
public:
    BinaryXInputStream(Reference<XInputStream> rInputStream)
        : mxInputStream(rInputStream)
        , mxSeekable(Reference<XSeekable>(rInputStream, UNO_QUERY_THROW))

    { }

    void skip(sal_Int32 nOffset)
    {
        mxSeekable->seek(mxSeekable->getPosition() + nOffset);
    }

    sal_Int64 size()
    {
        return mxSeekable->getLength();
    }

    template<typename T>
    T readValue()
    {
        const sal_uInt32 nBytesToRead = sizeof(T);
        Sequence<sal_Int8> aSequence(nBytesToRead);
        sal_uInt32 nReadBytes = mxInputStream->readBytes(aSequence, nBytesToRead);
        if (nBytesToRead != nReadBytes)
        {
            throw RuntimeException("stream read: value was not read completely");
        }
        T returnValue;
        memcpy(&returnValue, aSequence.getArray(), nReadBytes);
        return returnValue;
    }

    sal_Int64 readInt64()
    {
        return readValue<sal_Int64>();
    }

    sal_Int32 readInt32()
    {
        return readValue<sal_Int32>();
    }

    sal_Int32 readArray(char* pArray, sal_Int32 nArraySize)
    {
        const sal_uInt32 nBytesToRead = sizeof(char) * nArraySize;
        Sequence<sal_Int8> aSequence(nBytesToRead);
        sal_uInt32 nReadBytes = mxInputStream->readBytes(aSequence, nBytesToRead);
        memcpy(pArray, aSequence.getArray(), nReadBytes);
        return nReadBytes;
    }

    OString readCharArray(sal_Int32 nLength)
    {
        Sequence<sal_Int8> aSequence(nLength);
        sal_uInt32 nReadBytes = mxInputStream->readBytes(aSequence, nLength);
        return OString(reinterpret_cast<sal_Char*>(aSequence.getArray()), nReadBytes);
    }

    OUString readUnicodeArray(sal_Int32 nLength)
    {
        Sequence<sal_Int8> aSequence(nLength * 2);
        sal_uInt32 nReadBytes = mxInputStream->readBytes(aSequence, nLength * 2);
        return OUString(reinterpret_cast<sal_Unicode*>(aSequence.getArray()), nReadBytes / 2);
    }
};

class BinaryXOutputStream
{
    Reference<XOutputStream> mxOutputStream;
    Reference<XSeekable> mxSeekable;
public:
    BinaryXOutputStream(Reference<XOutputStream> rOutputStream)
        : mxOutputStream(rOutputStream)
        , mxSeekable(Reference<XSeekable>(rOutputStream, UNO_QUERY))
    { }

    template <typename T>
    void writeValue(T nValue)
    {
        Sequence<sal_Int8> aSequence(sizeof(T));
        memcpy(aSequence.getArray(), &nValue, sizeof(T));
        mxOutputStream->writeBytes(aSequence);
    }

    void writeInt32(sal_Int32 nValue)
    {
        writeValue<sal_Int32>(nValue);
    }

    void writeInt64(sal_Int64 nValue)
    {
        writeValue<sal_Int64>(nValue);
    }

    void writeArray(const char * pArray, size_t nSize)
    {
        Sequence<sal_Int8> aSequence(nSize);
        memcpy(aSequence.getArray(), pArray, nSize);
        mxOutputStream->writeBytes(aSequence);
    }

    void writeUnicodeArray(const OUString & rValue)
    {
        Sequence<sal_Int8> aSequence(rValue.getLength() * 2);
        memcpy(aSequence.getArray(), rValue.getStr(), rValue.getLength() * 2);
        mxOutputStream->writeBytes(aSequence);
    }

    void seek(sal_Int32 nOffset)
    {
        if (mxSeekable.is())
            mxSeekable->seek(nOffset);
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
