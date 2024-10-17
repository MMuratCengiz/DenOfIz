%module DenOfIz

%include "typemaps.i"  // SWIG typemap support

%typemap(cstype) const char* "string"
%typemap(csin) const char* (string s) { $1 = (char*)Marshal.StringToHGlobalAnsi(s).ToPointer(); }
%typemap(csout) const char* "$csretval = Marshal.PtrToStringAnsi((IntPtr)$1);"
%typemap(csfree) const char* "Marshal.FreeHGlobal((IntPtr)$1);"

%typemap(cscode) const char* %{
    using System.Runtime.InteropServices;
%}

// InteropString class binding
%inline %{
class InteropString
{
    public:
        InteropString(const char* str = nullptr);
        ~InteropString();
        InteropString(const InteropString& other);
        InteropString& operator=(const InteropString& other);
        const char* Get() const;
        InteropString& Append(const char* str);
    };
%}
