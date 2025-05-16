// SwapChain_Cache_Java.i - Java proxy caching for ISwapChain to reduce GC pressure

%rename(getRenderTarget_original) DenOfIz::ISwapChain::GetRenderTarget;
%rename(getViewport_original) DenOfIz::ISwapChain::GetViewport;
%rename(resize_original) DenOfIz::ISwapChain::Resize;

%typemap(javacode) DenOfIz::ISwapChain %{
    private final java.util.HashMap<Integer, com.denofiz.graphics.ITextureResource> renderTargetCache = new java.util.HashMap<>();
    private com.denofiz.graphics.Viewport viewportCache = null;
    private boolean viewportInvalidated = true;
%}

%extend DenOfIz::ISwapChain {
  %proxycode %{
    public com.denofiz.graphics.ITextureResource getRenderTarget(int image) {
        return renderTargetCache.computeIfAbsent(image, this::getRenderTarget_original);
    }

    public com.denofiz.graphics.Viewport getViewport() {
        if (viewportCache == null || viewportInvalidated) {
            viewportCache = getViewport_original();
            viewportInvalidated = false;
        }
        return viewportCache;
    }

    public void resize(int width, int height) {
        resize_original(width, height);
        viewportInvalidated = true;
        renderTargetCache.clear();
    }
  %}
}

%typemap(javafinalize) DenOfIz::ISwapChain %{
    @SuppressWarnings({"deprecation", "removal"})
    protected void finalize() {
        renderTargetCache.clear();
        viewportCache = null;
        delete();
    }
%}