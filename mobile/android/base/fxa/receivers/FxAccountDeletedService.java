/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.goanna.fxa.receivers;

import org.mozilla.goanna.background.common.log.Logger;
import org.mozilla.goanna.fxa.FxAccountConstants;
import org.mozilla.goanna.fxa.sync.FxAccountNotificationManager;
import org.mozilla.goanna.fxa.sync.FxAccountSyncAdapter;
import org.mozilla.goanna.sync.config.AccountPickler;
import org.mozilla.goanna.sync.repositories.android.FennecTabsRepository;

import android.app.IntentService;
import android.content.Context;
import android.content.Intent;

/**
 * A background service to clean up after a Firefox Account is deleted.
 * <p>
 * Note that we specifically handle deleting the pickle file using a Service and a
 * BroadcastReceiver, rather than a background thread, to allow channels sharing a Firefox account
 * to delete their respective pickle files (since, if one remains, the account will be restored
 * when that channel is used).
 */
public class FxAccountDeletedService extends IntentService {
  public static final String LOG_TAG = FxAccountDeletedService.class.getSimpleName();

  public FxAccountDeletedService() {
    super(LOG_TAG);
  }

  @Override
  protected void onHandleIntent(final Intent intent) {
    // Intent can, in theory, be null. Bug 1025937.
    if (intent == null) {
      Logger.debug(LOG_TAG, "Short-circuiting on null intent.");
      return;
    }

    final Context context = this;

    long intentVersion = intent.getLongExtra(
        FxAccountConstants.ACCOUNT_DELETED_INTENT_VERSION_KEY, 0);
    long expectedVersion = FxAccountConstants.ACCOUNT_DELETED_INTENT_VERSION;
    if (intentVersion != expectedVersion) {
      Logger.warn(LOG_TAG, "Intent malformed: version " + intentVersion + " given but " +
          "version " + expectedVersion + "expected. Not cleaning up after deleted Account.");
      return;
    }

    // Android Account name, not Sync encoded account name.
    final String accountName = intent.getStringExtra(
        FxAccountConstants.ACCOUNT_DELETED_INTENT_ACCOUNT_KEY);
    if (accountName == null) {
      Logger.warn(LOG_TAG, "Intent malformed: no account name given. Not cleaning up after " +
          "deleted Account.");
      return;
    }

    Logger.info(LOG_TAG, "Firefox account named " + accountName + " being removed; " +
        "deleting saved pickle file '" + FxAccountConstants.ACCOUNT_PICKLE_FILENAME + "'.");
    deletePickle(context);

    // Delete client database and non-local tabs.
    Logger.info(LOG_TAG, "Deleting the entire clients database and non-local tabs");
    FennecTabsRepository.deleteNonLocalClientsAndTabs(context);

    // Remove any displayed notifications.
    new FxAccountNotificationManager(FxAccountSyncAdapter.NOTIFICATION_ID).clear(context);

    // Bug 1147275: Delete cached oauth tokens. There's no way to query all
    // oauth tokens from Android, so this is tricky to do comprehensively. We
    // can query, individually, for specific oauth tokens to delete, however.
  }

  public static void deletePickle(final Context context) {
    try {
      AccountPickler.deletePickle(context, FxAccountConstants.ACCOUNT_PICKLE_FILENAME);
    } catch (Exception e) {
      // This should never happen, but we really don't want to die in a background thread.
      Logger.warn(LOG_TAG, "Got exception deleting saved pickle file; ignoring.", e);
    }
  }
}
