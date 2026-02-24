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
#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

DenOfIz_ResourceBindingType DxcEnumConverter::ReflectTypeToBufferBindingType( const D3D_SHADER_INPUT_TYPE type )
{
    switch ( type )
    {
    case D3D_SIT_CBUFFER:
        return DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER;
    case D3D_SIT_TEXTURE:
        return DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE;
    case D3D_SIT_SAMPLER:
        return DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER;
    case D3D_SIT_TBUFFER:
    case D3D_SIT_BYTEADDRESS:
    case D3D_SIT_STRUCTURED:
    case D3D_SIT_RTACCELERATIONSTRUCTURE:
        return DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE;
    case D3D_SIT_UAV_APPEND_STRUCTURED:
    case D3D_SIT_UAV_CONSUME_STRUCTURED:
    case D3D_SIT_UAV_RWSTRUCTURED:
    case D3D_SIT_UAV_RWTYPED:
    case D3D_SIT_UAV_RWBYTEADDRESS:
    case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
        return DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS;
    case D3D_SIT_UAV_FEEDBACKTEXTURE:
        break;
    }
    spdlog::error( "Unknown resource type" );
    return DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER;
}

uint32_t DxcEnumConverter::ReflectTypeToRootSignatureType( const D3D_SHADER_INPUT_TYPE type, const D3D_SRV_DIMENSION dimension )
{
    switch ( type )
    {
    case D3D_SIT_RTACCELERATIONSTRUCTURE:
        return DENOFIZ_RESOURCE_DESCRIPTOR_ACCELERATION_STRUCTURE_BIT;
    case D3D_SIT_CBUFFER:
        return DENOFIZ_RESOURCE_DESCRIPTOR_UNIFORM_BUFFER_BIT;
    case D3D_SIT_TBUFFER:
    case D3D_SIT_TEXTURE:
        return DENOFIZ_RESOURCE_DESCRIPTOR_TEXTURE_BIT;
    case D3D_SIT_SAMPLER:
        return DENOFIZ_RESOURCE_DESCRIPTOR_SAMPLER_BIT;
    case D3D_SIT_BYTEADDRESS:
    case D3D_SIT_STRUCTURED:
        return DENOFIZ_RESOURCE_DESCRIPTOR_BUFFER_BIT;
    case D3D_SIT_UAV_APPEND_STRUCTURED:
    case D3D_SIT_UAV_CONSUME_STRUCTURED:
    case D3D_SIT_UAV_RWSTRUCTURED:
    case D3D_SIT_UAV_RWTYPED:
    case D3D_SIT_UAV_RWBYTEADDRESS:
    case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
        switch ( dimension )
        {
        case D3D_SRV_DIMENSION_BUFFER:
            return DENOFIZ_RESOURCE_DESCRIPTOR_RW_BUFFER_BIT;
        case D3D_SRV_DIMENSION_TEXTURE1D:
        case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
        case D3D_SRV_DIMENSION_TEXTURE2D:
        case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
        case D3D_SRV_DIMENSION_TEXTURE2DMS:
        case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:
        case D3D_SRV_DIMENSION_TEXTURE3D:
        case D3D_SRV_DIMENSION_TEXTURECUBE:
        case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
            return DENOFIZ_RESOURCE_DESCRIPTOR_RW_TEXTURE_BIT;
        default:
            break;
        }
        return DENOFIZ_RESOURCE_DESCRIPTOR_RW_BUFFER_BIT;
    case D3D_SIT_UAV_FEEDBACKTEXTURE:
        break;
    }
    spdlog::error( "Unknown resource type" );
    return DENOFIZ_RESOURCE_DESCRIPTOR_TEXTURE_BIT;
}

std::string DxcEnumConverter::GetBindingTypeString( const DenOfIz_ResourceBindingType type )
{
    switch ( type )
    {
    case DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER:
        return "CBV";
    case DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE:
        return "SRV";
    case DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS:
        return "UAV";
    case DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER:
        return "Sampler";
    default:
        return "Unknown";
    }
}

