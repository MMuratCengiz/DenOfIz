package com.denofiz.graphics;

import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Used to load native libraries and engine logging infrastructure(among other things)
 */
public class DenOfIzRuntime {
    private static final AtomicBoolean initialized = new AtomicBoolean(false);
    private static final Object lock = new Object();
    
    static {
        NativeLibraryLoader.initialize();
        
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            if (initialized.get()) {
                GraphicsApi.reportLiveObjects();
            }
        }));
    }
    
    private DenOfIzRuntime() {}

    public static void initializeRuntime() {
        if (initialized.get()) {
            return;
        }

        synchronized (lock) {
            if (initialized.get()) {
                return;
            }

            NativeLibraryLoader.initialize();
            initialized.set(true);
        }
    }
    public static void initializeEngine(EngineDesc engineDesc) {
        Engine.init(engineDesc);
    }

    public static boolean isInitialized() {
        return initialized.get();
    }
}