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

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <memory>
#include <unordered_map>
#include "DenOfIzGraphics/Assets/Import/AssimpImporter.h"
#include "DenOfIzGraphics/Assets/Import/ImporterCommon.h"

namespace DenOfIz
{
    // Pre-counts all resources in the scene for proper arena allocation
    struct AssimpSceneStats
    {
        uint32_t TotalVertices       = 0;
        uint32_t TotalIndices        = 0;
        uint32_t TotalMeshes         = 0;
        uint32_t TotalUniqueMeshes   = 0;
        uint32_t TotalMaterials      = 0;
        uint32_t TotalTextures       = 0;
        uint32_t TotalAnimations     = 0;
        uint32_t TotalBones          = 0;
        uint32_t TotalJoints         = 0;
        uint32_t TotalAnimationKeys  = 0;
        uint32_t MaxChildrenPerJoint = 0;
        uint32_t MaxUVChannels       = 0;
        uint32_t MaxColorChannels    = 0;

        size_t EstimatedArenaSize     = 0;
        size_t EstimatedAssetsCreated = 0;
    };

    class AssimpSceneLoader
    {
        std::unique_ptr<Assimp::Importer> m_importer;
        const aiScene                    *m_scene = nullptr;
        AssimpSceneStats                  m_stats;
        unsigned int                      m_importFlags = 0;
        AssimpImportDesc                  m_desc;

        std::unordered_map<std::string, bool>        m_uniqueBoneNames;
        std::unordered_map<const aiNode *, uint32_t> m_nodeChildrenCount;

    public:
        AssimpSceneLoader( );
        ~AssimpSceneLoader( );

        bool                    LoadScene( const InteropString &filePath, const AssimpImportDesc &desc );
        const aiScene          *GetScene( ) const;
        const AssimpSceneStats &GetStats( ) const;
        unsigned int            GetImportFlags( ) const;

    private:
        void ConfigureImportFlags( const AssimpImportDesc &desc );
        void GatherSceneStatistics( );
        void CountNodeHierarchy( const aiNode *node );
        void CountBoneHierarchy( const aiNode *node );
    };

} // namespace DenOfIz
