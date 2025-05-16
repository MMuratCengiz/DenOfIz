%module(directors="1") DenOfIzGraphicsJava

%include "arrays_java.i"
%include "enums.swg"

%javaconst(1);
%feature("autodoc", "1");
%feature("accessors", "1");
%rename("%(firstlowercase)s", %$isfunction) "";
%rename("waitOnFence") "DenOfIz::IFence::Wait";
%rename("notifySemaphore") "DenOfIz::ISemaphore::Notify";
%include "FrameSync_Cache_Java.i"
%include "SwapChain_Cache_Java.i"