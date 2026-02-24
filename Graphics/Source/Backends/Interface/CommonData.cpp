/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "DenOfIzGraphics/Backends/Interface/CommonData.h"

uint32_t DenOfIz_Format_NumBytes( const DenOfIz_Format format )
{
    switch ( format )
    {
    case DENOFIZ_FORMAT_R32G32B32A32_FLOAT:
    case DENOFIZ_FORMAT_R32G32B32A32_UINT:
    case DENOFIZ_FORMAT_R32G32B32A32_SINT:
    case DENOFIZ_FORMAT_R32G32B32A32_TYPELESS:
        return 16;
    case DENOFIZ_FORMAT_R32G32B32_FLOAT:
    case DENOFIZ_FORMAT_R32G32B32_UINT:
    case DENOFIZ_FORMAT_R32G32B32_SINT:
        return 12;
    case DENOFIZ_FORMAT_R16G16B16A16_FLOAT:
    case DENOFIZ_FORMAT_R16G16B16A16_UNORM:
    case DENOFIZ_FORMAT_R16G16B16A16_UINT:
    case DENOFIZ_FORMAT_R16G16B16A16_SNORM:
    case DENOFIZ_FORMAT_R16G16B16A16_SINT:
    case DENOFIZ_FORMAT_R16G16B16A16_TYPELESS:
    case DENOFIZ_FORMAT_R32G32_FLOAT:
    case DENOFIZ_FORMAT_R32G32_UINT:
    case DENOFIZ_FORMAT_R32G32_SINT:
    case DENOFIZ_FORMAT_R32G32_TYPELESS:
        return 8;
    case DENOFIZ_FORMAT_R10G10B10A2_UNORM:
    case DENOFIZ_FORMAT_R10G10B10A2_UINT:
    case DENOFIZ_FORMAT_R10G10B10A2_TYPELESS:
    case DENOFIZ_FORMAT_R8G8B8A8_UNORM:
    case DENOFIZ_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DENOFIZ_FORMAT_R8G8B8A8_UINT:
    case DENOFIZ_FORMAT_R8G8B8A8_SNORM:
    case DENOFIZ_FORMAT_R8G8B8A8_SINT:
    case DENOFIZ_FORMAT_R8G8B8A8_TYPELESS:
    case DENOFIZ_FORMAT_R16G16_FLOAT:
    case DENOFIZ_FORMAT_R16G16_UNORM:
    case DENOFIZ_FORMAT_R16G16_UINT:
    case DENOFIZ_FORMAT_R16G16_SNORM:
    case DENOFIZ_FORMAT_R16G16_SINT:
    case DENOFIZ_FORMAT_R16G16_TYPELESS:
    case DENOFIZ_FORMAT_D32_FLOAT:
    case DENOFIZ_FORMAT_R32_FLOAT:
    case DENOFIZ_FORMAT_R32_UINT:
    case DENOFIZ_FORMAT_R32_SINT:
    case DENOFIZ_FORMAT_R32_TYPELESS:
    case DENOFIZ_FORMAT_D24_UNORM_S8_UINT:
        return 4;
    case DENOFIZ_FORMAT_R8G8_UNORM:
    case DENOFIZ_FORMAT_R8G8_UINT:
    case DENOFIZ_FORMAT_R8G8_SNORM:
    case DENOFIZ_FORMAT_R8G8_SINT:
    case DENOFIZ_FORMAT_R8G8_TYPELESS:
    case DENOFIZ_FORMAT_R16_FLOAT:
    case DENOFIZ_FORMAT_D16_UNORM:
    case DENOFIZ_FORMAT_R16_UNORM:
    case DENOFIZ_FORMAT_R16_UINT:
    case DENOFIZ_FORMAT_R16_SNORM:
    case DENOFIZ_FORMAT_R16_SINT:
    case DENOFIZ_FORMAT_R16_TYPELESS:
        return 2;
    case DENOFIZ_FORMAT_R8_UNORM:
    case DENOFIZ_FORMAT_R8_UINT:
    case DENOFIZ_FORMAT_R8_SNORM:
    case DENOFIZ_FORMAT_R8_SINT:
    case DENOFIZ_FORMAT_R8_TYPELESS:
        return 1;
        // Recheck what the below are and what the expected sizes are.
    case DENOFIZ_FORMAT_BC1_UNORM:
    case DENOFIZ_FORMAT_BC1_UNORM_SRGB:
        return 8;
    case DENOFIZ_FORMAT_BC2_UNORM:
    case DENOFIZ_FORMAT_BC2_UNORM_SRGB:
    case DENOFIZ_FORMAT_BC3_UNORM:
    case DENOFIZ_FORMAT_BC3_UNORM_SRGB:
        return 16;
    case DENOFIZ_FORMAT_BC4_UNORM:
    case DENOFIZ_FORMAT_BC4_SNORM:
        return 8;
    case DENOFIZ_FORMAT_BC5_UNORM:
    case DENOFIZ_FORMAT_BC5_SNORM:
        return 16;
    case DENOFIZ_FORMAT_B8G8R8A8_UNORM:
        return 4;
    case DENOFIZ_FORMAT_BC6H_UFLOAT16:
    case DENOFIZ_FORMAT_BC6H_SFLOAT16:
    case DENOFIZ_FORMAT_BC7_UNORM:
    case DENOFIZ_FORMAT_BC7_UNORM_SRGB:
        return 16;
    default:
        return 1;
    }
}

