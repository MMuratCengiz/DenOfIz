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

#include "DenOfIzGraphicsInternal/Utilities/ContainerUtilities.h"
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetReader.h"
#include "DenOfIzGraphics/Assets/Shaders/DxcEnumConverter.h"
#include "DenOfIzGraphics/Assets/Shaders/DxilToMsl.h"
#include "DenOfIzGraphics/Assets/Shaders/ReflectionDebugOutput.h"
#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"
#include <ranges>
#include <set>
#include <utility>

using namespace DenOfIz;

#define DXC_CHECK_RESULT( result )                                                                                                                                                 \
    do                                                                                                                                                                             \
    {                                                                                                                                                                              \
        if ( FAILED( result ) )                                                                                                                                                    \
        {                                                                                                                                                                          \
            LOG( ERROR ) << "DXC Error: " << result;                                                                                                                               \
        }                                                                                                                                                                          \
    }                                                                                                                                                                              \
    while ( false )

ShaderProgram::ShaderProgram( ShaderProgramDesc desc ) : m_desc( std::move( desc ) )
{
    Compile( );
    CreateReflectionData( );
}

ShaderProgram::ShaderProgram( const ShaderAsset &asset )
{
    CompiledShader shader = ShaderAssetReader::ConvertToCompiledShader( asset );
    for ( int i = 0; i < shader.Stages.NumElements( ); ++i )
    {
        CompiledShaderStage *shaderStage = shader.Stages.GetElement( i );
        m_compiledShaders.emplace_back( std::unique_ptr<CompiledShaderStage>( shaderStage ) );
    }
    m_reflectDesc     = shader.ReflectDesc;
    m_desc            = { };
    m_desc.RayTracing = shader.RayTracing;
}

/**
 * \brief Compiles the shaders targeting MSL/DXIL/SPIR-V. MSL is double compiled, first time to DXIL and reflect and provide a root signature to the second compilation.
 */
void ShaderProgram::Compile( )
{
    std::vector<CompiledShaderStage *> dxilShaders;

    for ( int i = 0; i < m_desc.ShaderStages.NumElements( ); ++i )
    {
        const auto &stage = m_desc.ShaderStages.GetElement( i );
        // Validate Shader
        if ( stage.Path.IsEmpty( ) && stage.Data.NumElements( ) == 0 )
        {
            LOG( ERROR ) << "Either stage.Path or stage.Data must be set for stage " << i << "";
            continue;
        }

        CompileDesc compileDesc = { };
        compileDesc.Path        = stage.Path;
        compileDesc.CodePage    = stage.CodePage;
        compileDesc.Data        = stage.Data;
        compileDesc.Defines     = stage.Defines;
        compileDesc.EntryPoint  = stage.EntryPoint;
        compileDesc.Stage       = stage.Stage;
        compileDesc.TargetIL    = TargetIL::DXIL;

        auto [ dxil, reflection ] = m_compiler.CompileHLSL( compileDesc );

        compileDesc.TargetIL = TargetIL::SPIRV;
        auto [ spirv, _ ]    = m_compiler.CompileHLSL( compileDesc );

        const auto &compiledShader = m_compiledShaders.emplace_back( std::make_unique<CompiledShaderStage>( ) );
        compiledShader->Stage      = stage.Stage;
        compiledShader->EntryPoint = stage.EntryPoint;
        compiledShader->RayTracing = stage.RayTracing;
        compiledShader->Reflection = reflection;
        compiledShader->DXIL       = std::move( dxil );
        compiledShader->SPIRV      = std::move( spirv );
        compiledShader->MSL        = { }; // Set below

        m_shaderDescs.push_back( stage );
    }

#if defined( _WIN32 ) || defined( __APPLE__ ) // TODO metal shader converter on linux: not yet supported
    DxilToMslDesc dxilToMslDesc{ };
    dxilToMslDesc.Shaders    = m_desc.ShaderStages;
    dxilToMslDesc.RayTracing = m_desc.RayTracing;

    for ( auto &shader : m_compiledShaders )
    {
        dxilToMslDesc.DXILShaders.AddElement( shader.get( ) );
    }

    DxilToMsl dxilToMsl{ };
    auto      mslShaders = dxilToMsl.Convert( dxilToMslDesc );
    if ( mslShaders.NumElements( ) != m_desc.ShaderStages.NumElements( ) )
    {
        LOG( ERROR ) << "Num DXIL shaders != Num MSL Shaders, probable bug in DxilToMsl";
        return;
    }

    for ( int i = 0; i < mslShaders.NumElements( ); ++i )
    {
        m_compiledShaders[ i ]->MSL = std::move( mslShaders.GetElement( i ) );
    }
#else
    LOG( ERROR ) << "MSL compilation is not supported on this platform";
#endif
}

