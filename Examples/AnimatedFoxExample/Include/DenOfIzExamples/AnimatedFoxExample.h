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
#include "DenOfIzExamples/GltfLoader.h"
#include "DenOfIzExamples/GltfExporter.h"
#include "DenOfIzExamples/IExample.h"
#include "DenOfIzGraphics/Animation/OzzAnimation.h"
#include "DenOfIzGraphics/Assets/Import/OzzExporter.h"

#include <DirectXMath.h>
#include <memory>
#include <vector>

namespace DenOfIz
{
    struct SkinnedVertex
    {
        DenOfIz_Float3 Position;
        DenOfIz_Float3 Normal;
        DenOfIz_Float2 TexCoord;
        DenOfIz_Float4 Tangent;
        DenOfIz_UInt4  BlendIndices;
        DenOfIz_Float4 BoneWeights;
    };

    struct SkinnedModelConstantBuffer
    {
        DenOfIz_Float4x4 BoneTransforms[ 128 ];
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
        DenOfIz_Buffer  m_vertexBuffer         = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer  m_indexBuffer          = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer  m_boneTransformsBuffer = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer  m_perFrameBuffer       = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer  m_materialBuffer       = DENOFIZ_NULL_HANDLE;
        DenOfIz_Texture m_texture              = DENOFIZ_NULL_HANDLE;
        DenOfIz_Sampler m_defaultSampler       = DENOFIZ_NULL_HANDLE;

        std::vector<DenOfIz_Texture> m_depthTextures;

        DenOfIz_Pipeline                     m_skinnedMeshPipeline      = DENOFIZ_NULL_HANDLE;
        DenOfIz_RootSignature                m_skinnedMeshRootSignature = DENOFIZ_NULL_HANDLE;
        std::vector<DenOfIz_BindGroupLayout> m_bindGroupLayouts;
        DenOfIz_BindGroup                    m_bindGroup     = DENOFIZ_NULL_HANDLE;
        DenOfIz_InputLayout                  m_inputLayout   = DENOFIZ_NULL_HANDLE;
        DenOfIz_ShaderProgram                m_shaderProgram = DENOFIZ_NULL_HANDLE;

        std::unique_ptr<GltfLoader> m_gltfLoader;

        DenOfIz_OzzAnimation m_ozzAnimation  = DENOFIZ_NULL_HANDLE;
        DenOfIz_OzzContext   m_walkContext   = DENOFIZ_NULL_HANDLE;
        DenOfIz_OzzContext   m_runContext    = DENOFIZ_NULL_HANDLE;
        DenOfIz_OzzContext   m_activeContext = DENOFIZ_NULL_HANDLE;

        std::vector<DenOfIz_Float4x4> m_inverseBindMatrices;
        std::vector<SkinnedVertex>    m_vertices;
        std::vector<uint32_t>         m_indices{ };
        SkinnedModelConstantBuffer   *m_boneTransformsData = nullptr;
        PerFrameConstantBuffer       *m_perFrameData       = nullptr;
        MaterialConstantBuffer       *m_materialData       = nullptr;

        DenOfIz_Float4x4Array m_modelTransforms{ };

        bool        m_animPlaying    = true;
        std::string m_currentAnim    = "Walk";
        float       m_animSpeed      = 1.0f;
        float       m_animationRatio = 0.0f;

    public:
        ~AnimatedFoxExample( ) override;
        void Init( ) override;
        void ModifyApiPreferences( DenOfIz_APIPreference &apiPreference ) override;
        void Update( ) override;
        void Render( uint32_t frameIndex, DenOfIz_CommandList commandList ) override;
        void HandleEvent( DenOfIz_Event &event ) override;
        void OnResize( uint32_t width, uint32_t height ) override;

    private:
        void LoadFoxAssets( );
        bool ExportFoxModel( const DenOfIz_StringView &gltfPath );
        void LoadMeshFromGltf( const char *gltfPath );
        void SetupAnimation( );
        void CreateBuffers( );
        void CreateShaders( );
        void UpdateBoneTransforms( );
        void CreateTexture( );
        void CreateDefaultSampler( );
    };
} // namespace DenOfIz
