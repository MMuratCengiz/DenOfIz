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

#include <DenOfIzGraphics/Assets/Serde/Shader/ShaderAsset.h>
#include <DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetReader.h>
#include <DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetWriter.h>

using namespace DenOfIz;

class ShaderAssetSerdeTest : public testing::Test
{
protected:
    static InteropArray<Byte> CreateTestShaderData( const uint32_t size, const uint32_t seed )
    {
        InteropArray<Byte> data( size );

        for ( uint32_t i = 0; i < data.NumElements( ); ++i )
        {
            data.SetElement( i, static_cast<Byte>( ( i + seed * 50 ) % 256 ) );
        }

        return data;
    }

    static ShaderAsset CreateSampleShaderAsset( )
    {
        ShaderAsset asset;
        asset.Uri = AssetUri::Create( "shaders/TestShader.dzshader" );

        ShaderStageAsset vertexStage;
        vertexStage.Stage      = ShaderStage::Vertex;
        vertexStage.EntryPoint = "VSMain";
        vertexStage.DXIL       = CreateTestShaderData( 1024, 1 );
        vertexStage.SPIRV      = CreateTestShaderData( 2048, 2 );
        vertexStage.MSL        = CreateTestShaderData( 1536, 3 );
        vertexStage.Reflection = CreateTestShaderData( 512, 4 );

        ShaderStageAsset pixelStage;
        pixelStage.Stage      = ShaderStage::Pixel;
        pixelStage.EntryPoint = "PSMain";
        pixelStage.DXIL       = CreateTestShaderData( 768, 5 );
        pixelStage.SPIRV      = CreateTestShaderData( 1536, 6 );
        pixelStage.MSL        = CreateTestShaderData( 1024, 7 );
        pixelStage.Reflection = CreateTestShaderData( 384, 8 );

        asset.Stages.Resize( 2 );
        asset.Stages.SetElement( 0, vertexStage );
        asset.Stages.SetElement( 1, pixelStage );

        RootSignatureDesc rootSig;

        ResourceBindingDesc binding;
        binding.Name          = "g_texture";
        binding.BindingType   = ResourceBindingType::ShaderResource;
        binding.Binding       = 0;
        binding.RegisterSpace = 0;
        binding.Descriptor    = static_cast<ResourceDescriptor>( 1 );
        binding.ArraySize     = 1;

        binding.Stages.Resize( 1 );
        binding.Stages.SetElement( 0, ShaderStage::Pixel );

        binding.Reflection.Name     = "Texture2D";
        binding.Reflection.Type     = ReflectionBindingType::Texture;
        binding.Reflection.NumBytes = 8;

        rootSig.ResourceBindings.Resize( 1 );
        rootSig.ResourceBindings.SetElement( 0, binding );

        StaticSamplerDesc sampler;
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
        sampler.Binding.Descriptor    = static_cast<ResourceDescriptor>( 2 );
        sampler.Binding.ArraySize     = 1;

        sampler.Binding.Stages.Resize( 1 );
        sampler.Binding.Stages.SetElement( 0, ShaderStage::Pixel );

        rootSig.StaticSamplers.Resize( 1 );
        rootSig.StaticSamplers.SetElement( 0, sampler );

        // Setup input layout
        InputLayoutDesc inputLayout;
        InputGroupDesc  inputGroup;

        // Position
        InputLayoutElementDesc posElement;
        posElement.Semantic      = "POSITION";
        posElement.SemanticIndex = 0;
        posElement.Format        = Format::R32G32B32Float;

        // TexCoord
        InputLayoutElementDesc texcoordElement;
        texcoordElement.Semantic      = "TEXCOORD";
        texcoordElement.SemanticIndex = 0;
        texcoordElement.Format        = Format::R32G32Float;

        inputGroup.Elements.Resize( 2 );
        inputGroup.Elements.SetElement( 0, posElement );
        inputGroup.Elements.SetElement( 1, texcoordElement );
        inputGroup.StepRate = StepRate::PerVertex;

        inputLayout.InputGroups.Resize( 1 );
        inputLayout.InputGroups.SetElement( 0, inputGroup );

        // Assign reflect descriptors
        asset.ReflectDesc.RootSignature = rootSig;
        asset.ReflectDesc.InputLayout   = inputLayout;

        return asset;
    }