std::string DxcEnumConverter::GetStagesString( const DenOfIz_ShaderStageFlags stages )
{
    std::string result;

    auto appendStage = [ &result ]( const char *name )
    {
        if ( !result.empty( ) )
        {
            result += "|";
        }
        result += name;
    };

    if ( stages & DENOFIZ_SHADER_STAGE_VERTEX_BIT )
    {
        appendStage( "Vertex" );
    }
    if ( stages & DENOFIZ_SHADER_STAGE_PIXEL_BIT )
    {
        appendStage( "Pixel" );
    }
    if ( stages & DENOFIZ_SHADER_STAGE_GEOMETRY_BIT )
    {
        appendStage( "Geometry" );
    }
    if ( stages & DENOFIZ_SHADER_STAGE_HULL_BIT )
    {
        appendStage( "Hull" );
    }
    if ( stages & DENOFIZ_SHADER_STAGE_DOMAIN_BIT )
    {
        appendStage( "Domain" );
    }
    if ( stages & DENOFIZ_SHADER_STAGE_COMPUTE_BIT )
    {
        appendStage( "Compute" );
    }
    if ( stages & DENOFIZ_SHADER_STAGE_RAY_GEN_BIT )
    {
        appendStage( "Raygen" );
    }
    if ( stages & DENOFIZ_SHADER_STAGE_ANY_HIT_BIT )
    {
        appendStage( "AnyHit" );
    }
    if ( stages & DENOFIZ_SHADER_STAGE_CLOSEST_HIT_BIT )
    {
        appendStage( "ClosestHit" );
    }
    if ( stages & DENOFIZ_SHADER_STAGE_MISS_BIT )
    {
        appendStage( "Miss" );
    }
    if ( stages & DENOFIZ_SHADER_STAGE_INTERSECTION_BIT )
    {
        appendStage( "Intersection" );
    }
    if ( stages & DENOFIZ_SHADER_STAGE_CALLABLE_BIT )
    {
        appendStage( "Callable" );
    }
    if ( stages & DENOFIZ_SHADER_STAGE_TASK_BIT )
    {
        appendStage( "Task" );
    }
    if ( stages & DENOFIZ_SHADER_STAGE_MESH_BIT )
    {
        appendStage( "Mesh" );
    }

    return result;
}

#if defined( _WIN32 ) || defined( __APPLE__ ) // TODO metal shader converter on linux: not yet supported
IRShaderVisibility DxcEnumConverter::ShaderStageToShaderVisibility( DenOfIz_ShaderStageFlags stage )
{
    if ( stage == DENOFIZ_SHADER_STAGE_VERTEX_BIT )
    {
        return IRShaderVisibilityVertex;
    }
    if ( stage == DENOFIZ_SHADER_STAGE_PIXEL_BIT )
    {
        return IRShaderVisibilityPixel;
    }
    if ( stage == DENOFIZ_SHADER_STAGE_HULL_BIT )
    {
        return IRShaderVisibilityHull;
    }
    if ( stage == DENOFIZ_SHADER_STAGE_DOMAIN_BIT )
    {
        return IRShaderVisibilityDomain;
    }
    if ( stage == DENOFIZ_SHADER_STAGE_GEOMETRY_BIT )
    {
        return IRShaderVisibilityGeometry;
    }
    return IRShaderVisibilityAll;
}

IRRootParameterType DxcEnumConverter::BindingTypeToIRRootParameterType( const DenOfIz_ResourceBindingType &type )
{
    switch ( type )
    {
    case DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER:
        return IRRootParameterTypeCBV;
    case DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE:
        return IRRootParameterTypeSRV;
    case DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS:
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
        spdlog::error( "Unknown resource type" );
        break;
    }
    return descriptorRangeType;
}
#endif