void ShaderProgram::CreateReflectionData( )
{
    m_reflectDesc = { };
    m_reflectDesc.LocalRootSignatures.Resize( m_compiledShaders.size( ) );
    m_reflectDesc.ThreadGroups.Resize( m_compiledShaders.size( ) );

    InputLayoutDesc   &inputLayout   = m_reflectDesc.InputLayout;
    RootSignatureDesc &rootSignature = m_reflectDesc.RootSignature;

    // TODO These don't really need to be stored this way
    std::vector<uint32_t> descriptorTableLocations;
    std::vector<uint32_t> localDescriptorTableLocations;

    ReflectionState reflectionState   = { };
    reflectionState.RootSignatureDesc = &rootSignature;
    reflectionState.InputLayoutDesc   = &inputLayout;

    for ( int stageIndex = 0; stageIndex < m_compiledShaders.size( ); ++stageIndex )
    {
        auto &shader                         = m_compiledShaders[ stageIndex ];
        reflectionState.CompiledShader       = shader.get( );
        reflectionState.ShaderDesc           = &m_desc.ShaderStages.GetElement( stageIndex );
        LocalRootSignatureDesc &recordLayout = m_reflectDesc.LocalRootSignatures.GetElement( stageIndex );
        reflectionState.LocalRootSignature   = &recordLayout;

        auto            reflectionBlob = shader->Reflection;
        const DxcBuffer reflectionBuffer{
            .Ptr      = reflectionBlob.Data( ),
            .Size     = reflectionBlob.NumElements( ),
            .Encoding = 0,
        };

        ID3D12ShaderReflection  *shaderReflection  = nullptr;
        ID3D12LibraryReflection *libraryReflection = nullptr;

        switch ( shader->Stage )
        {
        case ShaderStage::AnyHit:
        case ShaderStage::ClosestHit:
        case ShaderStage::Callable:
        case ShaderStage::Intersection:
        case ShaderStage::Raygen:
        case ShaderStage::Miss:
            DXC_CHECK_RESULT( m_compiler.DxcUtils( )->CreateReflection( &reflectionBuffer, IID_PPV_ARGS( &libraryReflection ) ) );
            reflectionState.LibraryReflection = libraryReflection;
            ReflectLibrary( reflectionState );
            break;
        case ShaderStage::Vertex:
        default:
            DXC_CHECK_RESULT( m_compiler.DxcUtils( )->CreateReflection( &reflectionBuffer, IID_PPV_ARGS( &shaderReflection ) ) );
            reflectionState.ShaderReflection = shaderReflection;
            ReflectShader( reflectionState );

            // Extract thread group size for compute/mesh/task shaders
            if ( shader->Stage == ShaderStage::Compute || shader->Stage == ShaderStage::Mesh || shader->Stage == ShaderStage::Task )
            {
                ThreadGroupInfo threadGroup = ShaderReflectionHelper::ExtractThreadGroupSize( shaderReflection, nullptr );
                shader->ThreadGroup         = threadGroup;
                m_reflectDesc.ThreadGroups.SetElement( stageIndex, threadGroup );
            }
            break;
        }

        if ( shaderReflection )
        {
            shaderReflection->Release( );
        }
        if ( libraryReflection )
        {
            libraryReflection->Release( );
        }
    }

    // Process bindless arrays from all shader stages
    ProcessBindlessArrays( rootSignature );

#ifndef NDEBUG
    ReflectionDebugOutput::DumpReflectionInfo( m_reflectDesc );
#endif
}

