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

#include "DenOfIzGraphicsInternal/Assets/Shaders/DxcEnumConverter.h"

using namespace DenOfIz;

ResourceBindingType DxcEnumConverter::ReflectTypeToBufferBindingType( const D3D_SHADER_INPUT_TYPE type )
{
    switch ( type )
    {
    case D3D_SIT_CBUFFER:
        return ResourceBindingType::ConstantBuffer;
    case D3D_SIT_TEXTURE:
        return ResourceBindingType::ShaderResource;
    case D3D_SIT_SAMPLER:
        return ResourceBindingType::Sampler;
    case D3D_SIT_TBUFFER:
    case D3D_SIT_BYTEADDRESS:
    case D3D_SIT_STRUCTURED:
    case D3D_SIT_RTACCELERATIONSTRUCTURE:
        return ResourceBindingType::ShaderResource;
    case D3D_SIT_UAV_APPEND_STRUCTURED:
    case D3D_SIT_UAV_CONSUME_STRUCTURED:
    case D3D_SIT_UAV_RWSTRUCTURED:
    case D3D_SIT_UAV_RWTYPED:
    case D3D_SIT_UAV_RWBYTEADDRESS:
    case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
        return ResourceBindingType::UnorderedAccess;
    case D3D_SIT_UAV_FEEDBACKTEXTURE:
        break;
    }
    LOG( ERROR ) << "Unknown resource type";
    return ResourceBindingType::ConstantBuffer;
}
ReflectionBindingType DxcEnumConverter::ReflectTypeToBufferReflectionBindingType( const D3D_SHADER_INPUT_TYPE type )
{
    switch ( type )
    {
    case D3D_SIT_CBUFFER:
        return ReflectionBindingType::Struct;
    case D3D_SIT_TBUFFER:
        break;
    case D3D_SIT_TEXTURE:
        return ReflectionBindingType::Texture;
    case D3D_SIT_SAMPLER:
        return ReflectionBindingType::SamplerDesc;
    case D3D_SIT_UAV_RWTYPED:
    case D3D_SIT_STRUCTURED:
    case D3D_SIT_UAV_RWSTRUCTURED:
    case D3D_SIT_BYTEADDRESS:
    case D3D_SIT_UAV_RWBYTEADDRESS:
    case D3D_SIT_UAV_APPEND_STRUCTURED:
    case D3D_SIT_UAV_CONSUME_STRUCTURED:
    case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
    case D3D_SIT_RTACCELERATIONSTRUCTURE:
    case D3D_SIT_UAV_FEEDBACKTEXTURE:
        return ReflectionBindingType::Pointer;
    }
    return ReflectionBindingType::Struct;
}

ResourceDescriptor DxcEnumConverter::ReflectTypeToRootSignatureType( const D3D_SHADER_INPUT_TYPE type, const D3D_SRV_DIMENSION dimension )
{
    switch ( type )
    {
    case D3D_SIT_RTACCELERATIONSTRUCTURE:
        return ResourceDescriptor::AccelerationStructure;
    case D3D_SIT_CBUFFER:
        return ResourceDescriptor::UniformBuffer;
    case D3D_SIT_TBUFFER:
    case D3D_SIT_TEXTURE:
        return ResourceDescriptor::Texture;
    case D3D_SIT_SAMPLER:
        return ResourceDescriptor::Sampler;
    case D3D_SIT_BYTEADDRESS:
    case D3D_SIT_STRUCTURED:
        return ResourceDescriptor::Buffer;
    case D3D_SIT_UAV_APPEND_STRUCTURED:
    case D3D_SIT_UAV_CONSUME_STRUCTURED:
    case D3D_SIT_UAV_RWSTRUCTURED:
    case D3D_SIT_UAV_RWTYPED:
    case D3D_SIT_UAV_RWBYTEADDRESS:
    case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
        switch ( dimension )
        {
    case D3D_SRV_DIMENSION_BUFFER:
        return ResourceDescriptor::RWBuffer;
    case D3D_SRV_DIMENSION_TEXTURE1D:
    case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
    case D3D_SRV_DIMENSION_TEXTURE2D:
    case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
    case D3D_SRV_DIMENSION_TEXTURE2DMS:
    case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:
    case D3D_SRV_DIMENSION_TEXTURE3D:
    case D3D_SRV_DIMENSION_TEXTURECUBE:
    case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
        return ResourceDescriptor::RWTexture;
    default:
        break;
        }
        return ResourceDescriptor::RWBuffer;
    case D3D_SIT_UAV_FEEDBACKTEXTURE:
        break;
    }
    LOG( ERROR ) << "Unknown resource type";
    return ResourceDescriptor::Texture;
}

