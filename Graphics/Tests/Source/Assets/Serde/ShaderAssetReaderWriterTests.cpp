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

#include "gtest/gtest.h"

#include "../../../../Internal/DenOfIzGraphicsInternal/Utilities/DZArenaHelper.h"
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetReader.h"
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"

using namespace DenOfIz;

class ShaderAssetSerdeTest : public testing::Test
{
    std::unique_ptr<ShaderAsset> m_asset;

protected:
    ByteArray CreateTestShaderData( DZArena &arena, const uint32_t size, const uint32_t seed )
    {
        ByteArray data;
        DZArenaArrayHelper<ByteArray, Byte>::AllocateArray( arena, data, size );

        for ( uint32_t i = 0; i < data.NumElements; ++i )
        {
            data.Elements[ i ] = static_cast<Byte>( ( i + seed * 50 ) % 256 );
        }

        return data;
    }

    ShaderAsset *CreateSampleShaderAsset( )
    {
        m_asset      = std::make_unique<ShaderAsset>( );
        m_asset->Uri = AssetUri::Create( "shaders/TestShader.dzshader" );

        m_asset->_Arena.EnsureCapacity( 16384 );
        DZArenaArrayHelper<ShaderStageAssetArray, ShaderStageAsset>::AllocateAndConstructArray( m_asset->_Arena, m_asset->Stages, 2 );

        ShaderStageAsset &vertexStage = m_asset->Stages.Elements[ 0 ];
        vertexStage.Stage             = ShaderStage::Vertex;
        vertexStage.EntryPoint        = "VSMain";
        vertexStage.DXIL              = CreateTestShaderData( m_asset->_Arena, 1024, 1 );
        vertexStage.SPIRV             = CreateTestShaderData( m_asset->_Arena, 2048, 2 );
        vertexStage.MSL               = CreateTestShaderData( m_asset->_Arena, 1536, 3 );
        vertexStage.Reflection        = CreateTestShaderData( m_asset->_Arena, 512, 4 );

        ShaderStageAsset &pixelStage = m_asset->Stages.Elements[ 1 ];
        pixelStage.Stage             = ShaderStage::Pixel;
        pixelStage.EntryPoint        = "PSMain";
        pixelStage.DXIL              = CreateTestShaderData( m_asset->_Arena, 768, 5 );
        pixelStage.SPIRV             = CreateTestShaderData( m_asset->_Arena, 1536, 6 );
        pixelStage.MSL               = CreateTestShaderData( m_asset->_Arena, 1024, 7 );
        pixelStage.Reflection        = CreateTestShaderData( m_asset->_Arena, 384, 8 );

        RootSignatureDesc &rootSig = m_asset->ReflectDesc.RootSignature;
        DZArenaArrayHelper<ResourceBindingDescArray, ResourceBindingDesc>::AllocateAndConstructArray( m_asset->_Arena, rootSig.ResourceBindings, 1 );
        DZArenaArrayHelper<StaticSamplerDescArray, StaticSamplerDesc>::AllocateAndConstructArray( m_asset->_Arena, rootSig.StaticSamplers, 1 );

        ResourceBindingDesc &binding = rootSig.ResourceBindings.Elements[ 0 ];
        binding.Name                 = "g_texture";
        binding.BindingType          = ResourceBindingType::ShaderResource;
        binding.Binding              = 0;
        binding.RegisterSpace        = 0;
        binding.Descriptor           = ResourceDescriptor::Texture;
        binding.ArraySize            = 1;

        DZArenaArrayHelper<ShaderStageArray, ShaderStage>::AllocateAndConstructArray( m_asset->_Arena, binding.Stages, 1 );
        binding.Stages.Elements[ 0 ] = ShaderStage::Pixel;

        binding.Reflection.Name     = "Texture2D";
        binding.Reflection.Type     = ReflectionBindingType::Texture;
        binding.Reflection.NumBytes = 8;

        StaticSamplerDesc &sampler    = rootSig.StaticSamplers.Elements[ 0 ];
        sampler.Sampler.MagFilter     = Filter::Linear;
        sampler.Sampler.MinFilter     = Filter::Linear;
        sampler.Sampler.AddressModeU  = SamplerAddressMode::ClampToBorder;
        sampler.Sampler.AddressModeV  = SamplerAddressMode::ClampToBorder;
        sampler.Sampler.AddressModeW  = SamplerAddressMode::ClampToBorder;
        sampler.Sampler.MaxAnisotropy = 1;
        sampler.Sampler.CompareOp     = CompareOp::Never;
        sampler.Sampler.MipmapMode    = MipmapMode::Linear;
        sampler.Sampler.MipLodBias    = 0.0f;
        sampler.Sampler.MinLod        = 0.0f;
        sampler.Sampler.MaxLod        = 1000.0f;
        sampler.Sampler.DebugName     = "g_sampler";

        sampler.Binding.Name          = "g_sampler";
        sampler.Binding.BindingType   = ResourceBindingType::Sampler;
        sampler.Binding.Binding       = 0;
        sampler.Binding.RegisterSpace = 0;
        sampler.Binding.Descriptor    = ResourceDescriptor::Sampler;
        sampler.Binding.ArraySize     = 1;

        DZArenaArrayHelper<ShaderStageArray, ShaderStage>::AllocateAndConstructArray( m_asset->_Arena, sampler.Binding.Stages, 1 );
        sampler.Binding.Stages.Elements[ 0 ] = ShaderStage::Pixel;

        // Setup input layout
        InputLayoutDesc &inputLayout = m_asset->ReflectDesc.InputLayout;
        DZArenaArrayHelper<InputGroupDescArray, InputGroupDesc>::AllocateAndConstructArray( m_asset->_Arena, inputLayout.InputGroups, 1 );
        InputGroupDesc &inputGroup = inputLayout.InputGroups.Elements[ 0 ];
        DZArenaArrayHelper<InputLayoutElementDescArray, InputLayoutElementDesc>::AllocateAndConstructArray( m_asset->_Arena, inputGroup.Elements, 2 );

        // Position
        InputLayoutElementDesc &posElement = inputGroup.Elements.Elements[ 0 ];
        posElement.Semantic                = "POSITION";
        posElement.SemanticIndex           = 0;
        posElement.Format                  = Format::R32G32B32Float;

        // TexCoord
        InputLayoutElementDesc &texcoordElement = inputGroup.Elements.Elements[ 1 ];
        texcoordElement.Semantic                = "TEXCOORD";
        texcoordElement.SemanticIndex           = 0;
        texcoordElement.Format                  = Format::R32G32Float;

        inputGroup.StepRate = StepRate::PerVertex;

        return m_asset.get( );
    }

