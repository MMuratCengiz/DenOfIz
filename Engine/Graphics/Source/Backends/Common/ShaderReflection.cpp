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

#include <DenOfIzGraphics/Backends/Common/ShaderReflection.h>
#include <DenOfIzGraphics/Backends/Interface/IShader.h>

using namespace DenOfIz;

ShaderReflection::ShaderReflection(std::vector<CompiledShader> shaderInfos) : Shaders(std::move(shaderInfos))
{
    bool first = true;
    for ( const CompiledShader &shaderInfo : Shaders )
    {
        OnEachShader(shaderInfo, first);
        first = false;
    }
}

void ShaderReflection::OnEachShader(const CompiledShader &shaderInfo, const bool &first)
{
    CComPtr<IDxcBlob> code = shaderInfo.Data;
    std::vector<uint32_t> codeToUVec(static_cast<uint32_t *>(code->GetBufferPointer()),
                                     static_cast<uint32_t *>(code->GetBufferPointer()) + code->GetBufferSize() / sizeof(uint32_t));

    spirv_cross::Compiler compiler(codeToUVec);

    auto shaderResources = compiler.get_shader_resources();

    auto stageInputs = shaderResources.stage_inputs;
    auto samplers = shaderResources.sampled_images;
    auto uniforms = shaderResources.uniform_buffers;
    auto shaderPushConstants = shaderResources.push_constant_buffers;

    uint32_t offsetIter = 0;

    if ( first )
    {
        std::sort(stageInputs.begin(), stageInputs.end(),
                  [ & ](const spirv_cross::Resource &r1, const spirv_cross::Resource &r2) -> bool
                  {
                      SpvDecoration decoration1 = GetDecoration(compiler, r1);
                      SpvDecoration decoration2 = GetDecoration(compiler, r2);
                      return decoration1.Location < decoration2.Location;
                  });

        uint32_t index;
        for ( const spirv_cross::Resource &resource : stageInputs )
        {
            SpvDecoration decoration = GetDecoration(compiler, resource);
            ShaderVarType gType = SpvTypeToCustomType(decoration.Type);
            CreateVertexInput(offsetIter, gType, decoration, gType.Size);
            offsetIter += gType.Size;
        }
    }

    for ( const spirv_cross::Resource &resource : samplers )
    {
        CreateUniformInput(compiler, UniformType::Sampler, resource, shaderInfo.Stage);
    }

    for ( const spirv_cross::Resource &resource : uniforms )
    {
        CreateUniformInput(compiler, UniformType::Struct, resource, shaderInfo.Stage);
    }

    for ( const spirv_cross::Resource &resource : shaderPushConstants )
    {
        CreatePushConstant(compiler, resource, shaderInfo.Stage);
    }
}

void ShaderReflection::CreateVertexInput(const uint32_t &offset, const ShaderVarType &type, const SpvDecoration &decoration, const uint32_t &size)
{
    VertexInput &input = vertexInputs.emplace_back(VertexInput{});
    input.Location = decoration.Location;
    input.Format = type.Format;
    input.Offset = offset;
    input.Size = size;
    input.Name = decoration.Name;
}

void ShaderReflection::CreateUniformInput(const spirv_cross::Compiler &compiler, const UniformType &uniformType, const spirv_cross::Resource resource, const ShaderStage stage)
{
    SpvDecoration decoration = GetDecoration(compiler, resource);
    if ( decoration.Name == "type.$Globals" )
    {
        // Todo recursively read all children instead?
        for ( SpvDecoration child : decoration.Children )
        {
            AddResourceToInput(uniformType, stage, child);
        }
    }
    else
    {
        AddResourceToInput(uniformType, stage, decoration);
    }
}

void ShaderReflection::AddResourceToInput(const UniformType &uniformType, const ShaderStage &stage, const SpvDecoration &decoration)
{
    ShaderUniformInput &uniformInput = uniformInputs.emplace_back(ShaderUniformInput{});

    uniformInput.Name = decoration.Name;
    uniformInput.Location = decoration.Location;
    uniformInput.BoundDescriptorSet = decoration.Set;
    uniformInput.Stage = stage;
    uniformInput.Binding = decoration.Binding;
    uniformInput.ArraySize = decoration.ArraySize;
    uniformInput.Size = decoration.Size;
    uniformInput.Type = uniformType;
    uniformInput.Format = SpvTypeToCustomType(decoration.Type).Format;
}

void ShaderReflection::CreatePushConstant(const spirv_cross::Compiler &compiler, const spirv_cross::Resource resource, const ShaderStage stage)
{
    SpvDecoration decoration = GetDecoration(compiler, resource);

    PushConstant &pushConstant = pushConstants.emplace_back(PushConstant{});
    pushConstant.Offset = 0; // TODO Could a push constant have any other offset?
    pushConstant.Size = decoration.Size;
    pushConstant.Stage = stage;
    pushConstant.Name = decoration.Name;
    pushConstant.Children = std::move(decoration.Children);
}

