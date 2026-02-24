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

#include "DenOfIzExamples/GltfExporter.h"

#include <spdlog/spdlog.h>

#include <assimp/Exporter.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cgltf_write.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <unordered_map>

#define GLTF_EXPORTER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::GltfExporter, handle )

namespace DenOfIz
{
    class GltfExporter
    {
    public:
        Assimp::Importer m_importer;
        const aiScene   *m_scene       = nullptr;
        unsigned int     m_importFlags = 0;

        std::string m_resultGltfPath;
        std::string m_errorMessage;

        std::string                     m_name;
        std::vector<std::string>        m_supportedExtensions;
        std::vector<DenOfIz_StringView> m_supportedExtensionPtrs;

        GltfExporter( )
        {
            m_name                = "GltfExporter";
            m_supportedExtensions = { ".fbx", ".gltf", ".glb", ".obj", ".dae", ".blend", ".3ds", ".ase", ".ifc", ".xgl",
                                      ".zgl", ".ply",  ".dxf", ".lwo", ".lws", ".lxo",   ".stl", ".x",   ".ac",  ".ms3d" };
            for ( const auto &ext : m_supportedExtensions )
            {
                m_supportedExtensionPtrs.push_back( DENOFIZ_STRING( ext.c_str( ) ) );
            }
        }

        DenOfIz_StringView GetName( ) const
        {
            return { m_name.c_str( ), static_cast<uint32_t>( m_name.length( ) ) };
        }

        DenOfIz_StringViewArray GetSupportedExtensions( )
        {
            return { m_supportedExtensionPtrs.data( ), static_cast<uint32_t>( m_supportedExtensionPtrs.size( ) ) };
        }

        bool CanProcessFileExtension( const DenOfIz_StringView &extension ) const
        {
            std::string extStr( extension.Chars, extension.NumChars );
            std::transform( extStr.begin( ), extStr.end( ), extStr.begin( ), ::tolower );
            for ( const auto &supported : m_supportedExtensions )
            {
                if ( extStr == supported )
                {
                    return true;
                }
            }
            return false;
        }

        bool ValidateFile( const DenOfIz_StringView &filePath ) const
        {
            std::string path( filePath.Chars, filePath.NumChars );
            return std::filesystem::exists( path );
        }

        bool LoadScene( const std::string &filePath, const DenOfIz_GltfExportDesc &desc )
        {
            m_importFlags = aiProcess_ImproveCacheLocality | aiProcess_SortByPType | aiProcess_ValidateDataStructure;

            if ( desc.TriangulateMeshes )
            {
                m_importFlags |= aiProcess_Triangulate;
            }
            if ( desc.JoinIdenticalVertices )
            {
                m_importFlags |= aiProcess_JoinIdenticalVertices;
            }
            if ( desc.CalculateTangentSpace )
            {
                m_importFlags |= aiProcess_CalcTangentSpace;
            }
            if ( desc.FixInfacingNormals )
            {
                m_importFlags |= aiProcess_FixInfacingNormals;
            }
            if ( desc.LimitBoneWeights )
            {
                m_importFlags |= aiProcess_LimitBoneWeights;
                m_importer.SetPropertyInteger( AI_CONFIG_PP_LBW_MAX_WEIGHTS, desc.MaxBoneWeightsPerVertex );
            }
            if ( desc.RemoveRedundantMaterials )
            {
                m_importFlags |= aiProcess_RemoveRedundantMaterials;
            }
            if ( desc.GenerateNormals )
            {
                if ( desc.SmoothNormals )
                {
                    m_importFlags |= aiProcess_GenSmoothNormals;
                    m_importer.SetPropertyFloat( AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, desc.SmoothNormalsAngle );
                }
                else
                {
                    m_importFlags |= aiProcess_GenNormals;
                }
            }
            if ( desc.PreTransformVertices )
            {
                m_importFlags |= aiProcess_PreTransformVertices;
            }
            else if ( desc.OptimizeGraph )
            {
                m_importFlags |= aiProcess_OptimizeGraph;
            }
            if ( desc.OptimizeMeshes )
            {
                m_importFlags |= aiProcess_OptimizeMeshes;
            }
            if ( desc.MergeMeshes )
            {
                m_importFlags |= aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;
            }
            if ( desc.DropNormals )
            {
                m_importFlags |= aiProcess_DropNormals;
                m_importFlags &= ~( aiProcess_GenNormals | aiProcess_GenSmoothNormals );
            }

            m_importFlags |= aiProcess_GlobalScale;
            m_importer.SetPropertyFloat( AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, desc.ScaleFactor );

            m_scene = m_importer.ReadFile( filePath, m_importFlags );

            if ( !m_scene || !m_scene->mRootNode )
            {
                m_errorMessage = "Failed to load scene: ";
                m_errorMessage += m_importer.GetErrorString( );
                spdlog::error( "{}", m_errorMessage );
                return false;
            }

            spdlog::info( "Scene loaded: {} meshes, {} materials, {} animations", m_scene->mNumMeshes, m_scene->mNumMaterials, m_scene->mNumAnimations );

            return true;
        }

        bool WriteGltf( const std::string &outputPath, const DenOfIz_GltfExportDesc &desc )
        {
            Assimp::Exporter exporter;

            const char *formatId = ( desc.OutputFormat == DENOFIZ_GLTF_EXPORT_FORMAT_GLB ) ? "glb2" : "gltf2";

            unsigned int exportFlags = aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType;

            aiReturn result = exporter.Export( m_scene, formatId, outputPath, exportFlags );

            if ( result != aiReturn_SUCCESS )
            {
                m_errorMessage = "Failed to write glTF file: ";
                m_errorMessage += exporter.GetErrorString( );
                spdlog::error( "{}", m_errorMessage );
                return false;
            }

            spdlog::info( "Wrote glTF file: {}", outputPath );
            return true;
        }

