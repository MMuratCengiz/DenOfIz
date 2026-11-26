/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <memory>
#include <string>
#include <vector>
#include "DenOfIzGraphics/Utilities/InteropMath.h"
#include "Interop.h"

namespace DenOfIz
{
    struct GltfVertex
    {
        DenOfIz_Float3 Position;
        DenOfIz_Float3 Normal;
        DenOfIz_Float2 TexCoord;
        DenOfIz_Float4 Tangent;
        DenOfIz_UInt4  BlendIndices;
        DenOfIz_Float4 BoneWeights;
    };

    struct GltfMesh
    {
        std::vector<GltfVertex> Vertices;
        std::vector<uint32_t>   Indices;
        std::string             Name;
        uint32_t                MaterialIndex = 0;
    };

    struct GltfSkin
    {
        std::vector<DenOfIz_Float4x4> InverseBindMatrices;
        std::vector<std::string>      JointNames;
        std::vector<int32_t>          JointParents;
        std::string                   Name;
    };

    struct GltfLoadResult
    {
        bool                  Success = false;
        std::string           ErrorMessage;
        std::vector<GltfMesh> Meshes;
        std::vector<GltfSkin> Skins;
    };

    class GltfLoaderImpl;

    class DZ_EXAMPLES_API GltfLoader
    {
    public:
        GltfLoader( );
        ~GltfLoader( );

        GltfLoader( const GltfLoader & )            = delete;
        GltfLoader &operator=( const GltfLoader & ) = delete;
        GltfLoader( GltfLoader && )                 = delete;
        GltfLoader &operator=( GltfLoader && )      = delete;

        GltfLoadResult Load( const std::string &filePath, bool convertToLeftHanded = true );

    private:
        std::unique_ptr<GltfLoaderImpl> m_impl;
    };
} // namespace DenOfIz
