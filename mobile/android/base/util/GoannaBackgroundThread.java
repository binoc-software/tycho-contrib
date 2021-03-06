/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.goanna.util;

import android.os.Handler;
import android.os.Looper;

import java.util.concurrent.SynchronousQueue;

final class GoannaBackgroundThread extends Thread {
    private static final String LOOPER_NAME = "GoannaBackgroundThread";

    // Guarded by 'GoannaBackgroundThread.class'.
    private static Handler handler;
    private static Thread thread;

    // The initial Runnable to run on the new thread. Its purpose
    // is to avoid us having to wait for the new thread to start.
    private Runnable initialRunnable;

    // Singleton, so private constructor.
    private GoannaBackgroundThread(final Runnable initialRunnable) {
        this.initialRunnable = initialRunnable;
    }

    @Override
    public void run() {
        setName(LOOPER_NAME);
        Looper.prepare();

        synchronized (GoannaBackgroundThread.class) {
            handler = new Handler();
            GoannaBackgroundThread.class.notify();
        }

        if (initialRunnable != null) {
            initialRunnable.run();
            initialRunnable = null;
        }

        Looper.loop();
    }

    private static void startThread(final Runnable initialRunnable) {
        thread = new GoannaBackgroundThread(initialRunnable);
        ThreadUtils.setBackgroundThread(thread);

        thread.setDaemon(true);
        thread.start();
    }

    // Get a Handler for a looper thread, or create one if it doesn't yet exist.
    /*package*/ static synchronized Handler getHandler() {
        if (thread == null) {
            startThread(null);
        }

        while (handler == null) {
            try {
                GoannaBackgroundThread.class.wait();
            } catch (final InterruptedException e) {
            }
        }
        return handler;
    }

    /*package*/ static synchronized void post(final Runnable runnable) {
        if (thread == null) {
            startThread(runnable);
            return;
        }
        getHandler().post(runnable);
    }
}