DenOfIz_Format DenOfIz_Format_ToTypeless( const DenOfIz_Format format )
{
    switch ( format )
    {
    case DENOFIZ_FORMAT_R32G32B32A32_FLOAT:
    case DENOFIZ_FORMAT_R32G32B32A32_UINT:
    case DENOFIZ_FORMAT_R32G32B32A32_SINT:
        return DENOFIZ_FORMAT_R32G32B32A32_TYPELESS;

    case DENOFIZ_FORMAT_R16G16B16A16_FLOAT:
    case DENOFIZ_FORMAT_R16G16B16A16_UNORM:
    case DENOFIZ_FORMAT_R16G16B16A16_UINT:
    case DENOFIZ_FORMAT_R16G16B16A16_SNORM:
    case DENOFIZ_FORMAT_R16G16B16A16_SINT:
        return DENOFIZ_FORMAT_R16G16B16A16_TYPELESS;

    case DENOFIZ_FORMAT_R32G32_FLOAT:
    case DENOFIZ_FORMAT_R32G32_UINT:
    case DENOFIZ_FORMAT_R32G32_SINT:
        return DENOFIZ_FORMAT_R32G32_TYPELESS;

    case DENOFIZ_FORMAT_R10G10B10A2_UNORM:
    case DENOFIZ_FORMAT_R10G10B10A2_UINT:
        return DENOFIZ_FORMAT_R10G10B10A2_TYPELESS;

    case DENOFIZ_FORMAT_R8G8B8A8_UNORM:
    case DENOFIZ_FORMAT_R8G8B8A8_UINT:
    case DENOFIZ_FORMAT_R8G8B8A8_SNORM:
    case DENOFIZ_FORMAT_R8G8B8A8_SINT:
        return DENOFIZ_FORMAT_R8G8B8A8_TYPELESS;

    case DENOFIZ_FORMAT_R16G16_FLOAT:
    case DENOFIZ_FORMAT_R16G16_UNORM:
    case DENOFIZ_FORMAT_R16G16_UINT:
    case DENOFIZ_FORMAT_R16G16_SNORM:
    case DENOFIZ_FORMAT_R16G16_SINT:
        return DENOFIZ_FORMAT_R16G16_TYPELESS;

    case DENOFIZ_FORMAT_R32_FLOAT:
    case DENOFIZ_FORMAT_R32_UINT:
    case DENOFIZ_FORMAT_R32_SINT:
        return DENOFIZ_FORMAT_R32_TYPELESS;

    case DENOFIZ_FORMAT_R8G8_UNORM:
    case DENOFIZ_FORMAT_R8G8_UINT:
    case DENOFIZ_FORMAT_R8G8_SNORM:
    case DENOFIZ_FORMAT_R8G8_SINT:
        return DENOFIZ_FORMAT_R8G8_TYPELESS;

    case DENOFIZ_FORMAT_R16_FLOAT:
    case DENOFIZ_FORMAT_R16_UNORM:
    case DENOFIZ_FORMAT_R16_UINT:
    case DENOFIZ_FORMAT_R16_SNORM:
    case DENOFIZ_FORMAT_R16_SINT:
        return DENOFIZ_FORMAT_R16_TYPELESS;

    case DENOFIZ_FORMAT_R8_UNORM:
    case DENOFIZ_FORMAT_R8_UINT:
    case DENOFIZ_FORMAT_R8_SNORM:
    case DENOFIZ_FORMAT_R8_SINT:
        return DENOFIZ_FORMAT_R8_TYPELESS;

    default:
        return format;
    }
}