std::string DxcEnumConverter::GetFieldTypeString( const ReflectionFieldType type )
{
    switch ( type )
    {
    case ReflectionFieldType::Undefined:
        return "Undefined";
    case ReflectionFieldType::Void:
        return "Void";
    case ReflectionFieldType::Bool:
        return "Bool";
    case ReflectionFieldType::Int:
        return "Int";
    case ReflectionFieldType::Float:
        return "Float";
    case ReflectionFieldType::String:
        return "String";
    case ReflectionFieldType::Texture:
        return "Texture";
    case ReflectionFieldType::Texture1D:
        return "Texture1D";
    case ReflectionFieldType::Texture2D:
        return "Texture2D";
    case ReflectionFieldType::Texture3D:
        return "Texture3D";
    case ReflectionFieldType::TextureCube:
        return "TextureCube";
    case ReflectionFieldType::Sampler:
        return "Sampler";
    case ReflectionFieldType::Sampler1d:
        return "Sampler1d";
    case ReflectionFieldType::Sampler2d:
        return "Sampler2d";
    case ReflectionFieldType::Sampler3d:
        return "Sampler3d";
    case ReflectionFieldType::SamplerCube:
        return "SamplerCube";
    case ReflectionFieldType::PixelFragment:
        return "PixelFragment";
    case ReflectionFieldType::VertexFragment:
        return "VertexFragment";
    case ReflectionFieldType::Uint:
        return "Uint";
    case ReflectionFieldType::Uint8:
        return "Uint8";
    case ReflectionFieldType::DepthStencil:
        return "DepthStencil";
    case ReflectionFieldType::Blend:
        return "Blend";
    case ReflectionFieldType::Buffer:
        return "Buffer";
    case ReflectionFieldType::CBuffer:
        return "CBuffer";
    case ReflectionFieldType::TBuffer:
        return "TBuffer";
    case ReflectionFieldType::Texture1DArray:
        return "Texture1DArray";
    case ReflectionFieldType::Texture2DArray:
        return "Texture2DArray";
    case ReflectionFieldType::RenderTargetView:
        return "RenderTargetView";
    case ReflectionFieldType::DepthStencilView:
        return "DepthStencilView";
    case ReflectionFieldType::Texture2Dms:
        return "Texture2Dms";
    case ReflectionFieldType::Texture2DmsArray:
        return "Texture2DmsArray";
    case ReflectionFieldType::TextureCubeArray:
        return "TextureCubeArray";
    case ReflectionFieldType::InterfacePointer:
        return "InterfacePointer";
    case ReflectionFieldType::Double:
        return "Double";
    case ReflectionFieldType::RWTexture1D:
        return "RWTexture1D";
    case ReflectionFieldType::RWTexture1DArray:
        return "RWTexture1DArray";
    case ReflectionFieldType::RWTexture2D:
        return "RWTexture2D";
    case ReflectionFieldType::RWTexture2DArray:
        return "RWTexture2DArray";
    case ReflectionFieldType::RWTexture3D:
        return "RWTexture3D";
    case ReflectionFieldType::RWBuffer:
        return "RWBuffer";
    case ReflectionFieldType::ByteAddressBuffer:
        return "ByteAddressBuffer";
    case ReflectionFieldType::RWByteAddressBuffer:
        return "RWByteAddressBuffer";
    case ReflectionFieldType::StructuredBuffer:
        return "StructuredBuffer";
    case ReflectionFieldType::RWStructuredBuffer:
        return "RWStructuredBuffer";
    case ReflectionFieldType::AppendStructuredBuffer:
        return "AppendStructuredBuffer";
    case ReflectionFieldType::ConsumeStructuredBuffer:
        return "ConsumeStructuredBuffer";
    case ReflectionFieldType::Min8Float:
        return "Min8Float";
    case ReflectionFieldType::Min10Float:
        return "Min10Float";
    case ReflectionFieldType::Min16Float:
        return "Min16Float";
    case ReflectionFieldType::Min12Int:
        return "Min12Int";
    case ReflectionFieldType::Min16Int:
        return "Min16Int";
    case ReflectionFieldType::Min16UInt:
        return "Min16UInt";
    case ReflectionFieldType::Int16:
        return "Int16";
    case ReflectionFieldType::UInt16:
        return "UInt16";
    case ReflectionFieldType::Float16:
        return "Float16";
    case ReflectionFieldType::Int64:
        return "Int64";
    case ReflectionFieldType::UInt64:
        return "UInt64";
    case ReflectionFieldType::PixelShader:
        return "PixelShader";
    case ReflectionFieldType::VertexShader:
        return "VertexShader";
    case ReflectionFieldType::GeometryShader:
        return "GeometryShader";
    case ReflectionFieldType::HullShader:
        return "HullShader";
    case ReflectionFieldType::DomainShader:
        return "DomainShader";
    case ReflectionFieldType::ComputeShader:
        return "ComputeShader";
    }

    return "";
}