InteropArray<CompiledShaderStage *> ShaderProgram::CompiledShaders( ) const
{
    InteropArray<CompiledShaderStage *> compiledShaders;
    for ( auto &shader : m_compiledShaders )
    {
        compiledShaders.AddElement( shader.get( ) );
    }
    return std::move( compiledShaders );
}

Format MaskToFormat( const D3D_REGISTER_COMPONENT_TYPE componentType, const uint32_t mask )
{
    switch ( componentType )
    {
    case D3D_REGISTER_COMPONENT_UINT32:
        switch ( mask )
        {
        case 1:
            return Format::R32Uint;
        case 3:
            return Format::R32G32Uint;
        case 7:
            return Format::R32G32B32Uint;
        case 15:
            return Format::R32G32B32A32Uint;
        default:
            return Format::Undefined;
        }
    case D3D_REGISTER_COMPONENT_SINT32:
        switch ( mask )
        {
        case 1:
            return Format::R32Sint;
        case 3:
            return Format::R32G32Sint;
        case 7:
            return Format::R32G32B32Sint;
        case 15:
            return Format::R32G32B32A32Sint;
        default:
            return Format::Undefined;
        }
    case D3D_REGISTER_COMPONENT_FLOAT32:
        switch ( mask )
        {
        case 1:
            return Format::R32Float;
        case 3:
            return Format::R32G32Float;
        case 7:
            return Format::R32G32B32Float;
        case 15:
            return Format::R32G32B32A32Float;
        default:
            return Format::Undefined;
        }
#ifdef D3D_READY // Removed because they used to work with Directx Headers, however the dependency is now removed depending on DXC version which is a bit behind and doesn't have
                 // these values
    case D3D_REGISTER_COMPONENT_UINT16:
        switch ( mask )
        {
        case 1:
            return Format::R16Uint;
        case 3:
            return Format::R16G16Uint;
        case 15:
            return Format::R16G16B16A16Uint;
        default:
            return Format::Undefined;
        }
    case D3D_REGISTER_COMPONENT_SINT16:
        switch ( mask )
        {
        case 1:
            return Format::R16Sint;
        case 3:
            return Format::R16G16Sint;
        case 15:
            return Format::R16G16B16A16Sint;
        default:
            return Format::Undefined;
        }
    case D3D_REGISTER_COMPONENT_FLOAT16:
        switch ( mask )
        {
        case 1:
            return Format::R16Float;
        case 3:
            return Format::R16G16Float;
        case 15:
            return Format::R16G16B16A16Float;
        default:
            return Format::Undefined;
        }
#endif
    default:
        return Format::Undefined;
    }
}

ShaderReflectDesc ShaderProgram::Reflect( ) const
{
    return m_reflectDesc;
}

ShaderProgramDesc ShaderProgram::Desc( ) const
{
    return m_desc;
}

void ShaderProgram::ReflectShader( const ReflectionState &state ) const
{
    ID3D12ShaderReflection *shaderReflection = state.ShaderReflection;

    D3D12_SHADER_DESC shaderDesc{ };
    DXC_CHECK_RESULT( shaderReflection->GetDesc( &shaderDesc ) );

    if ( state.ShaderDesc->Stage == ShaderStage::Vertex )
    {
        InitInputLayout( shaderReflection, *state.InputLayoutDesc, shaderDesc );
    }

    DXC_CHECK_RESULT( shaderReflection->GetDesc( &shaderDesc ) );
    for ( const uint32_t i : std::views::iota( 0u, shaderDesc.BoundResources ) )
    {
        D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{ };
        DXC_CHECK_RESULT( shaderReflection->GetResourceBindingDesc( i, &shaderInputBindDesc ) );
        ProcessInputBindingDesc( state, shaderInputBindDesc, i );
    }
}

