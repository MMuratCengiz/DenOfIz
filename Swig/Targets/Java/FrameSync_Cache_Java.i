// FrameSync_Cache_Java.i - Java proxy caching for FrameSync to reduce GC pressure

%rename(getCommandList_original) DenOfIz::FrameSync::GetCommandList;
%typemap(javacode) DenOfIz::FrameSync %{
    private final java.util.HashMap<Long, com.denofiz.graphics.ICommandList> commandListCache = new java.util.HashMap<>();
%}

%extend DenOfIz::FrameSync {
  %proxycode %{
    public com.denofiz.graphics.ICommandList getCommandList(long frame) {
        return commandListCache.computeIfAbsent(frame, this::getCommandList_original);
    }
  %}
}

%typemap(javafinalize) DenOfIz::FrameSync %{
    @SuppressWarnings({"deprecation", "removal"})
    protected void finalize() {
        commandListCache.clear();
        delete();
    }
%}