std::string DxcEnumConverter::GetBindingTypeString( const ResourceBindingType type )
{
    switch ( type )
    {
    case ResourceBindingType::ConstantBuffer:
        return "CBV";
    case ResourceBindingType::ShaderResource:
        return "SRV";
    case ResourceBindingType::UnorderedAccess:
        return "UAV";
    case ResourceBindingType::Sampler:
        return "Sampler";
    default:
        return "Unknown";
    }
}

std::string DxcEnumConverter::GetStagesString( const InteropArray<ShaderStage> &stages )
{
    std::string result;
    for ( int i = 0; i < stages.NumElements( ); ++i )
    {
        if ( i > 0 )
            result += "|";
        switch ( stages.GetElement( i ) )
        {
        case ShaderStage::Vertex:
            result += "Vertex";
            break;
        case ShaderStage::Pixel:
            result += "Pixel";
            break;
        case ShaderStage::Compute:
            result += "Compute";
            break;
        case ShaderStage::Raygen:
            result += "Raygen";
            break;
        case ShaderStage::ClosestHit:
            result += "ClosestHit";
            break;
        case ShaderStage::Geometry:
            result += "Geometry";
            break;
        case ShaderStage::Hull:
            result += "Hull";
            break;
        case ShaderStage::Domain:
            result += "Domain";
            break;
        case ShaderStage::AllGraphics:
            result += "AllGraphics";
            break;
        case ShaderStage::All:
            result += "All";
            break;
        case ShaderStage::AnyHit:
            result += "AnyHit";
            break;
        case ShaderStage::Miss:
            result += "Miss";
            break;
        case ShaderStage::Intersection:
            result += "Intersection";
            break;
        case ShaderStage::Callable:
            result += "Callable";
            break;
        case ShaderStage::Task:
            result += "Task";
            break;
        case ShaderStage::Mesh:
            result += "Mesh";
            break;
        }
    }
    return result;
}

