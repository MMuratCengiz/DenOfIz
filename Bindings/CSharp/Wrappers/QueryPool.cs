using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class QueryPool : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(QueryPool wrapper) => wrapper?.Handle ?? 0;

        internal QueryPool(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public QueryType GetQueryPoolType()
        {
            ThrowIfDisposed();
            QueryType outType = default;
            Methods.DenOfIz_QueryPool_GetType(Handle, out outType);
            return outType;
        }

        public uint GetNumQueries()
        {
            ThrowIfDisposed();
            uint outNumQueries = default;
            Methods.DenOfIz_QueryPool_GetNumQueries(Handle, out outNumQueries);
            return outNumQueries;
        }

        public double GetTimestampFrequency()
        {
            ThrowIfDisposed();
            double outFrequency = default;
            Methods.DenOfIz_QueryPool_GetTimestampFrequency(Handle, out outFrequency);
            return outFrequency;
        }

        public QueryData GetQueryData(uint queryIndex)
        {
            ThrowIfDisposed();
            QueryData outData = default;
            Methods.DenOfIz_QueryPool_GetQueryData(Handle, queryIndex, out outData);
            return outData;
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_QueryPool_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(QueryPool));
            }
        }
    }
}
