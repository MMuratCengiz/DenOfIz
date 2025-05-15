package com.denofiz.graphics;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.net.URL;
import java.nio.file.Files;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicBoolean;

class NativeLibraryLoader {
    private static final Map<String, Boolean> loadedLibraries = new HashMap<>();
    private static final Object loadLock = new Object();
    private static final AtomicBoolean initialized = new AtomicBoolean(false);

    private NativeLibraryLoader() {
    }

    public static void initialize() {
        if (initialized.get()) {
            return;
        }

        synchronized (loadLock) {
            if (initialized.get()) {
                return;
            }

            String os = getOperatingSystem();
            String arch = getArchitecture();
            String resourcePath = "/native/" + os + "/" + arch + "/";
            loadNativeLibraries(resourcePath);
            initialized.set(true);
        }
    }

    private static void loadNativeLibraries(String resourcePath) {
        File tempDir;
        try {
            tempDir = Files.createTempDirectory("denofiz-native-libs").toFile();
            tempDir.deleteOnExit();
        } catch (IOException ex) {
            throw new RuntimeException("Failed to create temporary directory for native libraries", ex);
        }

        for (String resourceFile : getResourceFiles(resourcePath)) {
            String fullResourcePath = resourcePath + resourceFile;
            URL url = DenOfIzRuntime.class.getResource(fullResourcePath);
            if (url != null) {
                try {
                    File tempFile = new File(tempDir, resourceFile);
                    tempFile.deleteOnExit();
                    try (InputStream in = url.openStream(); FileOutputStream out = new FileOutputStream(tempFile)) {
                        byte[] buffer = new byte[4096];
                        int bytesRead;
                        while ((bytesRead = in.read(buffer)) != -1) {
                            out.write(buffer, 0, bytesRead);
                        }
                    }
                } catch (IOException ex) {
                    throw new RuntimeException("Failed to extract native library: " + fullResourcePath, ex);
                }
            } else {
                throw new RuntimeException("Native library not found: " + fullResourcePath);
            }
        }

        String os = getOperatingSystem();
        if ("windows".equals(os)) {
            loadLibrary(new File(tempDir, "dxcompiler.dll").getAbsolutePath());
            loadLibrary(new File(tempDir, "dxil.dll").getAbsolutePath());
            loadLibrary(new File(tempDir, "metalirconverter.dll").getAbsolutePath());
            loadLibrary(new File(tempDir, "DenOfIzGraphicsJava.dll").getAbsolutePath());
        } else if ("macos".equals(os)) {
            loadLibrary(new File(tempDir, "libdxcompiler.dylib").getAbsolutePath());
            loadLibrary(new File(tempDir, "libmetalirconverter.dylib").getAbsolutePath());
            loadLibrary(new File(tempDir, "libDenOfIzGraphicsJava.jnilib").getAbsolutePath());
        } else {
            loadLibrary(new File(tempDir, "libdxcompiler.so").getAbsolutePath());
            loadLibrary(new File(tempDir, "libDenOfIzGraphicsJava.so").getAbsolutePath());
        }
    }

    private static String[] getResourceFiles(String directory) {
        String os = getOperatingSystem();
        // Todo, we might have DenOfIzGraphics.dylib here and many more libraries if we build shared dlls
        if ("windows".equals(os)) {
            return new String[]{
                    "dxcompiler.dll",
                    "dxil.dll",
                    "metalirconverter.dll",
                    "DenOfIzGraphicsJava.dll"
            };
        } else if ("macos".equals(os)) {
            return new String[]{
                    "libdxcompiler.dylib",
                    "libmetalirconverter.dylib",
                    "libDenOfIzGraphicsJava.jnilib"
            };
        } else { // Linux
            return new String[]{
                    "libdxcompiler.so",
                    "libDenOfIzGraphicsJava.so"
            };
        }
    }

    private static void loadLibrary(String path) {
        if (loadedLibraries.containsKey(path)) {
            return;
        }

        System.load(path);
        loadedLibraries.put(path, true);
    }

    private static String getOperatingSystem() {
        String os = System.getProperty("os.name").toLowerCase();

        if (os.contains("win")) {
            return "windows";
        } else if (os.contains("mac")) {
            return "macos";
        } else {
            return "linux";
        }
    }

    private static String getArchitecture() {
        String arch = System.getProperty("os.arch").toLowerCase();

        if (arch.contains("amd64") || arch.contains("x86_64")) {
            return "x64";
        } else if (arch.contains("aarch64") || arch.contains("arm64")) {
            return "arm64";
        } else if (arch.contains("arm")) {
            return "arm";
        } else if (arch.contains("86")) {
            return "x86";
        } else {
            return "unknown";
        }
    }
}