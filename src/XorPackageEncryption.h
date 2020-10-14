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

#ifndef XORENCRYPTEDDATASPACESERVICE_H
#define XORENCRYPTEDDATASPACESERVICE_H

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/packages/XPackageEncryption.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/io/XSequenceOutputStream.hpp>

#define XORENCRYPTEDDATASPACESERVICE_IMPLEMENTATIONNAME "com.sun.star.comp.oox.crypto.IMPL.XorEncryptedDataSpace"
#define XORENCRYPTEDDATASPACESERVICE_SERVICENAME "com.sun.star.comp.oox.crypto.XorEncryptedDataSpace"

using namespace css;
using namespace css::beans;
using namespace css::io;
using namespace css::uno;
using namespace rtl;

class XorPackageEncryption : public ::cppu::WeakImplHelper3 <css::lang::XInitialization,
                                                  css::lang::XServiceInfo,
                                                  css::packages::XPackageEncryption>
{
    uno::Reference<uno::XComponentContext> mxContext;
    uno::Reference<io::XInputStream> getStream(const Sequence<NamedValue>& rStreams, const rtl::OUString sStreamName);
public:
    XorPackageEncryption(const Reference<XComponentContext>& rxContext);

     // XInitialization
     virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rArguments) override;

    // XServiceInfo
    ::rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException) override;
    virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& sServiceName)
        throw (css::uno::RuntimeException) override;
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException) override;

     // XPackageEncryption
    sal_Bool SAL_CALL checkDataIntegrity() override;
    sal_Bool SAL_CALL decrypt(const Reference<XInputStream>& rxInputStream,
        Reference<XOutputStream>& rxOutputStream) override;
    Sequence<NamedValue> SAL_CALL createEncryptionData(const rtl::OUString& /*rPassword*/) override;
    sal_Bool SAL_CALL readEncryptionInfo(const Sequence<NamedValue>& aStreams) override;
    sal_Bool SAL_CALL setupEncryption(const Sequence<NamedValue>& rMediaEncData) override;
    Sequence<NamedValue> SAL_CALL encrypt(const Reference<XInputStream>& rxInputStream) override;
    sal_Bool SAL_CALL generateEncryptionKey(const rtl::OUString& /*password*/) override;
private:
    Reference<XSequenceOutputStream> createStreamDataSpacesDataSpaceMap();
    Reference<XSequenceOutputStream> createStreamDataSpacesDataSpaceInfo();
    Reference<XSequenceOutputStream> createStreamDataSpacesTransformInfo();
    Reference<XSequenceOutputStream> createStreamDataSpacesVersion();
};

Reference<XInterface> SAL_CALL XorEncryptedDataSpaceService_createInstance(const Reference<XComponentContext> & rxContext)
throw (css::uno::Exception);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