ReflectionFieldType DxcEnumConverter::VariableTypeToReflectionType( const D3D_SHADER_VARIABLE_TYPE &type )
{
    switch ( type )
    {
    case D3D_SVT_VOID:
        return ReflectionFieldType::Void;
    case D3D_SVT_BOOL:
        return ReflectionFieldType::Bool;
    case D3D_SVT_INT:
        return ReflectionFieldType::Int;
    case D3D_SVT_FLOAT:
        return ReflectionFieldType::Float;
    case D3D_SVT_STRING:
        return ReflectionFieldType::String;
    case D3D_SVT_TEXTURE:
        return ReflectionFieldType::Texture;
    case D3D_SVT_TEXTURE1D:
        return ReflectionFieldType::Texture1D;
    case D3D_SVT_TEXTURE2D:
        return ReflectionFieldType::Texture2D;
    case D3D_SVT_TEXTURE3D:
        return ReflectionFieldType::Texture3D;
    case D3D_SVT_TEXTURECUBE:
        return ReflectionFieldType::TextureCube;
    case D3D_SVT_SAMPLER:
        return ReflectionFieldType::Sampler;
    case D3D_SVT_SAMPLER1D:
        return ReflectionFieldType::Sampler1d;
    case D3D_SVT_SAMPLER2D:
        return ReflectionFieldType::Sampler2d;
    case D3D_SVT_SAMPLER3D:
        return ReflectionFieldType::Sampler3d;
    case D3D_SVT_SAMPLERCUBE:
        return ReflectionFieldType::SamplerCube;
    case D3D_SVT_PIXELFRAGMENT:
        return ReflectionFieldType::PixelFragment;
    case D3D_SVT_VERTEXFRAGMENT:
        return ReflectionFieldType::VertexFragment;
    case D3D_SVT_UINT:
        return ReflectionFieldType::Uint;
    case D3D_SVT_UINT8:
        return ReflectionFieldType::Uint8;
    case D3D_SVT_DEPTHSTENCIL:
        return ReflectionFieldType::DepthStencil;
    case D3D_SVT_BLEND:
        return ReflectionFieldType::Blend;
    case D3D_SVT_BUFFER:
        return ReflectionFieldType::Buffer;
    case D3D_SVT_CBUFFER:
        return ReflectionFieldType::CBuffer;
    case D3D_SVT_TBUFFER:
        return ReflectionFieldType::TBuffer;
    case D3D_SVT_TEXTURE1DARRAY:
        return ReflectionFieldType::Texture1DArray;
    case D3D_SVT_TEXTURE2DARRAY:
        return ReflectionFieldType::Texture2DArray;
    case D3D_SVT_RENDERTARGETVIEW:
        return ReflectionFieldType::RenderTargetView;
    case D3D_SVT_DEPTHSTENCILVIEW:
        return ReflectionFieldType::DepthStencilView;
    case D3D_SVT_TEXTURE2DMS:
        return ReflectionFieldType::Texture2Dms;
    case D3D_SVT_TEXTURE2DMSARRAY:
        return ReflectionFieldType::Texture2DmsArray;
    case D3D_SVT_TEXTURECUBEARRAY:
        return ReflectionFieldType::TextureCubeArray;
    case D3D_SVT_INTERFACE_POINTER:
        return ReflectionFieldType::InterfacePointer;
    case D3D_SVT_DOUBLE:
        return ReflectionFieldType::Double;
    case D3D_SVT_RWTEXTURE1D:
        return ReflectionFieldType::RWTexture1D;
    case D3D_SVT_RWTEXTURE1DARRAY:
        return ReflectionFieldType::RWTexture1DArray;
    case D3D_SVT_RWTEXTURE2D:
        return ReflectionFieldType::RWTexture2D;
    case D3D_SVT_RWTEXTURE2DARRAY:
        return ReflectionFieldType::RWTexture2DArray;
    case D3D_SVT_RWTEXTURE3D:
        return ReflectionFieldType::RWTexture3D;
    case D3D_SVT_RWBUFFER:
        return ReflectionFieldType::RWBuffer;
    case D3D_SVT_BYTEADDRESS_BUFFER:
        return ReflectionFieldType::ByteAddressBuffer;
    case D3D_SVT_RWBYTEADDRESS_BUFFER:
        return ReflectionFieldType::RWByteAddressBuffer;
    case D3D_SVT_STRUCTURED_BUFFER:
        return ReflectionFieldType::StructuredBuffer;
    case D3D_SVT_RWSTRUCTURED_BUFFER:
        return ReflectionFieldType::RWStructuredBuffer;
    case D3D_SVT_APPEND_STRUCTURED_BUFFER:
        return ReflectionFieldType::AppendStructuredBuffer;
    case D3D_SVT_CONSUME_STRUCTURED_BUFFER:
        return ReflectionFieldType::ConsumeStructuredBuffer;
    case D3D_SVT_MIN8FLOAT:
        return ReflectionFieldType::Min8Float;
    case D3D_SVT_MIN10FLOAT:
        return ReflectionFieldType::Min10Float;
    case D3D_SVT_MIN16FLOAT:
        return ReflectionFieldType::Min16Float;
    case D3D_SVT_MIN12INT:
        return ReflectionFieldType::Min12Int;
    case D3D_SVT_MIN16INT:
        return ReflectionFieldType::Min16Int;
    case D3D_SVT_MIN16UINT:
        return ReflectionFieldType::Min16UInt;
    case D3D_SVT_INT16:
        return ReflectionFieldType::Int16;
    case D3D_SVT_UINT16:
        return ReflectionFieldType::UInt16;
    case D3D_SVT_FLOAT16:
        return ReflectionFieldType::Float16;
    case D3D_SVT_INT64:
        return ReflectionFieldType::Int64;
    case D3D_SVT_UINT64:
        return ReflectionFieldType::UInt64;
    case D3D_SVT_PIXELSHADER:
        return ReflectionFieldType::PixelShader;
    case D3D_SVT_VERTEXSHADER:
        return ReflectionFieldType::VertexShader;
    case D3D_SVT_GEOMETRYSHADER:
        return ReflectionFieldType::GeometryShader;
    case D3D_SVT_HULLSHADER:
        return ReflectionFieldType::HullShader;
    case D3D_SVT_DOMAINSHADER:
        return ReflectionFieldType::DomainShader;
    case D3D_SVT_COMPUTESHADER:
        return ReflectionFieldType::ComputeShader;
    default:
        return ReflectionFieldType::Undefined;
    }
}

