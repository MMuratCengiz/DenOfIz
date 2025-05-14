%module(directors="1") DenOfIzGraphics

%javapackage("com.denofiz.graphics")

%include "arrays_java.i"
%include "enums.swg"

%javaconst(1);
%feature("autodoc", "1");
%feature("accessors", "1");

%apply int[] {int *}
%apply long[] {long *}
%apply float[] {float *}
%apply boolean[] {bool *}
%apply double[] {double *}
%apply unsigned char[] {unsigned char *inputBytes}
%apply unsigned char[] {unsigned char *data}