DenOfIz_FormatSubType DenOfIz_Format_GetSubType( const DenOfIz_Format format )
{
    switch ( format )
    {
    case DENOFIZ_FORMAT_UNDEFINED:
        return DENOFIZ_FORMAT_SUB_TYPE_UNDEFINED;
    case DENOFIZ_FORMAT_R32G32B32A32_FLOAT:
    case DENOFIZ_FORMAT_R32G32B32_FLOAT:
    case DENOFIZ_FORMAT_R16G16B16A16_FLOAT:
    case DENOFIZ_FORMAT_R32G32_FLOAT:
    case DENOFIZ_FORMAT_R16G16_FLOAT:
    case DENOFIZ_FORMAT_D32_FLOAT:
    case DENOFIZ_FORMAT_R32_FLOAT:
    case DENOFIZ_FORMAT_R16_FLOAT:
    case DENOFIZ_FORMAT_BC6H_UFLOAT16:
    case DENOFIZ_FORMAT_BC6H_SFLOAT16:
        return DENOFIZ_FORMAT_SUB_TYPE_FLOAT;
    case DENOFIZ_FORMAT_R32G32B32A32_SINT:
    case DENOFIZ_FORMAT_R32G32B32_SINT:
    case DENOFIZ_FORMAT_R16G16B16A16_SINT:
    case DENOFIZ_FORMAT_R32G32_SINT:
    case DENOFIZ_FORMAT_R8G8B8A8_SINT:
    case DENOFIZ_FORMAT_R16G16_SINT:
    case DENOFIZ_FORMAT_R32_SINT:
    case DENOFIZ_FORMAT_R8G8_SINT:
    case DENOFIZ_FORMAT_R16_SINT:
    case DENOFIZ_FORMAT_R8_SINT:
        return DENOFIZ_FORMAT_SUB_TYPE_SINT;
    case DENOFIZ_FORMAT_R32G32B32A32_TYPELESS:
    case DENOFIZ_FORMAT_R16G16B16A16_TYPELESS:
    case DENOFIZ_FORMAT_R32G32_TYPELESS:
    case DENOFIZ_FORMAT_R10G10B10A2_TYPELESS:
    case DENOFIZ_FORMAT_R8G8B8A8_TYPELESS:
    case DENOFIZ_FORMAT_R16G16_TYPELESS:
    case DENOFIZ_FORMAT_R32_TYPELESS:
    case DENOFIZ_FORMAT_R8G8_TYPELESS:
    case DENOFIZ_FORMAT_R16_TYPELESS:
    case DENOFIZ_FORMAT_R8_TYPELESS:
        return DENOFIZ_FORMAT_SUB_TYPE_TYPELESS;
    case DENOFIZ_FORMAT_R32G32B32A32_UINT:
    case DENOFIZ_FORMAT_R32G32B32_UINT:
    case DENOFIZ_FORMAT_R16G16B16A16_UINT:
    case DENOFIZ_FORMAT_R32G32_UINT:
    case DENOFIZ_FORMAT_R10G10B10A2_UINT:
    case DENOFIZ_FORMAT_R8G8B8A8_UINT:
    case DENOFIZ_FORMAT_R16G16_UINT:
    case DENOFIZ_FORMAT_R32_UINT:
    case DENOFIZ_FORMAT_D24_UNORM_S8_UINT:
    case DENOFIZ_FORMAT_R8G8_UINT:
    case DENOFIZ_FORMAT_R16_UINT:
    case DENOFIZ_FORMAT_R8_UINT:
        return DENOFIZ_FORMAT_SUB_TYPE_UINT;
    case DENOFIZ_FORMAT_R16G16B16A16_SNORM:
    case DENOFIZ_FORMAT_R8G8B8A8_SNORM:
    case DENOFIZ_FORMAT_R16G16_SNORM:
    case DENOFIZ_FORMAT_R8G8_SNORM:
    case DENOFIZ_FORMAT_R16_SNORM:
    case DENOFIZ_FORMAT_R8_SNORM:
    case DENOFIZ_FORMAT_BC4_SNORM:
    case DENOFIZ_FORMAT_BC5_SNORM:
        return DENOFIZ_FORMAT_SUB_TYPE_SNORM;
    case DENOFIZ_FORMAT_R16G16B16A16_UNORM:
    case DENOFIZ_FORMAT_R10G10B10A2_UNORM:
    case DENOFIZ_FORMAT_R8G8B8A8_UNORM:
    case DENOFIZ_FORMAT_R16G16_UNORM:
    case DENOFIZ_FORMAT_R8G8_UNORM:
    case DENOFIZ_FORMAT_D16_UNORM:
    case DENOFIZ_FORMAT_R16_UNORM:
    case DENOFIZ_FORMAT_R8_UNORM:
    case DENOFIZ_FORMAT_BC1_UNORM:
    case DENOFIZ_FORMAT_BC2_UNORM:
    case DENOFIZ_FORMAT_BC3_UNORM:
    case DENOFIZ_FORMAT_BC4_UNORM:
    case DENOFIZ_FORMAT_BC5_UNORM:
    case DENOFIZ_FORMAT_B8G8R8A8_UNORM:
    case DENOFIZ_FORMAT_BC7_UNORM:
    case DENOFIZ_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DENOFIZ_FORMAT_BC1_UNORM_SRGB:
    case DENOFIZ_FORMAT_BC2_UNORM_SRGB:
    case DENOFIZ_FORMAT_BC3_UNORM_SRGB:
    case DENOFIZ_FORMAT_BC7_UNORM_SRGB:
        return DENOFIZ_FORMAT_SUB_TYPE_UNORM;
    }

    return DENOFIZ_FORMAT_SUB_TYPE_UNDEFINED;
}