#if defined(_WIN32) || defined(__APPLE__) // TODO metal shader converter on linux: not yet supported
IRShaderVisibility DxcEnumConverter::ShaderStageToShaderVisibility( ShaderStage stage )
{
    switch ( stage )
    {
    case ShaderStage::Vertex:
        return IRShaderVisibilityVertex;
    case ShaderStage::Pixel:
        return IRShaderVisibilityPixel;
    case ShaderStage::Hull:
        return IRShaderVisibilityHull;
    case ShaderStage::Domain:
        return IRShaderVisibilityDomain;
    case ShaderStage::Geometry:
        return IRShaderVisibilityGeometry;
    case ShaderStage::Compute:
        return IRShaderVisibilityAll;
    default:
        return IRShaderVisibilityAll;
    }
}

IRRootParameterType DxcEnumConverter::BindingTypeToIRRootParameterType( const ResourceBindingType &type )
{
    switch ( type )
    {
    case ResourceBindingType::ConstantBuffer:
        return IRRootParameterTypeCBV;
    case ResourceBindingType::ShaderResource:
        return IRRootParameterTypeSRV;
    case ResourceBindingType::UnorderedAccess:
        return IRRootParameterTypeUAV;
    default:
        break;
    }

    return IRRootParameterTypeCBV;
}

IRRootParameterType DxcEnumConverter::IRDescriptorRangeTypeToIRRootParameterType( const IRDescriptorRangeType &type )
{
    switch ( type )
    {
    case IRDescriptorRangeTypeCBV:
        return IRRootParameterTypeCBV;
    case IRDescriptorRangeTypeSRV:
        return IRRootParameterTypeSRV;
    case IRDescriptorRangeTypeUAV:
        return IRRootParameterTypeUAV;
    default:
        break;
    }

    return IRRootParameterTypeCBV;
}

IRDescriptorRangeType DxcEnumConverter::ShaderTypeToIRDescriptorType( const D3D_SHADER_INPUT_TYPE &type )
{
    IRDescriptorRangeType descriptorRangeType = IRDescriptorRangeTypeCBV;
    switch ( type )
    {
    case D3D_SIT_CBUFFER:
    case D3D_SIT_TBUFFER:
        descriptorRangeType = IRDescriptorRangeTypeCBV;
        break;
    case D3D_SIT_TEXTURE:
    case D3D_SIT_STRUCTURED:
    case D3D_SIT_BYTEADDRESS:
    case D3D_SIT_RTACCELERATIONSTRUCTURE:
        descriptorRangeType = IRDescriptorRangeTypeSRV;
        break;
    case D3D_SIT_SAMPLER:
        descriptorRangeType = IRDescriptorRangeTypeSampler;
        break;
    case D3D_SIT_UAV_APPEND_STRUCTURED:
    case D3D_SIT_UAV_CONSUME_STRUCTURED:
    case D3D_SIT_UAV_RWSTRUCTURED:
    case D3D_SIT_UAV_RWTYPED:
    case D3D_SIT_UAV_RWBYTEADDRESS:
    case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
    case D3D_SIT_UAV_FEEDBACKTEXTURE:
        descriptorRangeType = IRDescriptorRangeTypeUAV;
        break;
    default:
        LOG( ERROR ) << "Unknown resource type";
        break;
    }
    return descriptorRangeType;
}
#endif
