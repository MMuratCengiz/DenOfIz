using System.Runtime.CompilerServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public partial struct OzzExportResult
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public void Destroy()
        {
            Methods.DenOfIz_OzzExportResult_Destroy(ref this);
        }
    }
}
