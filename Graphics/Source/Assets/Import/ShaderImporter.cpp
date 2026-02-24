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
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"
#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"
#include "DenOfIzGraphicsInternal/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphicsInternal/Assets/Import/AssetPathUtilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

using namespace DenOfIz;

#define SHADER_IMPORTER_IMPL( handle ) DENOFIZ_FROM_HANDLE( ShaderImporter, handle )

namespace DenOfIz
{
    class ShaderImporter
    {
    public:
        std::string                     m_name;
        std::vector<DenOfIz_StringView> m_supportedExtensions;
        std::vector<std::string>        m_createdAssetStorage;
        std::vector<DenOfIz_StringView> m_createdAssetViews;
        std::string                     m_errorMessage;

        struct ImportContext
        {
            std::string              OutputShaderName;
            std::string              TargetDirectory;
            DenOfIz_ShaderImportDesc Desc;
            DenOfIz_ShaderAsset      ShaderAsset;
            std::string              ErrorMessage;
            DenOfIz_ImporterResult   Result;
        };

        struct ShaderStats
        {
            uint32_t StageCount;
            size_t   EstimatedArenaSize;
        };

        ShaderImporter( )
        {
            m_name = "Shader Importer";
            m_supportedExtensions.resize( 7 );
            m_supportedExtensions[ 0 ] = DENOFIZ_STRING( "hlsl" );
            m_supportedExtensions[ 1 ] = DENOFIZ_STRING( "vs.hlsl" );
            m_supportedExtensions[ 2 ] = DENOFIZ_STRING( "ps.hlsl" );
            m_supportedExtensions[ 3 ] = DENOFIZ_STRING( "gs.hlsl" );
            m_supportedExtensions[ 4 ] = DENOFIZ_STRING( "hs.hlsl" );
            m_supportedExtensions[ 5 ] = DENOFIZ_STRING( "ds.hlsl" );
            m_supportedExtensions[ 6 ] = DENOFIZ_STRING( "cs.hlsl" );
        }

        ~ShaderImporter( ) = default;

        DenOfIz_StringView GetName( ) const
        {
            return { m_name.c_str( ), static_cast<uint32_t>( m_name.size( ) ) };
        }

        DenOfIz_StringViewArray GetSupportedExtensions( )
        {
            return { m_supportedExtensions.data( ), static_cast<uint32_t>( m_supportedExtensions.size( ) ) };
        }

        bool CanProcessFileExtension( const DenOfIz_StringView &extension ) const
        {
            const std::string ext( extension.Chars, extension.NumChars );
            for ( const auto &supportedExtension : m_supportedExtensions )
            {
                if ( strcmp( ext.c_str( ), supportedExtension.Chars ) == 0 )
                {
                    return true;
                }
            }
            return false;
        }

        bool ValidateFile( const DenOfIz_StringView &filePath ) const
        {
            const std::string filePathStr( filePath.Chars, filePath.NumChars );
            if ( filePathStr.empty( ) )
            {
                return true;
            }

            const std::string resolvedPath = FileIO::GetResourcePath( DenOfIz_StringView( filePathStr.c_str( ), static_cast<uint32_t>( filePathStr.size( ) ) ) );
            if ( !FileIO::FileExists( DENOFIZ_STRING( resolvedPath.c_str( ) ) ) )
            {
                return false;
            }

            const std::string extension = AssetPathUtilities::GetFileExtension( filePathStr.c_str( ) );
            return CanProcessFileExtension( DENOFIZ_STRING( extension.c_str( ) ) );
        }

        ShaderStats CalculateShaderStats( const DenOfIz_ShaderImportDesc &desc ) const
        {
            ShaderStats stats{ };
            stats.StageCount         = desc.ProgramDesc.ShaderStages.NumElements;
            stats.EstimatedArenaSize = sizeof( DenOfIz_ShaderStageAsset ) * stats.StageCount;
            stats.EstimatedArenaSize += 8192;
            return stats;
        }

