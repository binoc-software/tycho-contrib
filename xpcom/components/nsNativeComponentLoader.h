/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1999 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#include "nsISupports.h"
#include "nsIRegistry.h"
#include "nsIComponentLoader.h"
#include "nsIComponentManager.h"
#include "nsIFileSpec.h"
#include "nsIRegistry.h"
#include "nsSpecialSystemDirectory.h"
#include "nsCOMPtr.h"
#include "nsHashtable.h"
#include "nsVoidArray.h"
#include "xcDll.h"

#ifndef nsNativeComponentLoader_h__
#define nsNativeComponentLoader_h__

class nsNativeComponentLoader : public nsIComponentLoader {

 public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSICOMPONENTLOADER

    nsNativeComponentLoader();
    virtual ~nsNativeComponentLoader();
    nsresult RegistryNameForLib(const char *aLibName,
                                char **aRegistryName);
    nsresult RegistryNameForSpec(nsIFileSpec *aSpec,
                                 char **aRegistryName);

 protected:
    nsCOMPtr<nsIRegistry> mRegistry;
    nsIComponentManager* mCompMgr;      // weak reference -- backpointer
    nsObjectHashtable*  mDllStore;
    NS_IMETHOD RegisterComponentsInDir(PRInt32 when, nsIFileSpec *dir);
    nsRegistryKey mXPCOMKey;
    nsVoidArray mDeferredComponents;

 private:
    nsresult CreateDll(nsIFileSpec *aSpec, const char *aLocation,
                       PRUint32 modifiedTime, PRUint32 fileSize, nsDll **aDll);
    nsresult SelfRegisterDll(nsDll *dll, const char *registryLocation,
                             PRBool deferred);
    nsresult SelfUnregisterDll(nsDll *dll);
    nsresult GetRegistryDllInfo(const char *aLocation, PRUint32 *lastModifiedTime,
                                PRUint32 *fileSize);
    nsresult GetRegistryDllInfo(nsRegistryKey key, PRUint32 *lastModifiedTime,
                                PRUint32 *fileSize);
    nsresult SetRegistryDllInfo(const char *aLocation, PRUint32 lastModifiedTime,
                                PRUint32 fileSize);
    nsresult GetFactoryFromModule(nsDll *aDll, const nsCID &aCID,
                                  nsIFactory **aFactory);
    /* obsolete! already! */
    nsresult GetFactoryFromNSGetFactory(nsDll *aDlll, const nsCID &aCID,
                                        nsIServiceManager *aServMgr,
                                        nsIFactory **aFactory);


    nsresult DumpLoadError(nsDll *dll, 
                           const char *aCallerName,
                           const char *aNsprErrorMsg);
};

#endif /* nsNativeComponentLoader_h__ */
