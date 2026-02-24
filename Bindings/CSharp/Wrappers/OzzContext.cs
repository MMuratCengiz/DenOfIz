using System;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class OzzContext
    {
        internal ulong Handle { get; private set; }

        internal OzzContext(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
        }

        public static implicit operator ulong(OzzContext wrapper) => wrapper?.Handle ?? 0;

    }
}
