%module(directors="1") DenOfIzGraphics

// ===========================================================================================
// IDxcBlob handler - Provides SWIG types for Microsoft DirectX Compiler interfaces
// ===========================================================================================

%{
#include <dxcapi.h>
%}

// Forward declare the DirectX interface
class IDxcBlob;

// Create a SWIG-compatible interface for IDxcBlob
%nodefaultctor IDxcBlob;
%nodefaultdtor IDxcBlob;
class IDxcBlob {
public:
    // Match the actual method names in dxcapi.h (they already use capital case)
    virtual void* GetBufferPointer() = 0;
    virtual size_t GetBufferSize() = 0;
};

// Apply compiler-specific typemaps based on target language
#ifdef SWIGJAVA
// Java-specific typemaps for IDxcBlob
%typemap(javacode) IDxcBlob %{
  public java.nio.ByteBuffer getBuffer() {
    long size = GetBufferSize();
    if (size <= 0) return java.nio.ByteBuffer.allocate(0);
    
    return DenOfIzGraphicsPINVOKE.IDxcBlob_GetByteBuffer(getCPtr(this), size);
  }
%}
#endif

#ifdef SWIGCSHARP
// C#-specific typemaps for IDxcBlob
%typemap(csimports) IDxcBlob %{
using System;
using System.Runtime.InteropServices;
%}

%typemap(cscode) IDxcBlob %{
  // The native object address
  private IntPtr handle;
  
  public IDxcBlob(IntPtr handle) {
    this.handle = handle;
  }
  
  public IntPtr GetNativeHandle() {
    return handle;
  }
  
  // Get the blob data size
  public int GetBufferSize() {
    return DenOfIzGraphicsPINVOKE.IDxcBlob_GetBufferSize(swigCPtr);
  }
  
  // Get the blob data as byte array
  public byte[] GetBufferBytes() {
    int size = GetBufferSize();
    if (size <= 0) return new byte[0];
    
    byte[] result = new byte[size];
    IntPtr pBuffer = DenOfIzGraphicsPINVOKE.IDxcBlob_GetBufferPointer(swigCPtr);
    Marshal.Copy(pBuffer, result, 0, size);
    return result;
  }
%}
#endif

// No need for %extend since we're using the actual method names from dxcapi.h

// Handle DirectX compiler blob pointers
%apply void* SWIGTYPE { IDxcBlob* IDxcBlobPtr }