void ShaderProgram::ReflectLibrary( ReflectionState &state ) const
{
    ID3D12LibraryReflection *libraryReflection = state.LibraryReflection;

    D3D12_LIBRARY_DESC libraryDesc{ };
    DXC_CHECK_RESULT( libraryReflection->GetDesc( &libraryDesc ) );

    for ( const uint32_t i : std::views::iota( 0u, libraryDesc.FunctionCount ) )
    {
        ID3D12FunctionReflection *functionReflection = libraryReflection->GetFunctionByIndex( i );
        D3D12_FUNCTION_DESC       functionDesc{ };
        DXC_CHECK_RESULT( functionReflection->GetDesc( &functionDesc ) );

        { // Only process the matching function
            // Check if the name is not mangled to start with
            std::string_view mangledName( functionDesc.Name );
            const bool       isMangled = mangledName.starts_with( "\01?" ) || mangledName.starts_with( "\x1?" );
            if ( !isMangled && functionDesc.Name != state.CompiledShader->EntryPoint.Get( ) )
            {
                continue;
            }
            if ( const size_t nameEnd = mangledName.find( '@' ); nameEnd != std::string_view::npos && isMangled )
            {
                if ( const std::string_view demangledName = mangledName.substr( 2, nameEnd - 2 ); demangledName != state.CompiledShader->EntryPoint.Get( ) )
                {
                    continue;
                }
            }
        }
        state.FunctionReflection = functionReflection;
        for ( const uint32_t j : std::views::iota( 0u, functionDesc.BoundResources ) )
        {
            D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{ };
            DXC_CHECK_RESULT( functionReflection->GetResourceBindingDesc( j, &shaderInputBindDesc ) );
            ProcessInputBindingDesc( state, shaderInputBindDesc, j );
        }
    }
}

void ShaderProgram::ProcessInputBindingDesc( const ReflectionState &state, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc, const int resourceIndex ) const
{
    if ( UpdateBoundResourceStage( state, shaderInputBindDesc ) )
    {
        return;
    }
    const bool                isLocal     = ShaderReflectionHelper::IsBindingLocalTo( state.ShaderDesc->RayTracing, shaderInputBindDesc );
    const ResourceBindingType bindingType = DxcEnumConverter::ReflectTypeToBufferBindingType( shaderInputBindDesc.Type );
    // Root constants are reserved for a specific register space
    // PS: Constant buffers in local root signatures are already handled as root constants
    if ( shaderInputBindDesc.Space == DZConfiguration::Instance( ).RootConstantRegisterSpace && !isLocal )
    {
        ReflectionDesc rootConstantReflection;
        ShaderReflectionHelper::FillReflectionData( state.ShaderReflection, state.FunctionReflection, rootConstantReflection, resourceIndex );
        if ( rootConstantReflection.Type != ReflectionBindingType::Pointer && rootConstantReflection.Type != ReflectionBindingType::Struct )
        {
            LOG( FATAL ) << "Root constant reflection type mismatch. RegisterSpace [" << shaderInputBindDesc.Space
                         << "] is reserved for root constants. Which cannot be samplers or textures.";
        }
        RootConstantResourceBindingDesc &rootConstantBinding = state.RootSignatureDesc->RootConstants.EmplaceElement( );
        rootConstantBinding.Name                             = shaderInputBindDesc.Name;
        rootConstantBinding.Binding                          = shaderInputBindDesc.BindPoint;
        rootConstantBinding.Stages.AddElement( state.ShaderDesc->Stage );
        rootConstantBinding.NumBytes   = rootConstantReflection.NumBytes;
        rootConstantBinding.Reflection = rootConstantReflection;
        return;
    }

    // If this register space is configured to be a LocalRootSignature, then populate the corresponding Bindings.
    InteropArray<ResourceBindingDesc> *resourceBindings = &state.RootSignatureDesc->ResourceBindings;
    if ( isLocal )
    {
        resourceBindings = &state.LocalRootSignature->ResourceBindings;
    }

    const bool isBindless      = ShaderReflectionHelper::IsBindingBindless( state.ShaderDesc->Bindless, shaderInputBindDesc );
    bool       isBindlessArray = false;
    for ( int i = 0; i < state.ShaderDesc->Bindless.BindlessArrays.NumElements( ); ++i )
    {
        const auto &bindlessSlot = state.ShaderDesc->Bindless.BindlessArrays.GetElement( i );
        if ( bindlessSlot.Binding == shaderInputBindDesc.BindPoint && bindlessSlot.RegisterSpace == shaderInputBindDesc.Space &&
             bindlessSlot.Type == DxcEnumConverter::ReflectTypeToBufferBindingType( shaderInputBindDesc.Type ) )
        {
            isBindlessArray = true;
            break;
        }
    }

    // Skip creating regular resource binding for bindless arrays as they're handled separately
    if ( isBindlessArray )
    {
        return;
    }

    ResourceBindingDesc &resourceBindingDesc = resourceBindings->EmplaceElement( );
    resourceBindingDesc.Name                 = shaderInputBindDesc.Name;
    resourceBindingDesc.Binding              = shaderInputBindDesc.BindPoint;
    resourceBindingDesc.RegisterSpace        = shaderInputBindDesc.Space;
    resourceBindingDesc.ArraySize            = isBindless ? UINT_MAX : shaderInputBindDesc.BindCount;
    resourceBindingDesc.BindingType          = bindingType;
    resourceBindingDesc.Descriptor           = DxcEnumConverter::ReflectTypeToRootSignatureType( shaderInputBindDesc.Type, shaderInputBindDesc.Dimension );
    resourceBindingDesc.Stages.AddElement( state.ShaderDesc->Stage );
    resourceBindingDesc.IsBindless = isBindless;
    ShaderReflectionHelper::FillReflectionData( state.ShaderReflection, state.FunctionReflection, resourceBindingDesc.Reflection, resourceIndex );
}

