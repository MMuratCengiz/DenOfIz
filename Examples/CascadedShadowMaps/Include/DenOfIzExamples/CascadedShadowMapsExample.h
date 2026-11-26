/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/
#pragma once

#include "DenOfIzExamples/IExample.h"
#include <DirectXMath.h>
#include <array>
#include <vector>

namespace DenOfIz
{
    class CascadedShadowMapsExample final : public IExample
    {
        static constexpr uint32_t NUM_CASCADES      = 4;
        static constexpr uint32_t SHADOW_MAP_SIZE   = 2048;
        static constexpr uint32_t NUM_OBJECTS        = 6;
        static constexpr float    CAMERA_NEAR        = 0.01f;
        static constexpr float    CAMERA_FAR         = 500.0f;
        static constexpr float    CASCADE_LAMBDA     = 0.75f;

        struct SceneVertex
        {
            float Position[ 3 ];
            float Normal[ 3 ];
            float TexCoord[ 2 ];
        };

        struct ObjectInfo
        {
            uint32_t IndexOffset;
            uint32_t IndexCount;
            uint32_t VertexOffset;
        };

        struct alignas( 16 ) ShadowCB
        {
            DirectX::XMFLOAT4X4 LightViewProj[ NUM_CASCADES ];
        };

        struct alignas( 16 ) SceneCB
        {
            DirectX::XMFLOAT4X4 ViewProj;
            DirectX::XMFLOAT4X4 LightViewProj[ NUM_CASCADES ];
            DirectX::XMFLOAT4   CascadeSplits;
            DirectX::XMFLOAT4   LightDir;
            DirectX::XMFLOAT4   CameraPos;
        };

        DenOfIz_Buffer                  m_vertexBuffer = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer                  m_indexBuffer  = DENOFIZ_NULL_HANDLE;
        std::array<ObjectInfo, NUM_OBJECTS> m_objects{ };
        uint32_t                        m_totalVertices = 0;
        uint32_t                        m_totalIndices  = 0;

        DenOfIz_Texture        m_shadowMapArray      = DENOFIZ_NULL_HANDLE;
        DenOfIz_ShaderProgram  m_shadowShaderProgram = DENOFIZ_NULL_HANDLE;
        DenOfIz_InputLayout    m_shadowInputLayout   = DENOFIZ_NULL_HANDLE;
        DenOfIz_RootSignature  m_shadowRootSignature = DENOFIZ_NULL_HANDLE;
        DenOfIz_Pipeline       m_shadowPipeline      = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer         m_shadowUniformBuffer = DENOFIZ_NULL_HANDLE;
        DenOfIz_BindGroup      m_shadowBindGroup     = DENOFIZ_NULL_HANDLE;
        std::vector<DenOfIz_BindGroupLayout> m_shadowBindGroupLayouts;
        ShadowCB              *m_shadowCBData = nullptr;

        DenOfIz_ShaderProgram  m_mainShaderProgram   = DENOFIZ_NULL_HANDLE;
        DenOfIz_InputLayout    m_mainInputLayout     = DENOFIZ_NULL_HANDLE;
        DenOfIz_RootSignature  m_mainRootSignature   = DENOFIZ_NULL_HANDLE;
        DenOfIz_Pipeline       m_mainPipeline        = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer         m_sceneUniformBuffer  = DENOFIZ_NULL_HANDLE;
        DenOfIz_Sampler        m_shadowSampler       = DENOFIZ_NULL_HANDLE;
        DenOfIz_BindGroup      m_mainBindGroup       = DENOFIZ_NULL_HANDLE;
        std::vector<DenOfIz_BindGroupLayout> m_mainBindGroupLayouts;
        SceneCB               *m_sceneCBData = nullptr;

        std::array<DirectX::XMFLOAT4X4, NUM_OBJECTS> m_models{ };

        std::array<float, NUM_CASCADES + 1> m_cascadeSplits{ };

        void CreateGeometry( );
        void CreateShadowResources( );
        void CreateMainResources( );
        void ComputeCascades( );
        void UpdateUniforms( );

    public:
        ~CascadedShadowMapsExample( ) override;
        void Init( ) override;
        void ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference ) override;
        void HandleEvent( DenOfIz_Event &event ) override;
        void Update( ) override;
        void Render( uint32_t frameIndex, DenOfIz_CommandList commandList ) override;
        void Quit( ) override;

        struct ExampleWindowDesc WindowDesc( ) override
        {
            auto windowDesc  = DenOfIz::ExampleWindowDesc( );
            windowDesc.Title = "CascadedShadowMaps";
            return windowDesc;
        }
    };
} // namespace DenOfIz
