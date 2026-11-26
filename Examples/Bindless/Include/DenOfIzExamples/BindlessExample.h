/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/
#pragma once

#include <vector>
#include "DenOfIzExamples/IExample.h"

namespace DenOfIz
{
    class BindlessExample final : public IExample
    {
        float                                m_elapsedTime   = 0.0f;
        DenOfIz_ShaderProgram                m_program       = DENOFIZ_NULL_HANDLE;
        DenOfIz_Pipeline                     m_pipeline      = DENOFIZ_NULL_HANDLE;
        DenOfIz_InputLayout                  m_inputLayout   = DENOFIZ_NULL_HANDLE;
        DenOfIz_RootSignature                m_rootSignature = DENOFIZ_NULL_HANDLE;
        std::vector<DenOfIz_BindGroupLayout> m_bindGroupLayouts;
        DenOfIz_Buffer                       m_vertexBuffer = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer                       m_indexBuffer  = DENOFIZ_NULL_HANDLE;

        static constexpr uint32_t NUM_TEXTURES               = 4;
        DenOfIz_Texture           m_textures[ NUM_TEXTURES ] = { };
        DenOfIz_Sampler           m_sampler                  = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer            m_constantBuffer           = DENOFIZ_NULL_HANDLE;
        DenOfIz_BindGroup         m_bindGroup                = DENOFIZ_NULL_HANDLE;
        DenOfIz_BindGroup         m_perFrameBindGroup        = DENOFIZ_NULL_HANDLE;
        uint32_t                  m_currentTextureIndex      = 0;

        struct PerFrameData
        {
            uint32_t textureIndex;
            float    time;
            uint32_t padding[ 2 ];
        };

    public:
        ~BindlessExample( ) override;
        void                     Init( ) override;
        void                     ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference ) override;
        void                     HandleEvent( DenOfIz_Event &event ) override;
        void                     Update( ) override;
        void                     Render( uint32_t frameIndex, DenOfIz_CommandList commandList ) override;
        struct ExampleWindowDesc WindowDesc( ) override
        {
            auto windowDesc   = ExampleWindowDesc( );
            windowDesc.Title  = "BindlessExample";
            windowDesc.Width  = 1280;
            windowDesc.Height = 720;
            return windowDesc;
        }

    private:
        void                     CreateVertexBuffer( );
        void                     CreateTextures( );
        void                     CreateSampler( );
        void                     CreateConstantBuffer( );
        static DenOfIz_ByteArray VertexShader( );
        static DenOfIz_ByteArray PixelShader( );
    };
} // namespace DenOfIz
