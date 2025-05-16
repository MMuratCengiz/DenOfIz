// SwapChain_Cache_CSharp.i - C# proxy caching for ISwapChain to reduce GC pressure

%rename(GetRenderTarget_original) DenOfIz::ISwapChain::GetRenderTarget;
%rename(GetViewport_original) DenOfIz::ISwapChain::GetViewport;
%rename(Resize_original) DenOfIz::ISwapChain::Resize;

%typemap(cscode) DenOfIz::ISwapChain %{
    private static readonly System.Collections.Generic.Dictionary<System.IntPtr, System.Collections.Generic.Dictionary<uint, DenOfIz.ITextureResource>>
        _swapChainRenderTargetCache = new System.Collections.Generic.Dictionary<System.IntPtr, System.Collections.Generic.Dictionary<uint, DenOfIz.ITextureResource>>();
    private static readonly System.Collections.Generic.Dictionary<System.IntPtr, DenOfIz.Viewport>
        _swapChainViewportCache = new System.Collections.Generic.Dictionary<System.IntPtr, DenOfIz.Viewport>();
    private static readonly System.Collections.Generic.Dictionary<System.IntPtr, bool>
        _swapChainResized = new System.Collections.Generic.Dictionary<System.IntPtr, bool>();
%}

%extend DenOfIz::ISwapChain {
  %proxycode %{
    public DenOfIz.ITextureResource GetRenderTarget(uint image) {
        System.IntPtr thisPtr = swigCPtr.Handle;

        if (!_swapChainRenderTargetCache.TryGetValue(thisPtr, out var imageCache)) {
            imageCache = new System.Collections.Generic.Dictionary<uint, DenOfIz.ITextureResource>();
            _swapChainRenderTargetCache[thisPtr] = imageCache;
        }

        if (!imageCache.TryGetValue(image, out var cachedTarget)) {
            cachedTarget = GetRenderTarget_original(image);
            imageCache[image] = cachedTarget;
        }

        return cachedTarget;
    }

    // Cached viewport, invalidated when resized
    public DenOfIz.Viewport GetViewport() {
        System.IntPtr thisPtr = swigCPtr.Handle;

        bool resized = false;
        if (_swapChainResized.TryGetValue(thisPtr, out resized) && resized) {
            _swapChainResized[thisPtr] = false;
            if (_swapChainViewportCache.ContainsKey(thisPtr)) {
                _swapChainViewportCache.Remove(thisPtr);
            }
        }
        if (!_swapChainViewportCache.TryGetValue(thisPtr, out var cachedViewport)) {
            cachedViewport = GetViewport_original();
            _swapChainViewportCache[thisPtr] = cachedViewport;
        }
        return cachedViewport;
    }

    public void Resize(uint width, uint height) {
        System.IntPtr thisPtr = swigCPtr.Handle;
        _swapChainResized[thisPtr] = true;
        Resize_original(width, height);
        if (_swapChainRenderTargetCache.TryGetValue(thisPtr, out var imageCache)) {
            imageCache.Clear();
        }
    }
  %}
}

%typemap(csdispose_derived) DenOfIz::ISwapChain %{
    protected override void Dispose(bool disposing) {
        lock(this) {
            if (swigCPtr.Handle != global::System.IntPtr.Zero) {
                if (disposing) {
                    System.IntPtr thisPtr = swigCPtr.Handle;
                    if (_swapChainRenderTargetCache.ContainsKey(thisPtr)) {
                        _swapChainRenderTargetCache.Remove(thisPtr);
                    }
                    if (_swapChainViewportCache.ContainsKey(thisPtr)) {
                        _swapChainViewportCache.Remove(thisPtr);
                    }
                    if (_swapChainResized.ContainsKey(thisPtr)) {
                        _swapChainResized.Remove(thisPtr);
                    }
                }
                if (swigCMemOwn) {
                    swigCMemOwn = false;
                    DenOfIzGraphicsPINVOKE.delete_ISwapChain(swigCPtr);
                }
                swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
            }
        }
        base.Dispose(disposing);
    }
%}
