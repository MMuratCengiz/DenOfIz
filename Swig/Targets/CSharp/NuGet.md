# NuGet - Notes

### OSX

For Osx the library is still not signed, so we must manually sign DenOfIzGraphics.dylib:

- cd into nuget package directory
- `codesign --force --timestamp --sign "$(security find-identity -v -p codesigning | grep "^ *1)" | sed -E 's/.*"([^"]+)".*/\1/')" runtimes/osx-arm64/native/DenOfIzGraphicsCSharp.dylib`
