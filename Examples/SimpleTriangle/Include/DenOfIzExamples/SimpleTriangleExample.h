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
    class SimpleTriangleExample final : public IExample
    {
        DenOfIz_ShaderProgram                m_shaderProgram = DENOFIZ_NULL_HANDLE;
        DenOfIz_Pipeline                     m_pipeline      = DENOFIZ_NULL_HANDLE;
        DenOfIz_InputLayout                  m_inputLayout   = DENOFIZ_NULL_HANDLE;
        DenOfIz_RootSignature                m_rootSignature = DENOFIZ_NULL_HANDLE;
        std::vector<DenOfIz_BindGroupLayout> m_bindGroupLayouts;
        DenOfIz_Buffer                       m_vertexBuffer = DENOFIZ_NULL_HANDLE;

    public:
        ~SimpleTriangleExample( ) override;
        void                     Init( ) override;
        void                     ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference ) override;
        void                     HandleEvent( DenOfIz_Event &event ) override;
        void                     Update( ) override;
        void                     Render( uint32_t frameIndex, DenOfIz_CommandList commandList ) override;
        struct ExampleWindowDesc WindowDesc( ) override
        {
            auto windowDesc   = DenOfIz::ExampleWindowDesc( );
            windowDesc.Title  = "SimpleTriangleExample";
            windowDesc.Width  = 1280;
            windowDesc.Height = 720;
            return windowDesc;
        }

    private:
        void                     CreateVertexBuffer( );
        static DenOfIz_ByteArray VertexShader( );
        static DenOfIz_ByteArray PixelShader( );
    };
} // namespace DenOfIz