ShaderVarType ShaderReflection::SpvTypeToCustomType(const spirv_cross::SPIRType &type)
{
    auto format = ImageFormat::Undefined;
    uint32_t size = 0;

    auto make32Int = [](const uint32_t &numOfElements) -> ImageFormat
    {
        if ( numOfElements == 1 )
            return ImageFormat::R32Sint;
        if ( numOfElements == 2 )
            return ImageFormat::R32G32Sint;
        if ( numOfElements == 3 )
            return ImageFormat::R32G32B32Sint;
        if ( numOfElements == 4 )
            return ImageFormat::R32G32B32A32Sint;
        return ImageFormat::Undefined;
    };

#if NO_64_BIT_EXISTS_IN_HLSL_GLSL
    auto make64Int = [](const uint32_t &numOfElements) -> ImageFormat
    {
        if ( numOfElements == 1 )
            return ImageFormat::R64Sint;
        if ( numOfElements == 2 )
            return ImageFormat::R64G64Sint;
        if ( numOfElements == 3 )
            return ImageFormat::R64G64B64Sint;
        if ( numOfElements == 4 )
            return ImageFormat::R64G64B64A64Sint;
        return ImageFormat::Undefined;
    };
#endif

    auto make32UInt = [](const uint32_t &numOfElements) -> ImageFormat
    {
        if ( numOfElements == 1 )
            return ImageFormat::R32Uint;
        if ( numOfElements == 2 )
            return ImageFormat::R32G32Uint;
        if ( numOfElements == 3 )
            return ImageFormat::R32G32B32Uint;
        if ( numOfElements == 4 )
            return ImageFormat::R32G32B32A32Uint;
        return ImageFormat::Undefined;
    };

    auto make32Float = [](const uint32_t &numOfElements) -> ImageFormat
    {
        if ( numOfElements == 1 )
            return ImageFormat::R32Float;
        if ( numOfElements == 2 )
            return ImageFormat::R32G32Float;
        if ( numOfElements == 3 )
            return ImageFormat::R32G32B32Float;
        if ( numOfElements == 4 )
            return ImageFormat::R32G32B32A32Float;
        return ImageFormat::Undefined;
    };

    // 64 bit types not supported by DX12
    switch ( type.basetype )
    {
    default:
        break;
    case spirv_cross::SPIRType::Short:
    case spirv_cross::SPIRType::UShort:
    case spirv_cross::SPIRType::Int:
        format = make32Int(type.vecsize);
        size = sizeof(int32_t);
        break;
    case spirv_cross::SPIRType::Int64:
        format = make32Int(type.vecsize);
        size = sizeof(int64_t);
        break;
    case spirv_cross::SPIRType::UInt:
    case spirv_cross::SPIRType::UInt64:
        format = make32UInt(type.vecsize);
        size = sizeof(uint32_t);
        break;
    case spirv_cross::SPIRType::Double:
    case spirv_cross::SPIRType::Float:
        format = make32Float(type.vecsize);
        size = sizeof(float);
        break;
        size = sizeof(double);
        break;
    }

    return ShaderVarType{ format, size * type.vecsize };
}

SpvDecoration ShaderReflection::GetDecoration(const spirv_cross::Compiler &compiler, const spirv_cross::Resource &resource)
{
    SpvDecoration decoration{ .Type = compiler.get_type(resource.type_id), .Set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet) };

    if ( decoration.Type.basetype == spirv_cross::SPIRType::Struct )
    {
        uint32_t structSize = static_cast<uint32_t>(compiler.get_declared_struct_size(decoration.Type));
        // Doesn't seem necessary at the moment:
        // uint32_t m1_size = compiler.get_declared_struct_member_size( decoration.type, 0 );
        uint32_t offsetIter = 0;

        for ( uint32_t i = 0; offsetIter != structSize; i++ )
        {
            uint32_t size = static_cast<uint32_t>(compiler.get_declared_struct_member_size(decoration.Type, i));

            auto &child = decoration.Children.emplace_back(SpvDecoration{
                .Type = compiler.get_type(decoration.Type.member_types[ i ]),
                .Set = decoration.Set,
            });

            child.Offset = offsetIter;
            child.Size = size;
            child.Name = compiler.get_member_name(resource.base_type_id, i);
            child.Location = decoration.Location;
            child.Binding = decoration.Binding;
            child.ArraySize = GetTypeArraySize(child);
            offsetIter += size;
        }

        decoration.Size = structSize;
    }

    decoration.Location = compiler.get_decoration(resource.id, spv::DecorationLocation);
    decoration.Binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

    uint32_t totalArraySize = GetTypeArraySize(decoration);

    decoration.ArraySize = totalArraySize;
    decoration.Name = resource.name;
    return decoration;
}

uint32_t ShaderReflection::GetTypeArraySize(SpvDecoration &decoration) const
{
    uint32_t totalArraySize = 0;
    for ( uint32_t dimensionSize : decoration.Type.array )
    {
        totalArraySize += dimensionSize;
    }
    totalArraySize = totalArraySize == 0 ? 1 : decoration.Size / totalArraySize;
    return totalArraySize;
}