bool DenOfIz_Format_IsBC( const DenOfIz_Format format )
{
    switch ( format )
    {
    case DENOFIZ_FORMAT_BC1_UNORM:
    case DENOFIZ_FORMAT_BC1_UNORM_SRGB:
    case DENOFIZ_FORMAT_BC2_UNORM:
    case DENOFIZ_FORMAT_BC2_UNORM_SRGB:
    case DENOFIZ_FORMAT_BC3_UNORM:
    case DENOFIZ_FORMAT_BC3_UNORM_SRGB:
    case DENOFIZ_FORMAT_BC4_UNORM:
    case DENOFIZ_FORMAT_BC4_SNORM:
    case DENOFIZ_FORMAT_BC5_UNORM:
    case DENOFIZ_FORMAT_BC5_SNORM:
    case DENOFIZ_FORMAT_BC6H_UFLOAT16:
    case DENOFIZ_FORMAT_BC6H_SFLOAT16:
    case DENOFIZ_FORMAT_BC7_UNORM:
    case DENOFIZ_FORMAT_BC7_UNORM_SRGB:
        return true;
    default:
        return false;
    }
}

uint32_t DenOfIz_Format_BlockSize( const DenOfIz_Format format )
{
    return DenOfIz_Format_IsBC( format ) ? 4 : 1;
}

int DenOfIz_MSAASampleCount_ToNumSamples( const DenOfIz_MSAASampleCount sampleCount )
{
    switch ( sampleCount )
    {
    case DENOFIZ_MSAA_SAMPLE_COUNT_1:
        return 1;
    case DENOFIZ_MSAA_SAMPLE_COUNT_2:
        return 2;
    case DENOFIZ_MSAA_SAMPLE_COUNT_4:
        return 4;
    case DENOFIZ_MSAA_SAMPLE_COUNT_8:
        return 8;
    case DENOFIZ_MSAA_SAMPLE_COUNT_16:
        return 16;
    case DENOFIZ_MSAA_SAMPLE_COUNT_32:
        return 32;
    case DENOFIZ_MSAA_SAMPLE_COUNT_64:
        return 64;
    default:
        return 1;
    }
}

DenOfIz_ResourceBindingType DenOfIz_ResourceBindingType_FromDescriptor( const uint32_t descriptor )
{
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_SAMPLER_BIT )
    {
        return DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER;
    }
    if ( descriptor & ( DENOFIZ_RESOURCE_DESCRIPTOR_UNIFORM_BUFFER_BIT | DENOFIZ_RESOURCE_DESCRIPTOR_ROOT_CONSTANT_BIT ) )
    {
        return DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER;
    }
    if ( descriptor & ( DENOFIZ_RESOURCE_DESCRIPTOR_RW_TEXTURE_BIT | DENOFIZ_RESOURCE_DESCRIPTOR_RW_BUFFER_BIT ) )
    {
        return DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS;
    }

    return DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE;
}

bool DenOfIz_Format_IsDepth( const DenOfIz_Format format )
{
    switch ( format )
    {
    case DENOFIZ_FORMAT_D32_FLOAT:
    case DENOFIZ_FORMAT_D24_UNORM_S8_UINT:
    case DENOFIZ_FORMAT_D16_UNORM:
        return true;
    default:
        return false;
    }
}

bool DenOfIz_Format_IsStencil( const DenOfIz_Format format )
{
    switch ( format )
    {
    case DENOFIZ_FORMAT_D24_UNORM_S8_UINT:
        return true;
    default:
        return false;
    }
}

bool DenOfIz_Format_IsDepthOnly( const DenOfIz_Format format )
{
    switch ( format )
    {
    case DENOFIZ_FORMAT_D32_FLOAT:
    case DENOFIZ_FORMAT_D16_UNORM:
        return true;
    default:
        return false;
    }
}

bool DenOfIz_Format_IsDepthStencil( const DenOfIz_Format format )
{
    switch ( format )
    {
    case DENOFIZ_FORMAT_D24_UNORM_S8_UINT:
        return true;
    default:
        return false;
    }
}

bool DenOfIz_Format_HasStencilComponent( const DenOfIz_Format format )
{
    return DenOfIz_Format_IsStencil( format );
}

uint32_t DenOfIz_ResourceBindingSlot_Key( const DenOfIz_ResourceBindingSlot *slot )
{
    return slot->Type * 1000 + slot->RegisterSpace * 100 + slot->Binding;
}
