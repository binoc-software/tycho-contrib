/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_FileService_h
#define mozilla_dom_FileService_h

#include "mozilla/Attributes.h"
#include "mozilla/dom/FileHelper.h"
#include "nsClassHashtable.h"
#include "nsIObserver.h"
#include "nsAutoPtr.h"
#include "nsCOMPtr.h"
#include "nsDebug.h"
#include "nsHashKeys.h"
#include "nsTArray.h"
#include "nsTHashtable.h"

class nsAString;
class nsIEventTarget;
class nsIOfflineStorage;
class nsIRunnable;

namespace mozilla {
namespace dom {

class FileHandleBase;

class FileService final : public nsIObserver
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIOBSERVER

  // Returns a non-owning reference!
  static FileService*
  GetOrCreate();

  // Returns a non-owning reference!
  static FileService*
  Get();

  static void
  Shutdown();

  // Returns true if we've begun the shutdown process.
  static bool
  IsShuttingDown();

  nsresult
  Enqueue(FileHandleBase* aFileHandle, FileHelper* aFileHelper);

  void
  NotifyFileHandleCompleted(FileHandleBase* aFileHandle);

  void
  WaitForStoragesToComplete(nsTArray<nsCOMPtr<nsIOfflineStorage> >& aStorages,
                            nsIRunnable* aCallback);

  void
  AbortFileHandlesForStorage(nsIOfflineStorage* aStorage);

  nsIEventTarget*
  StreamTransportTarget()
  {
    NS_ASSERTION(mStreamTransportTarget, "This should never be null!");
    return mStreamTransportTarget;
  }

private:
  class FileHandleQueue final : public FileHelperListener
  {
    friend class FileService;

  public:
    NS_IMETHOD_(MozExternalRefCountType)
    AddRef() override;

    NS_IMETHOD_(MozExternalRefCountType)
    Release() override;

    inline nsresult
    Enqueue(FileHelper* aFileHelper);

    virtual void
    OnFileHelperComplete(FileHelper* aFileHelper) override;

  private:
    inline
    explicit FileHandleQueue(FileHandleBase* aFileHandle);

    ~FileHandleQueue();

    nsresult
    ProcessQueue();

    ThreadSafeAutoRefCnt mRefCnt;
    NS_DECL_OWNINGTHREAD
    nsRefPtr<FileHandleBase> mFileHandle;
    nsTArray<nsRefPtr<FileHelper> > mQueue;
    nsRefPtr<FileHelper> mCurrentHelper;
  };

  struct DelayedEnqueueInfo
  {
    DelayedEnqueueInfo();
    ~DelayedEnqueueInfo();

    nsRefPtr<FileHandleBase> mFileHandle;
    nsRefPtr<FileHelper> mFileHelper;
  };

  class StorageInfo
  {
    friend class FileService;

  public:
    inline FileHandleQueue*
    CreateFileHandleQueue(FileHandleBase* aFileHandle);

    inline FileHandleQueue*
    GetFileHandleQueue(FileHandleBase* aFileHandle);

    void
    RemoveFileHandleQueue(FileHandleBase* aFileHandle);

    bool
    HasRunningFileHandles()
    {
      return !mFileHandleQueues.IsEmpty();
    }

    inline DelayedEnqueueInfo*
    CreateDelayedEnqueueInfo(FileHandleBase* aFileHandle,
                             FileHelper* aFileHelper);

    inline void
    CollectRunningAndDelayedFileHandles(
                              nsIOfflineStorage* aStorage,
                              nsTArray<nsRefPtr<FileHandleBase>>& aFileHandles);

    void
    LockFileForReading(const nsAString& aFileName)
    {
      mFilesReading.PutEntry(aFileName);
    }

    void
    LockFileForWriting(const nsAString& aFileName)
    {
      mFilesWriting.PutEntry(aFileName);
    }

    bool
    IsFileLockedForReading(const nsAString& aFileName)
    {
      return mFilesReading.Contains(aFileName);
    }

    bool
    IsFileLockedForWriting(const nsAString& aFileName)
    {
      return mFilesWriting.Contains(aFileName);
    }

  private:
    StorageInfo()
    {
    }

    nsTArray<nsRefPtr<FileHandleQueue>> mFileHandleQueues;
    nsTArray<DelayedEnqueueInfo> mDelayedEnqueueInfos;
    nsTHashtable<nsStringHashKey> mFilesReading;
    nsTHashtable<nsStringHashKey> mFilesWriting;
  };

  struct StoragesCompleteCallback
  {
    nsTArray<nsCOMPtr<nsIOfflineStorage> > mStorages;
    nsCOMPtr<nsIRunnable> mCallback;
  };

  FileService();
  ~FileService();

  nsresult
  Init();

  nsresult
  Cleanup();

  bool
  MaybeFireCallback(StoragesCompleteCallback& aCallback);

  nsCOMPtr<nsIEventTarget> mStreamTransportTarget;
  nsClassHashtable<nsCStringHashKey, StorageInfo> mStorageInfos;
  nsTArray<StoragesCompleteCallback> mCompleteCallbacks;
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_FileService_h
