package com.denofiz.graphics;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicBoolean;

class NativeLibraryLoader {
    private static final Map<String, Boolean> loadedLibraries = new HashMap<>();
    private static final Object loadLock = new Object();
    private static final AtomicBoolean initialized = new AtomicBoolean(false);
    
    private NativeLibraryLoader() {}

    public static void initialize() {
        if (initialized.get()) {
            return;
        }
        
        synchronized (loadLock) {
            if (initialized.get()) {
                return;
            }
            
            String nativePath = getNativeLibraryPath();
            loadAllNativeLibraries(nativePath);
            initialized.set(true);
        }
    }

    private static String getNativeLibraryPath() {
        String os = getOperatingSystem();
        String arch = getArchitecture();
        
        String resourcePath = "/native/" + os + "/" + arch + "/";
        File tempDir = createTempDirectory();
        
        extractNativeLibraries(resourcePath, tempDir);
        return tempDir.getAbsolutePath();
    }
    
    private static File createTempDirectory() {
        try {
            Path tempDir = Files.createTempDirectory("denofiz-native-libs");
            File directory = tempDir.toFile();
            directory.deleteOnExit();
            return directory;
        } catch (IOException e) {
            throw new RuntimeException("Failed to create temp directory for native libraries", e);
        }
    }

    private static void extractNativeLibraries(String resourcePath, File targetDir) {
        try {
            // Find all library files in the resource path
            String[] resourceFiles = getResourceFiles(resourcePath);
            if (resourceFiles == null || resourceFiles.length == 0) {
                throw new RuntimeException("No native libraries found in resources at: " + resourcePath);
            }
            
            for (String resourceFile : resourceFiles) {
                String fullResourcePath = resourcePath + resourceFile;
                
                try (InputStream is = NativeLibraryLoader.class.getResourceAsStream(fullResourcePath)) {
                    if (is == null) {
                        System.err.println("Warning: Could not find native library: " + fullResourcePath);
                        continue;
                    }
                    
                    File targetFile = new File(targetDir, resourceFile);
                    Files.copy(is, targetFile.toPath(), StandardCopyOption.REPLACE_EXISTING);
                    targetFile.deleteOnExit();
                }
            }
        } catch (IOException e) {
            throw new RuntimeException("Failed to extract native libraries", e);
        }
    }
    
    private static String[] getResourceFiles(String directory) {
        String os = getOperatingSystem();
        if ("windows".equals(os)) {
            return new String[] {
                "DenOfIzGraphicsJava.dll",
                "DenOfIzGraphics.dll",
                "dxcompiler.dll",
                "dxil.dll",
                "metalirconverter.dll"
            };
        } else if ("macos".equals(os)) {
            return new String[] {
                "libDenOfIzGraphicsJava.jnilib",
                "libDenOfIzGraphics.a",
                "libdxcompiler.dylib",
                "libmetalirconverter.dylib"
            };
        } else { // Linux
            return new String[] {
                "libDenOfIzGraphicsJava.so",
                "libDenOfIzGraphics.a",
                "libdxcompiler.so"
            };
        }
    }
    
    private static void loadAllNativeLibraries(String directory) {
        String os = getOperatingSystem();
        String libraryExtension = getLibraryExtension();
        
        String coreLibPrefix = "windows".equals(os) ?
                "DenOfIzGraphicsJava" : "libDenOfIzGraphicsJava";
        
        File dir = new File(directory);
        File[] files = dir.listFiles();
        if (files == null) {
            throw new RuntimeException("Failed to list files in native library directory: " + directory);
        }
        
        // First load the core library
        for (File file : files) {
            String filename = file.getName();
            if (filename.startsWith(coreLibPrefix) && filename.endsWith(libraryExtension)) {
                loadLibrary(file.getAbsolutePath());
                break;
            }
        }
        
        // Then load dependencies
        for (File file : files) {
            String filename = file.getName();
            if (!filename.startsWith(coreLibPrefix) && filename.endsWith(libraryExtension)) {
                loadLibrary(file.getAbsolutePath());
            }
        }
    }
    
    private static void loadLibrary(String path) {
        if (loadedLibraries.containsKey(path)) {
            return;
        }
        
        try {
            System.load(path);
            loadedLibraries.put(path, true);
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Failed to load native library: " + path);
            throw e;
        }
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
    
    private static String getLibraryExtension() {
        String os = getOperatingSystem();
        
        if ("windows".equals(os)) {
            return ".dll";
        } else if ("macos".equals(os)) {
            return ".dylib";
        } else {
            return ".so";
        }
    }
}