    static bool CompareByteArrays( const ByteArray &a, const ByteArray &b )
    {
        if ( a.NumElements != b.NumElements )
        {
            return false;
        }

        for ( size_t i = 0; i < a.NumElements; ++i )
        {
            if ( a.Elements[ i ] != b.Elements[ i ] )
            {
                return false;
            }
        }

        return true;
    }
};

TEST_F( ShaderAssetSerdeTest, WriteAndReadBack )
{
    BinaryContainer container;
    auto            sampleAsset = std::unique_ptr<ShaderAsset>( CreateSampleShaderAsset( ) );

    {
        BinaryWriter      writer( container );
        ShaderAssetWriter shaderWriter( ShaderAssetWriterDesc{ &writer } );
        shaderWriter.Write( *sampleAsset );
        shaderWriter.End( );
    }
    BinaryReader      reader( container );
    ShaderAssetReader shaderReader( ShaderAssetReaderDesc{ &reader } );
    auto              readAsset = std::unique_ptr<ShaderAsset>( shaderReader.Read( ) );

    // Verify basic properties
    ASSERT_EQ( readAsset->Magic, ShaderAsset{ }.Magic );
    ASSERT_EQ( readAsset->Version, ShaderAsset::Latest );
    ASSERT_STREQ( readAsset->Uri.ToInteropString( ).Get( ), sampleAsset->Uri.ToInteropString( ).Get( ) );

    // Verify shader stages
    ASSERT_EQ( readAsset->Stages.NumElements, sampleAsset->Stages.NumElements );

    for ( uint32_t i = 0; i < readAsset->Stages.NumElements; ++i )
    {
        const ShaderStageAsset &readStage   = readAsset->Stages.Elements[ i ];
        const ShaderStageAsset &sampleStage = sampleAsset->Stages.Elements[ i ];

        ASSERT_EQ( readStage.Stage, sampleStage.Stage );
        ASSERT_STREQ( readStage.EntryPoint.Get( ), sampleStage.EntryPoint.Get( ) );

        // Verify shader binary data
        ASSERT_TRUE( CompareByteArrays( readStage.DXIL, sampleStage.DXIL ) );
        ASSERT_TRUE( CompareByteArrays( readStage.SPIRV, sampleStage.SPIRV ) );
        ASSERT_TRUE( CompareByteArrays( readStage.MSL, sampleStage.MSL ) );
        ASSERT_TRUE( CompareByteArrays( readStage.Reflection, sampleStage.Reflection ) );
    }

    // Verify root signature
    ASSERT_EQ( readAsset->ReflectDesc.RootSignature.ResourceBindings.NumElements, sampleAsset->ReflectDesc.RootSignature.ResourceBindings.NumElements );

    for ( uint32_t i = 0; i < readAsset->ReflectDesc.RootSignature.ResourceBindings.NumElements; ++i )
    {
        const ResourceBindingDesc &readBinding   = readAsset->ReflectDesc.RootSignature.ResourceBindings.Elements[ i ];
        const ResourceBindingDesc &sampleBinding = sampleAsset->ReflectDesc.RootSignature.ResourceBindings.Elements[ i ];

        ASSERT_STREQ( readBinding.Name.Get( ), sampleBinding.Name.Get( ) );
        ASSERT_EQ( readBinding.BindingType, sampleBinding.BindingType );
        ASSERT_EQ( readBinding.Binding, sampleBinding.Binding );
        ASSERT_EQ( readBinding.RegisterSpace, sampleBinding.RegisterSpace );
        ASSERT_EQ( readBinding.Descriptor, sampleBinding.Descriptor );
        ASSERT_EQ( readBinding.ArraySize, sampleBinding.ArraySize );

        ASSERT_EQ( readBinding.Stages.NumElements, sampleBinding.Stages.NumElements );
        for ( uint32_t j = 0; j < readBinding.Stages.NumElements; ++j )
        {
            ASSERT_EQ( readBinding.Stages.Elements[ j ], sampleBinding.Stages.Elements[ j ] );
        }

        ASSERT_STREQ( readBinding.Reflection.Name.Get( ), sampleBinding.Reflection.Name.Get( ) );
        ASSERT_EQ( readBinding.Reflection.Type, sampleBinding.Reflection.Type );
        ASSERT_EQ( readBinding.Reflection.NumBytes, sampleBinding.Reflection.NumBytes );
    }

    // Verify static samplers
    ASSERT_EQ( readAsset->ReflectDesc.RootSignature.StaticSamplers.NumElements, sampleAsset->ReflectDesc.RootSignature.StaticSamplers.NumElements );

    for ( uint32_t i = 0; i < readAsset->ReflectDesc.RootSignature.StaticSamplers.NumElements; ++i )
    {
        const StaticSamplerDesc &readSampler   = readAsset->ReflectDesc.RootSignature.StaticSamplers.Elements[ i ];
        const StaticSamplerDesc &sampleSampler = sampleAsset->ReflectDesc.RootSignature.StaticSamplers.Elements[ i ];

        ASSERT_EQ( readSampler.Sampler.MagFilter, sampleSampler.Sampler.MagFilter );
        ASSERT_EQ( readSampler.Sampler.MinFilter, sampleSampler.Sampler.MinFilter );
        ASSERT_EQ( readSampler.Sampler.AddressModeU, sampleSampler.Sampler.AddressModeU );
        ASSERT_EQ( readSampler.Sampler.AddressModeV, sampleSampler.Sampler.AddressModeV );
        ASSERT_EQ( readSampler.Sampler.AddressModeW, sampleSampler.Sampler.AddressModeW );
        ASSERT_FLOAT_EQ( readSampler.Sampler.MaxAnisotropy, sampleSampler.Sampler.MaxAnisotropy );
        ASSERT_EQ( readSampler.Sampler.CompareOp, sampleSampler.Sampler.CompareOp );
        ASSERT_EQ( readSampler.Sampler.MipmapMode, sampleSampler.Sampler.MipmapMode );
        ASSERT_FLOAT_EQ( readSampler.Sampler.MipLodBias, sampleSampler.Sampler.MipLodBias );
        ASSERT_FLOAT_EQ( readSampler.Sampler.MinLod, sampleSampler.Sampler.MinLod );
        ASSERT_FLOAT_EQ( readSampler.Sampler.MaxLod, sampleSampler.Sampler.MaxLod );
        ASSERT_STREQ( readSampler.Sampler.DebugName.Get( ), sampleSampler.Sampler.DebugName.Get( ) );

        ASSERT_STREQ( readSampler.Binding.Name.Get( ), sampleSampler.Binding.Name.Get( ) );
        ASSERT_EQ( readSampler.Binding.BindingType, sampleSampler.Binding.BindingType );
        ASSERT_EQ( readSampler.Binding.Binding, sampleSampler.Binding.Binding );
        ASSERT_EQ( readSampler.Binding.RegisterSpace, sampleSampler.Binding.RegisterSpace );
        ASSERT_EQ( readSampler.Binding.Descriptor, sampleSampler.Binding.Descriptor );
        ASSERT_EQ( readSampler.Binding.ArraySize, sampleSampler.Binding.ArraySize );
    }

    // Verify input layout
    ASSERT_EQ( readAsset->ReflectDesc.InputLayout.InputGroups.NumElements, sampleAsset->ReflectDesc.InputLayout.InputGroups.NumElements );

    for ( uint32_t i = 0; i < readAsset->ReflectDesc.InputLayout.InputGroups.NumElements; ++i )
    {
        const InputGroupDesc &readGroup   = readAsset->ReflectDesc.InputLayout.InputGroups.Elements[ i ];
        const InputGroupDesc &sampleGroup = sampleAsset->ReflectDesc.InputLayout.InputGroups.Elements[ i ];

        ASSERT_EQ( readGroup.StepRate, sampleGroup.StepRate );
        ASSERT_EQ( readGroup.Elements.NumElements, sampleGroup.Elements.NumElements );

        for ( uint32_t j = 0; j < readGroup.Elements.NumElements; ++j )
        {
            const InputLayoutElementDesc &readElement   = readGroup.Elements.Elements[ j ];
            const InputLayoutElementDesc &sampleElement = sampleGroup.Elements.Elements[ j ];

            ASSERT_STREQ( readElement.Semantic.Get( ), sampleElement.Semantic.Get( ) );
            ASSERT_EQ( readElement.SemanticIndex, sampleElement.SemanticIndex );
            ASSERT_EQ( readElement.Format, sampleElement.Format );
        }
    }
}