        std::string WriteShaderAsset( const ImportContext &context ) const
        {
            const std::string assetName           = GetAssetName( context );
            const std::string sanitizedName       = AssetPathUtilities::SanitizeAssetName( assetName );
            const auto        extension           = DenOfIz_ShaderAsset_Extension( );
            const std::string shaderAssetFileName = AssetPathUtilities::CreateAssetFileName( "", sanitizedName, std::string( extension.Chars, extension.NumChars ) );
            std::string       outputPath          = context.TargetDirectory.c_str( );
            if ( !outputPath.empty( ) && outputPath.back( ) != '/' && outputPath.back( ) != '\\' )
            {
                outputPath += '/';
            }
            outputPath += shaderAssetFileName;

            DenOfIz_BinaryContainer container = DenOfIz_BinaryContainer_Create( );
            DenOfIz_BinaryWriter    writer    = DenOfIz_BinaryWriter_CreateFromContainer( container );

            DenOfIz_ShaderAssetWriterDesc writerDesc{ };
            writerDesc.Writer                      = writer;
            DenOfIz_ShaderAssetWriter shaderWriter = DenOfIz_ShaderAssetWriter_Create( &writerDesc );

            DenOfIz_ShaderAssetWriter_Write( shaderWriter, context.ShaderAsset );
            DenOfIz_ShaderAssetWriter_End( shaderWriter );
            DenOfIz_ShaderAssetWriter_Destroy( shaderWriter );
            DenOfIz_ShaderAsset_Destroy( context.ShaderAsset );

            const DenOfIz_StringView    outputPathView( outputPath.c_str( ), static_cast<uint32_t>( outputPath.size( ) ) );
            const DenOfIz_ByteArrayView containerData = DenOfIz_BinaryContainer_GetData( container );
            FileIO::WriteFile( outputPathView, containerData );

            DenOfIz_BinaryWriter_Destroy( writer );
            DenOfIz_BinaryContainer_Destroy( container );

            return outputPath;
        }

        static std::string GetAssetName( const ImportContext &context )
        {
            if ( !context.OutputShaderName.empty( ) )
            {
                return context.OutputShaderName;
            }
            if ( context.Desc.ProgramDesc.ShaderStages.NumElements > 0 )
            {
                const DenOfIz_ShaderStageDesc &primaryStage = context.Desc.ProgramDesc.ShaderStages.Elements[ 0 ];
                if ( primaryStage.Path.NumChars > 0 )
                {
                    return AssetPathUtilities::GetAssetNameFromFilePath( primaryStage.Path );
                }

                if ( primaryStage.EntryPoint.NumChars > 0 )
                {
                    const std::string entryPoint( primaryStage.EntryPoint.Chars, primaryStage.EntryPoint.NumChars );
                    const std::string name = "Shader_" + entryPoint;
                    return name.c_str( );
                }
            }

            return "ShaderProgram";
        }

