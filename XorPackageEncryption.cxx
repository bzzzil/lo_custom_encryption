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
#include "XorPackageEncryption.h"
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/io/SequenceInputStream.hpp>
#include <com/sun/star/packages/XPackageEncryption.hpp>
#include <com/sun/star/packages/NoEncryptionException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "BinaryStreamHelpers.h"

#include <map>
#include <memory>

using namespace css;
using namespace css::beans;
using namespace css::io;
using namespace css::uno;
using namespace css::container;
using namespace rtl;
using namespace std;

#define XOR_VALUE 127
#define DATASPACE_NAME "XorEncryptedDataSpace"
#define TRANSFORM_NAME "XorEncryptedTransform"

void lcl_getListOfStreams(Reference<XNameContainer>& xOLEStorage, map<OUString, Sequence<sal_Int8>>& aStreams, const OUString& sPrefix)
{
    Sequence<OUString> oElementNames = xOLEStorage->getElementNames();
    for (const auto & sName : oElementNames)
    {
        OUString sStreamFullName = sPrefix.getLength() ? sPrefix + "/" + sName : sName;

        Reference<XInterface> xSubElement(xOLEStorage->getByName(sName), UNO_QUERY);
        Reference<XNameContainer> xSubStorage(xSubElement, UNO_QUERY);
        if (xSubStorage.is())
        {
            lcl_getListOfStreams(xSubStorage, aStreams, sStreamFullName);
        }
        else
        {
            Reference<XInputStream> xStream(xSubElement, UNO_QUERY);
            if (xStream.is())
            {
                BinaryXInputStream aBinaryInputStream(xStream);

                css::uno::Sequence< sal_Int8 > oData;
                sal_Int32 nStreamSize = aBinaryInputStream.size();
                sal_Int32 nReadBytes = xStream->readBytes(oData, nStreamSize);
                assert(nStreamSize == nReadBytes);

                aStreams.insert({ sStreamFullName, oData });
            }
            else
            {
                // TODO Something went wrong
            }
        }
    }
}

Reference<XInputStream> XorPackageEncryption::getStream(const Sequence<NamedValue>& rStreams, const OUString sStreamName)
{
    for (const auto& aStream : rStreams)
    {
        if (aStream.Name == sStreamName)
        {
            Sequence<sal_Int8> aSeq;
            aStream.Value >>= aSeq;
            Reference<XInputStream> aStream(
                SequenceInputStream::createStreamFromSequence(mxContext, aSeq),
                UNO_QUERY_THROW);
            return aStream;
        }
    }
    return nullptr;
}

XorPackageEncryption::XorPackageEncryption(const Reference<XComponentContext>& rxContext)
    : mxContext(rxContext)
{
}

void SAL_CALL XorPackageEncryption::initialize(const Sequence<Any>& rArguments)
{
}

Sequence<OUString> XorPackageEncryption::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence<OUString> lNames(1);
    lNames[0] = OUString(XORENCRYPTEDDATASPACESERVICE_SERVICENAME);
    return lNames;
}

sal_Bool SAL_CALL XorPackageEncryption::supportsService(const OUString& sServiceName) throw (RuntimeException)
{
    if (sServiceName == XORENCRYPTEDDATASPACESERVICE_SERVICENAME)
        return true;
    else
        return false;
}

OUString SAL_CALL XorPackageEncryption::getImplementationName() throw (RuntimeException)
{
    return OUString(XORENCRYPTEDDATASPACESERVICE_IMPLEMENTATIONNAME);
}


sal_Bool SAL_CALL XorPackageEncryption::checkDataIntegrity()
{
    return true;
}

sal_Bool XorPackageEncryption::decrypt(const Reference<XInputStream>& rxInputStream, Reference<XOutputStream>& rxOutputStream)
{
    BinaryXInputStream aInputStream(rxInputStream);
    BinaryXOutputStream aOutputStream(rxOutputStream);

    aInputStream.readInt64(); // Skip stream size

    for (int i = 0; i < aInputStream.size() - sizeof(sal_Int64); i++)
    {
        sal_Char byte = aInputStream.readValue<sal_Char>();
        byte ^= XOR_VALUE;
        aOutputStream.writeValue<sal_Char>(byte);
    }

    rxOutputStream->flush();

    return true;
}

