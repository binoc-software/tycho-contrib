/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set sw=2 ts=8 et tw=80 : */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_net_WebSocketChannelChild_h
#define mozilla_net_WebSocketChannelChild_h

#include "mozilla/net/PWebSocketChild.h"
#include "mozilla/net/BaseWebSocketChannel.h"
#include "nsString.h"

namespace mozilla {
namespace net {

class ChannelEvent;
class ChannelEventQueue;

class WebSocketChannelChild : public BaseWebSocketChannel,
                              public PWebSocketChild
{
 public:
  explicit WebSocketChannelChild(bool aSecure);

  NS_DECL_THREADSAFE_ISUPPORTS
  NS_DECL_NSITHREADRETARGETABLEREQUEST

  // nsIWebSocketChannel methods BaseWebSocketChannel didn't implement for us
  //
  NS_IMETHOD AsyncOpen(nsIURI *aURI, const nsACString &aOrigin,
                       nsIWebSocketListener *aListener, nsISupports *aContext) override;
  NS_IMETHOD Close(uint16_t code, const nsACString & reason) override;
  NS_IMETHOD SendMsg(const nsACString &aMsg) override;
  NS_IMETHOD SendBinaryMsg(const nsACString &aMsg) override;
  NS_IMETHOD SendBinaryStream(nsIInputStream *aStream, uint32_t aLength) override;
  nsresult SendBinaryStream(OptionalInputStreamParams *aStream, uint32_t aLength);
  NS_IMETHOD GetSecurityInfo(nsISupports **aSecurityInfo) override;

  void AddIPDLReference();
  void ReleaseIPDLReference();

  // Off main thread URI access.
  void GetEffectiveURL(nsAString& aEffectiveURL) const override;
  bool IsEncrypted() const override;

 private:
  ~WebSocketChannelChild();

  bool RecvOnStart(const nsCString& aProtocol, const nsCString& aExtensions,
                   const nsString& aEffectiveURL, const bool& aSecure) override;
  bool RecvOnStop(const nsresult& aStatusCode) override;
  bool RecvOnMessageAvailable(const nsCString& aMsg) override;
  bool RecvOnBinaryMessageAvailable(const nsCString& aMsg) override;
  bool RecvOnAcknowledge(const uint32_t& aSize) override;
  bool RecvOnServerClose(const uint16_t& aCode, const nsCString &aReason) override;

  void OnStart(const nsCString& aProtocol, const nsCString& aExtensions,
               const nsString& aEffectiveURL, const bool& aSecure);
  void OnStop(const nsresult& aStatusCode);
  void OnMessageAvailable(const nsCString& aMsg);
  void OnBinaryMessageAvailable(const nsCString& aMsg);
  void OnAcknowledge(const uint32_t& aSize);
  void OnServerClose(const uint16_t& aCode, const nsCString& aReason);
  void AsyncOpenFailed();  

  void DispatchToTargetThread(ChannelEvent *aChannelEvent);
  bool IsOnTargetThread();

  void MaybeReleaseIPCObject();

  nsRefPtr<ChannelEventQueue> mEventQ;
  nsString mEffectiveURL;

  // This variable is protected by mutex.
  enum {
    Opened,
    Closing,
    Closed
  } mIPCState;

  mozilla::Mutex mMutex;

  friend class StartEvent;
  friend class StopEvent;
  friend class MessageEvent;
  friend class AcknowledgeEvent;
  friend class ServerCloseEvent;
  friend class AsyncOpenFailedEvent;
};

} // namespace net
} // namespace mozilla

#endif // mozilla_net_WebSocketChannelChild_h
