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
#include <assimp/mesh.h>
#include "MeshData.h"

#include <unordered_map>

namespace DenOfIz
{
    class DZ_API MeshImporter
    {
    public:
        static constexpr uint32_t DefaultStreamSize = 1024;
        static bool               ImportMesh( const InteropString &path, MeshStreamCallback *callback, bool importTangents = true, bool optimizeMesh = true,
                                              uint32_t streamSize = DefaultStreamSize );

    private:
        static void ProcessAssimpMesh( const aiMesh *aiMesh, const aiScene *scene, MeshStreamCallback *callback, uint32_t streamSize );
        static void ProcessAnimations( const aiScene *scene, MeshStreamCallback *callback );
        static void ProcessJoints( const aiMesh *aiMesh, const aiScene *scene, MeshStreamCallback *callback );
        static void ProcessNodeHierarchy( const aiNode *node, InteropArray<JointNode> &hierarchy, const std::unordered_map<std::string, uint32_t> &boneMap, int32_t parentIndex );
        static MeshBufferSizes CalculateBufferSizes( const aiScene *scene );
    };

} // namespace DenOfIz