Sequence<NamedValue> XorPackageEncryption::createEncryptionData(const OUString& /*rPassword*/)
{
	// Here we need to ensure that we can really to decrypt this doc and than set the CryptoType
	// Otherwise return emty sequence
    Sequence<NamedValue> aResult(1);
    aResult[0] = NamedValue("CryptoType", makeAny(OUString("XorEncryptedDataSpace")));
    return aResult;
}

sal_Bool XorPackageEncryption::readEncryptionInfo(const Sequence<NamedValue>& aStreams)
{
    return true;
}

sal_Bool XorPackageEncryption::setupEncryption(const Sequence<NamedValue>& rMediaEncData)
{
    return true;
}

Reference<XSequenceOutputStream> XorPackageEncryption::createStreamDataSpacesDataSpaceMap()
{
    Reference<XOutputStream> xStream(
        mxContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.io.SequenceOutputStream", mxContext),
        UNO_QUERY);
    BinaryXOutputStream aStream(xStream);

    aStream.writeInt32(8); // Header length
    aStream.writeInt32(1); // Entries count

    OUString sReferenceComponent("EncryptedPackage");

    aStream.writeInt32(0x60); // Length
    aStream.writeInt32(1); // References count
    aStream.writeInt32(0); // References component type

    aStream.writeInt32(sReferenceComponent.getLength() * 2);
    aStream.writeUnicodeArray(sReferenceComponent);
    for (int i = 0; i < sReferenceComponent.getLength() * 2 % 4; i++) // Padding
    {
        aStream.writeValue<sal_Char>(0);
    }

    OUString sDataSpaceName(DATASPACE_NAME);
    aStream.writeInt32(sDataSpaceName.getLength() * 2);
    aStream.writeUnicodeArray(sDataSpaceName);
    for (int i = 0; i < sDataSpaceName.getLength() * 2 % 4; i++) // Padding
    {
        aStream.writeValue<sal_Char>(0);
    }

    xStream->flush();

    Reference<XSequenceOutputStream> xSequence(xStream, UNO_QUERY);
    return xSequence;
}

Reference<XSequenceOutputStream> XorPackageEncryption::createStreamDataSpacesDataSpaceInfo()
{
    Reference<XOutputStream> xStream(
        mxContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.io.SequenceOutputStream", mxContext),
        UNO_QUERY);
    BinaryXOutputStream aStream(xStream);

    aStream.writeInt32(0x08); // Header length
    aStream.writeInt32(1); // Entries count

    OUString sTransformName(TRANSFORM_NAME);
    aStream.writeInt32(sTransformName.getLength() * 2);
    aStream.writeUnicodeArray(sTransformName);
    for (int i = 0; i < sTransformName.getLength() * 2 % 4; i++) // Padding
    {
        aStream.writeValue<sal_Char>(0);
    }

    xStream->flush();

    Reference<XSequenceOutputStream> xSequence(xStream, UNO_QUERY);
    return xSequence;
}

Reference<XSequenceOutputStream> XorPackageEncryption::createStreamDataSpacesTransformInfo()
{
    // Write 0x6DataSpaces/TransformInfo/[transformname]
    Reference<XOutputStream> xStream(
        mxContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.io.SequenceOutputStream", mxContext),
        UNO_QUERY);
    BinaryXOutputStream aStream(xStream);
    OUString sTransformId("{C73DFACD-061F-43B0-8B64-AC620D2A8B50}");

    // MS-OFFCRYPTO 2.1.8: TransformInfoHeader
    sal_uInt32 nLength
        = sTransformId.getLength() * 2 + ((4 - (sTransformId.getLength() & 3)) & 3) + 10;
    aStream.writeInt32(nLength); // TransformLength, will be written later
    aStream.writeInt32(1); // TransformType

    // TransformId
    aStream.writeInt32(sTransformId.getLength() * 2);
    aStream.writeUnicodeArray(sTransformId);
    for (int i = 0; i < sTransformId.getLength() * 2 % 4; i++) // Padding
    {
        aStream.writeValue<sal_Char>(0);
    }

    // TransformName
    OUString sTransformInfoName("Microsoft.Metadata.XorTransform");
    aStream.writeInt32(sTransformInfoName.getLength() * 2);
    aStream.writeUnicodeArray(sTransformInfoName);
    for (int i = 0; i < sTransformInfoName.getLength() * 2 % 4; i++) // Padding
    {
        aStream.writeValue<sal_Char>(0);
    }

    aStream.writeInt32(1); // ReaderVersion
    aStream.writeInt32(1); // UpdateVersion
    aStream.writeInt32(1); // WriterVersion

    aStream.writeInt32(4); // Extensibility Header
    xStream->flush();

    Reference<XSequenceOutputStream> xSequence(xStream, UNO_QUERY);
    return xSequence;
}