void ShaderProgram::ProcessBindlessArrays( RootSignatureDesc &rootSignature ) const
{
    for ( int stageIndex = 0; stageIndex < m_desc.ShaderStages.NumElements( ); ++stageIndex )
    {
        const auto &shaderStage = m_desc.ShaderStages.GetElement( stageIndex );
        for ( int i = 0; i < shaderStage.Bindless.BindlessArrays.NumElements( ); ++i )
        {
            const auto &bindlessSlot  = shaderStage.Bindless.BindlessArrays.GetElement( i );
            bool        alreadyExists = false;
            for ( int j = 0; j < rootSignature.BindlessResources.NumElements( ); ++j )
            {
                const auto &existing = rootSignature.BindlessResources.GetElement( j );
                if ( existing.Binding == bindlessSlot.Binding && existing.RegisterSpace == bindlessSlot.RegisterSpace && existing.Type == bindlessSlot.Type )
                {
                    alreadyExists = true;
                    break;
                }
            }

            if ( !alreadyExists )
            {
                BindlessResourceDesc &bindlessResource = rootSignature.BindlessResources.EmplaceElement( );
                bindlessResource.Binding               = bindlessSlot.Binding;
                bindlessResource.RegisterSpace         = bindlessSlot.RegisterSpace;
                bindlessResource.Type                  = bindlessSlot.Type;
                bindlessResource.MaxArraySize          = bindlessSlot.MaxArraySize;
                bindlessResource.IsDynamic             = true;
                bindlessResource.Name                  = InteropString( "BindlessArray_" )
                                            .Append( std::to_string( bindlessSlot.Binding ).c_str( ) )
                                            .Append( "_" )
                                            .Append( std::to_string( bindlessSlot.RegisterSpace ).c_str( ) );
            }
        }
    }
}