        DenOfIz_GltfExportResult Export( const DenOfIz_GltfExportDesc &desc )
        {
            DenOfIz_GltfExportResult result = { };
            result.ResultCode               = DENOFIZ_GLTF_EXPORT_SUCCESS;

            m_resultGltfPath.clear( );
            m_errorMessage.clear( );

            std::string sourcePathStr( desc.SourceFilePath.Chars, desc.SourceFilePath.NumChars );
            std::string prefixStr( desc.AssetNamePrefix.Chars, desc.AssetNamePrefix.NumChars );

            if ( !LoadScene( sourcePathStr, desc ) )
            {
                result.ResultCode   = DENOFIZ_GLTF_EXPORT_IMPORT_FAILED;
                char *errMsg        = strdup( m_errorMessage.c_str( ) );
                result.ErrorMessage = { errMsg, static_cast<uint32_t>( m_errorMessage.length( ) ) };
                return result;
            }

            std::string targetDirStr( desc.TargetDirectory.Chars, desc.TargetDirectory.NumChars );
            std::filesystem::path targetDir( targetDirStr );
            if ( !std::filesystem::exists( targetDir ) )
            {
                std::filesystem::create_directories( targetDir );
            }
            std::string resolvedTargetDir = std::filesystem::absolute( targetDir ).string( );

            std::string baseName      = prefixStr.empty( ) ? std::filesystem::path( sourcePathStr ).stem( ).string( ) : prefixStr;
            std::string gltfExtension = ( desc.OutputFormat == DENOFIZ_GLTF_EXPORT_FORMAT_GLB ) ? ".glb" : ".gltf";
            std::string gltfPath      = ( std::filesystem::path( resolvedTargetDir ) / ( baseName + gltfExtension ) ).string( );

            if ( !WriteGltf( gltfPath, desc ) )
            {
                result.ResultCode   = DENOFIZ_GLTF_EXPORT_WRITE_FAILED;
                char *errMsg        = strdup( m_errorMessage.c_str( ) );
                result.ErrorMessage = { errMsg, static_cast<uint32_t>( m_errorMessage.length( ) ) };
                return result;
            }

            m_resultGltfPath = gltfPath;

            char *gltfPathCopy  = strdup( m_resultGltfPath.c_str( ) );
            result.GltfFilePath = { gltfPathCopy, static_cast<uint32_t>( m_resultGltfPath.length( ) ) };

            return result;
        }
    };
} // namespace DenOfIz

extern "C"
{

    DenOfIz_GltfExporter DenOfIz_GltfExporter_Create( )
    {
        auto *impl = new DenOfIz::GltfExporter( );
        return DENOFIZ_TO_HANDLE( impl );
    }

    void DenOfIz_GltfExporter_Destroy( DenOfIz_GltfExporter exporter )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( exporter ) )
        {
            return;
        }
        delete GLTF_EXPORTER_IMPL( exporter );
    }

    DenOfIz_StringView DenOfIz_GltfExporter_GetName( DenOfIz_GltfExporter exporter )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( exporter ) )
        {
            return { NULL, 0 };
        }
        return GLTF_EXPORTER_IMPL( exporter )->GetName( );
    }

    DenOfIz_StringViewArray DenOfIz_GltfExporter_GetSupportedExtensions( DenOfIz_GltfExporter exporter )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( exporter ) )
        {
            return { NULL, 0 };
        }
        return GLTF_EXPORTER_IMPL( exporter )->GetSupportedExtensions( );
    }

    bool DenOfIz_GltfExporter_CanProcessFileExtension( DenOfIz_GltfExporter exporter, DenOfIz_StringView extension )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( exporter ) )
        {
            return false;
        }
        return GLTF_EXPORTER_IMPL( exporter )->CanProcessFileExtension( extension );
    }

    bool DenOfIz_GltfExporter_ValidateFile( DenOfIz_GltfExporter exporter, DenOfIz_StringView filePath )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( exporter ) )
        {
            return false;
        }
        return GLTF_EXPORTER_IMPL( exporter )->ValidateFile( filePath );
    }

    DenOfIz_GltfExportResult DenOfIz_GltfExporter_Export( DenOfIz_GltfExporter exporter, const DenOfIz_GltfExportDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( exporter ) || desc == NULL )
        {
            DenOfIz_GltfExportResult result = { };
            result.ResultCode               = DENOFIZ_GLTF_EXPORT_INVALID_PARAMETERS;
            return result;
        }
        return GLTF_EXPORTER_IMPL( exporter )->Export( *desc );
    }

    void DenOfIz_GltfExportResult_Destroy( DenOfIz_GltfExportResult *result )
    {
        if ( result == NULL )
        {
            return;
        }
        if ( result->ErrorMessage.Chars != NULL )
        {
            free( const_cast<char *>( result->ErrorMessage.Chars ) );
            result->ErrorMessage.Chars    = NULL;
            result->ErrorMessage.NumChars = 0;
        }
        if ( result->GltfFilePath.Chars != NULL )
        {
            free( const_cast<char *>( result->GltfFilePath.Chars ) );
            result->GltfFilePath.Chars    = NULL;
            result->GltfFilePath.NumChars = 0;
        }
    }
}