Reference<XSequenceOutputStream> XorPackageEncryption::createStreamDataSpacesVersion()
{
    Reference<XOutputStream> xStream(mxContext->getServiceManager()->createInstanceWithContext(
        "com.sun.star.io.SequenceOutputStream", mxContext),
        UNO_QUERY);
    BinaryXOutputStream aStream(xStream);

    OUString sFeatureIdentifier("Microsoft.Container.DataSpaces");
    aStream.writeInt32(sFeatureIdentifier.getLength() * 2);
    aStream.writeUnicodeArray(sFeatureIdentifier);
    for (int i = 0; i < sFeatureIdentifier.getLength() * 2 % 4; i++) // Padding
    {
        aStream.writeValue<sal_Char>(0);
    }

    aStream.writeInt32(1); // Reader version
    aStream.writeInt32(1); // Updater version
    aStream.writeInt32(1); // Writer version

    xStream->flush();

    Reference<XSequenceOutputStream> xSequence(xStream, UNO_QUERY);
    return xSequence;
}

Sequence<NamedValue> XorPackageEncryption::encrypt(const Reference<XInputStream>& rxInputStream)
{
    // Store all streams into sequence and return back
    Sequence<NamedValue> aStreams(5);

    // Some MS specific streams sued in real encryption types. Create them like real
    aStreams[0] = NamedValue("\006DataSpaces/DataSpaceMap", 
        makeAny(createStreamDataSpacesDataSpaceMap()->getWrittenBytes()));

    aStreams[1] = NamedValue("\006DataSpaces/Version", 
        makeAny(createStreamDataSpacesVersion()->getWrittenBytes()));

    OUString sStreamName = "\006DataSpaces/DataSpaceInfo/" + OUString(DATASPACE_NAME);
    aStreams[2] = NamedValue(sStreamName, 
        makeAny(createStreamDataSpacesDataSpaceInfo()->getWrittenBytes()));

    sStreamName = "\006DataSpaces/TransformInfo/" + OUString(TRANSFORM_NAME) + "/\006Primary";
    aStreams[3] = NamedValue(sStreamName, 
        makeAny(createStreamDataSpacesTransformInfo()->getWrittenBytes()));

    // Create EncryptedPackage
    BinaryXInputStream aInputStream(rxInputStream);
    Reference<XOutputStream> xEncryptedPackage(mxContext->getServiceManager()->createInstanceWithContext(
        "com.sun.star.io.SequenceOutputStream", mxContext),
        UNO_QUERY);

    BinaryXOutputStream aEncryptedPackage(xEncryptedPackage);
    aEncryptedPackage.writeInt64(aInputStream.size()); // Stream size

    // "Very serious encryption" by itself
    for (int i=0;i<aInputStream.size();i++)
    {
        sal_Char byte = aInputStream.readValue<sal_Char>();
        byte ^= XOR_VALUE;
        aEncryptedPackage.writeValue<sal_Char>(byte);
    }

    xEncryptedPackage->flush();
    Reference<XSequenceOutputStream> xEncryptedPackageSequence(xEncryptedPackage, UNO_QUERY);

    aStreams[4] = NamedValue("EncryptedPackage", makeAny(xEncryptedPackageSequence->getWrittenBytes()));

    return aStreams;
}

sal_Bool XorPackageEncryption::generateEncryptionKey(const OUString& /*password*/)
{
    return true;
}

Reference< XInterface > SAL_CALL XorEncryptedDataSpaceService_createInstance(const Reference< XComponentContext > & rxContext) throw(Exception)
{
    return (cppu::OWeakObject*) new XorPackageEncryption(rxContext);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