bool ShaderProgram::UpdateBoundResourceStage( const ReflectionState &state, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc ) const
{
    const ResourceBindingType bindingType = DxcEnumConverter::ReflectTypeToBufferBindingType( shaderInputBindDesc.Type );
    // Check if Resource is already bound, if so add the stage to the existing binding and continue
    bool found = false;

    // Check if it is a root constant:
    if ( shaderInputBindDesc.Space == DZConfiguration::Instance( ).RootConstantRegisterSpace )
    {
        for ( int bindingIndex = 0; bindingIndex < state.RootSignatureDesc->RootConstants.NumElements( ); ++bindingIndex )
        {
            if ( auto &boundBinding = state.RootSignatureDesc->RootConstants.GetElement( bindingIndex ); boundBinding.Binding == shaderInputBindDesc.BindPoint )
            {
                found = true;
                boundBinding.Stages.AddElement( state.ShaderDesc->Stage );
                break;
            }
        }
        return found;
    }

    for ( int bindingIndex = 0; bindingIndex < state.RootSignatureDesc->ResourceBindings.NumElements( ); ++bindingIndex )
    {
        auto &boundBinding  = state.RootSignatureDesc->ResourceBindings.GetElement( bindingIndex );
        bool  isSameBinding = boundBinding.RegisterSpace == shaderInputBindDesc.Space;
        isSameBinding       = isSameBinding && boundBinding.Binding == shaderInputBindDesc.BindPoint;
        isSameBinding       = isSameBinding && boundBinding.BindingType == bindingType;
        isSameBinding       = isSameBinding && strcmp( boundBinding.Name.Get( ), shaderInputBindDesc.Name ) == 0;
        if ( isSameBinding )
        {
            found            = true;
            bool stageExists = false;
            for ( int stageIndex = 0; stageIndex < boundBinding.Stages.NumElements( ); ++stageIndex )
            {
                if ( boundBinding.Stages.GetElement( stageIndex ) == state.ShaderDesc->Stage )
                {
                    stageExists = true;
                    break;
                }
            }
            if ( !stageExists )
            {
                boundBinding.Stages.AddElement( state.ShaderDesc->Stage );
            }
        }
    }
    return found;
}

void ShaderProgram::InitInputLayout( ID3D12ShaderReflection *shaderReflection, InputLayoutDesc &inputLayoutDesc, const D3D12_SHADER_DESC &shaderDesc ) const
{
    constexpr D3D_NAME providedSemantics[ 8 ] = {
        D3D_NAME_VERTEX_ID, D3D_NAME_INSTANCE_ID,   D3D_NAME_PRIMITIVE_ID, D3D_NAME_RENDER_TARGET_ARRAY_INDEX, D3D_NAME_VIEWPORT_ARRAY_INDEX,
        D3D_NAME_VERTEX_ID, D3D_NAME_CLIP_DISTANCE, D3D_NAME_CULL_DISTANCE
    };

    std::vector<InputLayoutElementDesc> inputElements;
    for ( const uint32_t parameterIndex : std::views::iota( 0u, shaderDesc.InputParameters ) )
    {
        D3D12_SIGNATURE_PARAMETER_DESC signatureParameterDesc{ };
        DXC_CHECK_RESULT( shaderReflection->GetInputParameterDesc( parameterIndex, &signatureParameterDesc ) );

        bool isSemanticProvided = false;
        for ( const uint32_t i : std::views::iota( 0u, 8u ) )
        {
            if ( signatureParameterDesc.SystemValueType == providedSemantics[ i ] )
            {
                isSemanticProvided = true;
                break;
            }
        }

        if ( isSemanticProvided )
        {
            continue;
        }

        InputLayoutElementDesc &inputElementDesc = inputElements.emplace_back( );
        inputElementDesc.Semantic                = signatureParameterDesc.SemanticName;
        inputElementDesc.SemanticIndex           = signatureParameterDesc.SemanticIndex;
        inputElementDesc.Format                  = MaskToFormat( signatureParameterDesc.ComponentType, signatureParameterDesc.Mask );
    }

    if ( !inputElements.empty( ) )
    {
        auto &inputElementsArray = inputLayoutDesc.InputGroups.EmplaceElement( );
        for ( int i = 0; i < inputElements.size( ); ++i )
        {
            inputElementsArray.Elements.AddElement( inputElements[ i ] );
        }
    }
}

ShaderProgram::~ShaderProgram( )
{
}
