// FrameSync_Cache_CSharp.i - C# proxy caching for FrameSync to reduce GC pressure

%rename(GetCommandList_original) DenOfIz::FrameSync::GetCommandList;
%ignore DenOfIz::FrameSync::GetCommandList_original;

%typemap(cscode) DenOfIz::FrameSync %{
  private static readonly System.Collections.Generic.Dictionary<System.IntPtr, System.Collections.Generic.Dictionary<uint, DenOfIz.ICommandList>>
      _frameSyncCommandListCache = new System.Collections.Generic.Dictionary<System.IntPtr, System.Collections.Generic.Dictionary<uint, DenOfIz.ICommandList>>();
%}
%extend DenOfIz::FrameSync {
  %proxycode %{
    public DenOfIz.ICommandList GetCommandList(uint frame) {
      System.IntPtr thisPtr = swigCPtr.Handle;

      if (!_frameSyncCommandListCache.TryGetValue(thisPtr, out var frameCache)) {
          frameCache = new System.Collections.Generic.Dictionary<uint, DenOfIz.ICommandList>();
          _frameSyncCommandListCache[thisPtr] = frameCache;
      }
      if (!frameCache.TryGetValue(frame, out var cachedList)) {
          cachedList = GetCommandList_original(frame);
          frameCache[frame] = cachedList;
      }
      return cachedList;
    }
  %}
}

%typemap(csdispose_derived) DenOfIz::FrameSync %{
  protected override void Dispose(bool disposing) {
    lock(this) {
      if (swigCPtr.Handle != global::System.IntPtr.Zero) {
        if (disposing) {
          System.IntPtr thisPtr = swigCPtr.Handle;
          if (_frameSyncCommandListCache.ContainsKey(thisPtr)) {
            _frameSyncCommandListCache.Remove(thisPtr);
          }
        }
        if (swigCMemOwn) {
          swigCMemOwn = false;
          DenOfIzGraphicsPINVOKE.delete_FrameSync(swigCPtr);
        }
        swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
      }
    }
    base.Dispose(disposing);
  }
%}