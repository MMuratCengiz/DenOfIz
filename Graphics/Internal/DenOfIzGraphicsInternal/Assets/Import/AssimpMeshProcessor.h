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

#pragma once

#include <assimp/scene.h>
#include <set>
#include <vector>
#include "AssimpImportContext.h"
#include "DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetWriter.h"

namespace DenOfIz
{
    struct MeshProcessingStats
    {
        uint32_t ProcessedVertices = 0;
        uint32_t ProcessedIndices  = 0;
        uint32_t ProcessedMeshes   = 0;
    };

    class AssimpMeshProcessor
    {
        MeshProcessingStats         m_stats;
        std::vector<const aiMesh *> m_meshesToProcess;
        std::vector<SubMeshData>    m_subMeshData;

    public:
        AssimpMeshProcessor( );
        ~AssimpMeshProcessor( );

        ImporterResultCode         CollectMeshes( AssimpImportContext &context );
        ImporterResultCode         ProcessAllMeshes( AssimpImportContext &context, MeshAssetWriter &meshWriter );
        const MeshProcessingStats &GetStats( ) const;

    private:
        ImporterResultCode ProcessSingleMesh( AssimpImportContext &context, const aiMesh *mesh, MeshAssetWriter &assetWriter );

        void CollectMeshesFromNode( AssimpImportContext &context, const aiNode *node, std::vector<const aiMesh *> &uniqueMeshes, std::set<unsigned int> &processedIndices );
        void DetermineVertexAttributes( const aiMesh *mesh, VertexEnabledAttributes &attributes, VertexAttributeConfig &config, const AssimpImportDesc &desc ) const;
        void CalculateMeshBounds( const aiMesh *mesh, float scaleFactor, Float_3 &outMin, Float_3 &outMax ) const;

        void PrepareBoneInfluences( AssimpImportContext &context, const aiMesh *mesh, std::vector<std::vector<std::pair<int, float>>> &boneInfluences ) const;
        void ApplyBoneInfluencesToVertex( MeshVertex &vertex, const std::vector<std::pair<int, float>> &influences ) const;

        Float_4 ConvertPosition( const aiVector3D &pos, float scaleFactor ) const;
        Float_4 ConvertNormal( const aiVector3D &normal ) const;
        Float_4 ConvertTangent( const aiVector3D &tangent ) const;
        Float_2 ConvertUV( const aiVector3D &uv ) const;
        Float_4 ConvertColor( const aiColor4D &color ) const;
    };

} // namespace DenOfIz