        DenOfIz_ImporterResult Import( const DenOfIz_ShaderImportDesc &desc )
        {
            m_createdAssetStorage.clear( );
            m_createdAssetViews.clear( );

            ImportContext context{ };
            context.TargetDirectory   = std::string( desc.TargetDirectory.Chars, desc.TargetDirectory.NumChars );
            context.OutputShaderName  = std::string( desc.OutputShaderName.Chars, desc.OutputShaderName.NumChars );
            context.Result.ResultCode = DENOFIZ_IMPORTER_RESULT_SUCCESS;
            context.Desc              = desc;

            if ( context.Desc.ProgramDesc.ShaderStages.NumElements == 0 )
            {
                spdlog::warn( "No Shader Stages provided." );
                m_errorMessage = "No Shader Stages provided.";
                return DenOfIz_ImporterResult{ DENOFIZ_IMPORTER_RESULT_INVALID_PARAMETERS, DENOFIZ_STRING( m_errorMessage.c_str( ) ) };
            }

            const DenOfIz_ShaderProgramDesc shaderProgramDesc = context.Desc.ProgramDesc;
            DenOfIz_ShaderProgram           shaderProgram     = DenOfIz_ShaderProgram_Create( &shaderProgramDesc );

            DenOfIz_CompiledShader compiledShader{ };
            compiledShader.RayTracing = shaderProgramDesc.RayTracing;
            DenOfIz_ShaderProgram_CompiledShaders( shaderProgram, &compiledShader.Stages );
            DenOfIz_ShaderProgram_Reflect( shaderProgram, &compiledShader.ReflectDesc );
            compiledShader.RayTracing = shaderProgramDesc.RayTracing;
            context.ShaderAsset       = DenOfIz_ShaderAssetWriter_CreateFromCompiledShader( &compiledShader );

            m_createdAssetStorage.push_back( WriteShaderAsset( context ) );

            DenOfIz_ShaderProgram_Destroy( shaderProgram );

            if ( !m_createdAssetStorage.empty( ) )
            {
                auto *createdAssetsArray = static_cast<DenOfIz_StringView *>( malloc( m_createdAssetStorage.size( ) * sizeof( DenOfIz_StringView ) ) );
                for ( size_t i = 0; i < m_createdAssetStorage.size( ); ++i )
                {
                    char *assetPathCopy              = strdup( m_createdAssetStorage[ i ].c_str( ) );
                    createdAssetsArray[ i ].Chars    = assetPathCopy;
                    createdAssetsArray[ i ].NumChars = static_cast<uint32_t>( m_createdAssetStorage[ i ].length( ) );
                }
                context.Result.CreatedAssets.NumElements = static_cast<uint32_t>( m_createdAssetStorage.size( ) );
                context.Result.CreatedAssets.Elements    = createdAssetsArray;
            }
            else
            {
                context.Result.CreatedAssets.NumElements = 0;
                context.Result.CreatedAssets.Elements    = nullptr;
            }

            return context.Result;
        }
    };
} // namespace DenOfIz

extern "C"
{

    DenOfIz_ShaderImporter DenOfIz_ShaderImporter_Create( )
    {
        auto *impl = new ShaderImporter( );
        return DENOFIZ_TO_HANDLE( impl );
    }

    void DenOfIz_ShaderImporter_Destroy( DenOfIz_ShaderImporter importer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( importer ) )
        {
            return;
        }
        delete SHADER_IMPORTER_IMPL( importer );
    }

    DenOfIz_StringView DenOfIz_ShaderImporter_GetName( DenOfIz_ShaderImporter importer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( importer ) )
        {
            return { NULL, 0 };
        }
        return SHADER_IMPORTER_IMPL( importer )->GetName( );
    }

    DenOfIz_StringViewArray DenOfIz_ShaderImporter_GetSupportedExtensions( DenOfIz_ShaderImporter importer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( importer ) )
        {
            return { NULL, 0 };
        }
        return SHADER_IMPORTER_IMPL( importer )->GetSupportedExtensions( );
    }

    bool DenOfIz_ShaderImporter_CanProcessFileExtension( DenOfIz_ShaderImporter importer, DenOfIz_StringView extension )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( importer ) )
        {
            return false;
        }
        return SHADER_IMPORTER_IMPL( importer )->CanProcessFileExtension( extension );
    }

    bool DenOfIz_ShaderImporter_ValidateFile( DenOfIz_ShaderImporter importer, DenOfIz_StringView filePath )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( importer ) )
        {
            return false;
        }
        return SHADER_IMPORTER_IMPL( importer )->ValidateFile( filePath );
    }

    DenOfIz_ImporterResult DenOfIz_ShaderImporter_Import( DenOfIz_ShaderImporter importer, const DenOfIz_ShaderImportDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( importer ) || desc == NULL )
        {
            DenOfIz_ImporterResult result{ };
            result.ResultCode = DENOFIZ_IMPORTER_RESULT_INVALID_PARAMETERS;
            return result;
        }
        return SHADER_IMPORTER_IMPL( importer )->Import( *desc );
    }
}
