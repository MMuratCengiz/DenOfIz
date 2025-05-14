%module(directors="1") DenOfIzGraphics

// Make sure to use the same module name as the main interface file
// to avoid duplicate class definitions

%csnamespace DenOfIz

%include "arrays_csharp.i"
%apply unsigned char INPUT[]  {unsigned char *inputBytes}