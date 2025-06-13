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
#include "DenOfIzGraphics/Assets/Import/ShaderImporter.h"
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphics/Assets/FileSystem/PathResolver.h"
#include "DenOfIzGraphics/Assets/Import/AssetPathUtilities.h"
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"
#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

ShaderImporter::ShaderImporter( ) : m_name( "Shader Importer" )
{
    m_supportedExtensions               = InteropStringArray::Create( 7 );
    m_supportedExtensions.Elements[ 0 ] = "hlsl";
    m_supportedExtensions.Elements[ 1 ] = "vs.hlsl";
    m_supportedExtensions.Elements[ 2 ] = "ps.hlsl";
    m_supportedExtensions.Elements[ 3 ] = "gs.hlsl";
    m_supportedExtensions.Elements[ 4 ] = "hs.hlsl";
    m_supportedExtensions.Elements[ 5 ] = "ds.hlsl";
    m_supportedExtensions.Elements[ 6 ] = "cs.hlsl";
}

ShaderImporter::~ShaderImporter( )
{
    m_supportedExtensions.Dispose( );
}

InteropString ShaderImporter::GetName( ) const
{
    return m_name;
}

InteropStringArray ShaderImporter::GetSupportedExtensions( ) const
{
    InteropStringArray copy = InteropStringArray::Create( m_supportedExtensions.NumElements );
    for ( size_t i = 0; i < m_supportedExtensions.NumElements; ++i )
    {
        copy.Elements[ i ] = m_supportedExtensions.Elements[ i ];
    }
    return copy;
}

bool ShaderImporter::CanProcessFileExtension( const InteropString &extension ) const
{
    for ( int i = 0; i < m_supportedExtensions.NumElements; ++i )
    {
        if ( strcmp( extension.Get( ), m_supportedExtensions.Elements[ i ].Get( ) ) == 0 )
        {
            return true;
        }
    }
    return false;
}

ImporterResult ShaderImporter::Import( const ShaderImportDesc &desc )
{
    ImportContext context;
    context.Result.ResultCode = ImporterResultCode::Success;
    context.Desc              = desc;
    if ( context.Desc.ProgramDesc.ShaderStages.NumElements == 0 )
    {
        spdlog::warn( "No Shader Stages provided." );
        return ImporterResult{ ImporterResultCode::InvalidParameters, "No Shader Stages provided." };
    }

    const ShaderProgramDesc shaderProgramDesc = context.Desc.ProgramDesc;
    const ShaderProgram     shaderProgram( shaderProgramDesc );

    CompiledShader compiledShader;
    compiledShader.RayTracing  = shaderProgramDesc.RayTracing;
    compiledShader.Stages      = shaderProgram.CompiledShaders( );
    compiledShader.ReflectDesc = shaderProgram.Reflect( );

    context.ShaderAsset = ShaderAssetWriter::CreateFromCompiledShader( compiledShader );

    AssetUri shaderAssetUri;
    WriteShaderAsset( context, shaderAssetUri );
    m_createdAssets.push_back( shaderAssetUri );
    
    // Copy the created assets to the result
    context.Result.CreatedAssets.NumElements = static_cast<uint32_t>(m_createdAssets.size());
    context.Result.CreatedAssets.Elements = m_createdAssets.data();
    return context.Result;
}

bool ShaderImporter::ValidateFile( const InteropString &filePath ) const
{
    if ( filePath.IsEmpty( ) )
    {
        return true;
    }

    const std::string resolvedPath = PathResolver::ResolvePath( filePath.Get( ) );
    if ( !FileIO::FileExists( filePath ) )
    {
        return false;
    }

    const InteropString extension = AssetPathUtilities::GetFileExtension( filePath );
    return CanProcessFileExtension( extension.Get( ) );
}

void ShaderImporter::WriteShaderAsset( const ImportContext &context, AssetUri &outAssetUri )
{
    InteropString       assetName           = GetAssetName( context );
    const InteropString sanitizedName       = AssetPathUtilities::SanitizeAssetName( assetName );
    const InteropString shaderAssetFileName = AssetPathUtilities::CreateAssetFileName( context.Desc.OutputShaderName, sanitizedName, ShaderAsset::Extension( ) );
    std::string         outputPath          = context.Desc.TargetDirectory.Get( );
    if ( !outputPath.empty( ) && outputPath.back( ) != '/' && outputPath.back( ) != '\\' )
    {
        outputPath += '/';
    }
    outputPath += shaderAssetFileName.Get( );

    BinaryContainer container{ };
    BinaryWriter    writer( container );

    ShaderAssetWriterDesc writerDesc{ };
    writerDesc.Writer = &writer;
    ShaderAssetWriter shaderWriter( writerDesc );

    shaderWriter.Write( *context.ShaderAsset );
    shaderWriter.End( );
    delete context.ShaderAsset;

    FileIO::WriteFile( outputPath.c_str( ), container.GetData( ) );

    outAssetUri.Path = outputPath.c_str( );
}

InteropString ShaderImporter::GetAssetName( const ImportContext &context )
{
    if ( !context.Desc.OutputShaderName.IsEmpty( ) )
    {
        return context.Desc.OutputShaderName;
    }
    if ( context.Desc.ProgramDesc.ShaderStages.NumElements > 0 )
    {
        const ShaderStageDesc &primaryStage = context.Desc.ProgramDesc.ShaderStages.Elements[ 0 ];
        if ( !primaryStage.Path.IsEmpty( ) )
        {
            return AssetPathUtilities::GetAssetNameFromFilePath( primaryStage.Path );
        }

        if ( !primaryStage.EntryPoint.IsEmpty( ) )
        {
            std::string name = "Shader_";
            name += primaryStage.EntryPoint.Get( );
            return name.c_str( );
        }
    }

    return "ShaderProgram";
}

ShaderStage ShaderImporter::InferShaderStageFromExtension( const std::string &fileExtension )
{
    if ( fileExtension == "vs.hlsl" )
    {
        return ShaderStage::Vertex;
    }
    if ( fileExtension == "ps.hlsl" )
    {
        return ShaderStage::Pixel;
    }
    if ( fileExtension == "gs.hlsl" )
    {
        return ShaderStage::Geometry;
    }
    if ( fileExtension == "hs.hlsl" )
    {
        return ShaderStage::Hull;
    }
    if ( fileExtension == "ds.hlsl" )
    {
        return ShaderStage::Domain;
    }
    if ( fileExtension == "cs.hlsl" )
    {
        return ShaderStage::Compute;
    }
    return ShaderStage::Pixel;
}
