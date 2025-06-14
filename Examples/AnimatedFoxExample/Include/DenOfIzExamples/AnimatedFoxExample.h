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

// Include first because DirectXMath breaks NULL on third party libraries
#include "DenOfIzExamples/IExample.h"
#include "DenOfIzGraphics/Animation/AnimationStateManager.h"
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphics/Assets/Import/AssimpImporter.h"
#include "DenOfIzGraphics/Assets/Serde/Animation/AnimationAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Mesh/MeshAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAsset.h"
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"

#include <DirectXMath.h>

namespace DenOfIz
{
    struct SkinnedVertex
    {
        Float_3  Position;
        Float_3  Normal;
        Float_2  TexCoord;
        Float_4  Tangent;
        UInt32_4 BlendIndices;
        Float_4  BoneWeights;
    };

    struct SkinnedModelConstantBuffer
    {
        Float_4x4 BoneTransforms[ 128 ];
    };

    struct PerFrameConstantBuffer
    {
        XMMATRIX ViewProjection;
        XMVECTOR CameraPosition;
        XMVECTOR Time;
    };

    struct MaterialConstantBuffer
    {
        XMVECTOR DiffuseColor;
        XMVECTOR AmbientColor;
        float    SpecularPower;
        float    SpecularIntensity;
        float    Padding[ 2 ];
    };

    class AnimatedFoxExample final : public IExample
    {
        std::unique_ptr<IBufferResource>  m_vertexBuffer;
        std::unique_ptr<IBufferResource>  m_indexBuffer;
        std::unique_ptr<IBufferResource>  m_boneTransformsBuffer;
        std::unique_ptr<IBufferResource>  m_perFrameBuffer;
        std::unique_ptr<IBufferResource>  m_materialBuffer;
        std::unique_ptr<ITextureResource> m_texture;
        std::unique_ptr<ISampler>         m_defaultSampler;

        std::unique_ptr<IPipeline>          m_skinnedMeshPipeline;
        std::unique_ptr<IRootSignature>     m_skinnedMeshRootSignature;
        std::unique_ptr<IResourceBindGroup> m_resourceBindGroup;

        std::unique_ptr<MeshAsset>      m_foxMesh;
        std::unique_ptr<SkeletonAsset>  m_foxSkeleton;
        std::unique_ptr<AnimationAsset> m_walkAnimation;
        std::unique_ptr<AnimationAsset> m_runAnimation;

        std::unique_ptr<BinaryReader>       m_textureAssetBinaryReader;
        std::unique_ptr<TextureAssetReader> m_textureAssetReader;

        std::vector<SkinnedVertex>             m_vertices;
        std::vector<uint32_t>                  m_indices{ };
        SkinnedModelConstantBuffer            *m_boneTransformsData = nullptr;
        PerFrameConstantBuffer                *m_perFrameData       = nullptr;
        MaterialConstantBuffer                *m_materialData       = nullptr;
        std::unique_ptr<AnimationStateManager> m_animationManager;

        bool          m_animPlaying = true;
        InteropString m_currentAnim = "Walk";
        float         m_animSpeed   = 1.0f;

    public:
        ~AnimatedFoxExample( ) override;
        void Init( ) override;
        void ModifyApiPreferences( APIPreference &apiPreference ) override;
        void Update( ) override;
        void Render( uint32_t frameIndex, ICommandList *commandList ) override;
        void HandleEvent( Event &event ) override;

    private:
        void                              LoadFoxAssets( );
        bool                              ImportFoxModel( const InteropString &gltfPath ) const;
        void                              SetupAnimation( );
        void                              CreateBuffers( );
        void                              CreateShaders( );
        void                              UpdateBoneTransforms( );
        std::unique_ptr<ITextureResource> CreateTexture( ) const;
        std::unique_ptr<ISampler>         CreateDefaultSampler( ) const;
    };
} // namespace DenOfIz
