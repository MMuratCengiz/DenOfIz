using System;
using DenOfIz.Native;

namespace DenOfIz
{
    public static partial class DenOfIzGraphics
    {
        public static uint NumBytes(Format format)
        {
            return Methods.DenOfIz_Format_NumBytes(format);
        }

        public static Format ToTypeless(Format format)
        {
            return Methods.DenOfIz_Format_ToTypeless(format);
        }

        public static FormatSubType GetSubType(Format format)
        {
            return Methods.DenOfIz_Format_GetSubType(format);
        }

        public static bool IsBC(Format format)
        {
            return Methods.DenOfIz_Format_IsBC(format);
        }

        public static uint BlockSize(Format format)
        {
            return Methods.DenOfIz_Format_BlockSize(format);
        }

        public static bool IsDepth(Format format)
        {
            return Methods.DenOfIz_Format_IsDepth(format);
        }

        public static bool IsStencil(Format format)
        {
            return Methods.DenOfIz_Format_IsStencil(format);
        }

        public static bool IsDepthOnly(Format format)
        {
            return Methods.DenOfIz_Format_IsDepthOnly(format);
        }

        public static bool IsDepthStencil(Format format)
        {
            return Methods.DenOfIz_Format_IsDepthStencil(format);
        }

        public static bool HasStencilComponent(Format format)
        {
            return Methods.DenOfIz_Format_HasStencilComponent(format);
        }

        public static ResourceBindingType FromDescriptor(uint descriptor)
        {
            return Methods.DenOfIz_ResourceBindingType_FromDescriptor(descriptor);
        }

        public static int ToNumSamples(MSAASampleCount sampleCount)
        {
            return Methods.DenOfIz_MSAASampleCount_ToNumSamples(sampleCount);
        }

    }
}