    static bool CompareInteropArrays( const InteropArray<Byte> &a, const InteropArray<Byte> &b )
    {
        if ( a.NumElements( ) != b.NumElements( ) )
        {
            return false;
        }

        for ( size_t i = 0; i < a.NumElements( ); ++i )
        {
            if ( a.GetElement( i ) != b.GetElement( i ) )
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
    BinaryWriter    writer( container );

    ShaderAsset sampleAsset = CreateSampleShaderAsset( );

    ShaderAssetWriter shaderWriter( ShaderAssetWriterDesc{ &writer } );
    shaderWriter.Write( sampleAsset );
    shaderWriter.End( );

    BinaryReader      reader( container );
    ShaderAssetReader shaderReader( ShaderAssetReaderDesc{ &reader } );

    ShaderAsset readAsset = shaderReader.Read( );

    // Verify basic properties
    ASSERT_EQ( readAsset.Magic, ShaderAsset{ }.Magic );
    ASSERT_EQ( readAsset.Version, ShaderAsset::Latest );
    ASSERT_STREQ( readAsset.Uri.ToInteropString( ).Get( ), sampleAsset.Uri.ToInteropString( ).Get( ) );

    // Verify shader stages
    ASSERT_EQ( readAsset.Stages.NumElements( ), sampleAsset.Stages.NumElements( ) );

    for ( uint32_t i = 0; i < readAsset.Stages.NumElements( ); ++i )
    {
        const ShaderStageAsset &readStage   = readAsset.Stages.GetElement( i );
        const ShaderStageAsset &sampleStage = sampleAsset.Stages.GetElement( i );

        ASSERT_EQ( readStage.Stage, sampleStage.Stage );
        ASSERT_STREQ( readStage.EntryPoint.Get( ), sampleStage.EntryPoint.Get( ) );

        // Verify shader binary data
        ASSERT_TRUE( CompareInteropArrays( readStage.DXIL, sampleStage.DXIL ) );
        ASSERT_TRUE( CompareInteropArrays( readStage.SPIRV, sampleStage.SPIRV ) );
        ASSERT_TRUE( CompareInteropArrays( readStage.MSL, sampleStage.MSL ) );
        ASSERT_TRUE( CompareInteropArrays( readStage.Reflection, sampleStage.Reflection ) );
    }

    // Verify root signature
    ASSERT_EQ( readAsset.ReflectDesc.RootSignature.ResourceBindings.NumElements( ), sampleAsset.ReflectDesc.RootSignature.ResourceBindings.NumElements( ) );

    for ( uint32_t i = 0; i < readAsset.ReflectDesc.RootSignature.ResourceBindings.NumElements( ); ++i )
    {
        const ResourceBindingDesc &readBinding   = readAsset.ReflectDesc.RootSignature.ResourceBindings.GetElement( i );
        const ResourceBindingDesc &sampleBinding = sampleAsset.ReflectDesc.RootSignature.ResourceBindings.GetElement( i );

        ASSERT_STREQ( readBinding.Name.Get( ), sampleBinding.Name.Get( ) );
        ASSERT_EQ( readBinding.BindingType, sampleBinding.BindingType );
        ASSERT_EQ( readBinding.Binding, sampleBinding.Binding );
        ASSERT_EQ( readBinding.RegisterSpace, sampleBinding.RegisterSpace );
        ASSERT_EQ( readBinding.Descriptor.Value( ), sampleBinding.Descriptor.Value( ) );
        ASSERT_EQ( readBinding.ArraySize, sampleBinding.ArraySize );

        ASSERT_EQ( readBinding.Stages.NumElements( ), sampleBinding.Stages.NumElements( ) );
        for ( uint32_t j = 0; j < readBinding.Stages.NumElements( ); ++j )
        {
            ASSERT_EQ( readBinding.Stages.GetElement( j ), sampleBinding.Stages.GetElement( j ) );
        }

        ASSERT_STREQ( readBinding.Reflection.Name.Get( ), sampleBinding.Reflection.Name.Get( ) );
        ASSERT_EQ( readBinding.Reflection.Type, sampleBinding.Reflection.Type );
        ASSERT_EQ( readBinding.Reflection.NumBytes, sampleBinding.Reflection.NumBytes );
    }

    // Verify static samplers
    ASSERT_EQ( readAsset.ReflectDesc.RootSignature.StaticSamplers.NumElements( ), sampleAsset.ReflectDesc.RootSignature.StaticSamplers.NumElements( ) );

    for ( uint32_t i = 0; i < readAsset.ReflectDesc.RootSignature.StaticSamplers.NumElements( ); ++i )
    {
        const StaticSamplerDesc &readSampler   = readAsset.ReflectDesc.RootSignature.StaticSamplers.GetElement( i );
        const StaticSamplerDesc &sampleSampler = sampleAsset.ReflectDesc.RootSignature.StaticSamplers.GetElement( i );

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
        ASSERT_EQ( readSampler.Binding.Descriptor.Value( ), sampleSampler.Binding.Descriptor.Value( ) );
        ASSERT_EQ( readSampler.Binding.ArraySize, sampleSampler.Binding.ArraySize );
    }

    // Verify input layout
    ASSERT_EQ( readAsset.ReflectDesc.InputLayout.InputGroups.NumElements( ), sampleAsset.ReflectDesc.InputLayout.InputGroups.NumElements( ) );

    for ( uint32_t i = 0; i < readAsset.ReflectDesc.InputLayout.InputGroups.NumElements( ); ++i )
    {
        const InputGroupDesc &readGroup   = readAsset.ReflectDesc.InputLayout.InputGroups.GetElement( i );
        const InputGroupDesc &sampleGroup = sampleAsset.ReflectDesc.InputLayout.InputGroups.GetElement( i );

        ASSERT_EQ( readGroup.StepRate, sampleGroup.StepRate );
        ASSERT_EQ( readGroup.Elements.NumElements( ), sampleGroup.Elements.NumElements( ) );

        for ( uint32_t j = 0; j < readGroup.Elements.NumElements( ); ++j )
        {
            const InputLayoutElementDesc &readElement   = readGroup.Elements.GetElement( j );
            const InputLayoutElementDesc &sampleElement = sampleGroup.Elements.GetElement( j );

            ASSERT_STREQ( readElement.Semantic.Get( ), sampleElement.Semantic.Get( ) );
            ASSERT_EQ( readElement.SemanticIndex, sampleElement.SemanticIndex );
            ASSERT_EQ( readElement.Format, sampleElement.Format );
        }
    }
}
