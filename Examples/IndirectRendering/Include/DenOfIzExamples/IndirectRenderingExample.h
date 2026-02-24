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

#include <vector>
#include "DenOfIzExamples/IExample.h"

namespace DenOfIz
{
    class IndirectRenderingExample final : public IExample
    {
        static constexpr uint32_t NUM_TRIANGLES = 150;
        static constexpr uint32_t NUM_GROUPS    = 1;

        float m_elapsedTime = 0.0f;

        DenOfIz_ShaderProgram                m_graphicsProgram       = DENOFIZ_NULL_HANDLE;
        DenOfIz_ShaderProgram                m_computeProgram        = DENOFIZ_NULL_HANDLE;
        DenOfIz_Pipeline                     m_graphicsPipeline      = DENOFIZ_NULL_HANDLE;
        DenOfIz_Pipeline                     m_computePipeline       = DENOFIZ_NULL_HANDLE;
        DenOfIz_InputLayout                  m_inputLayout           = DENOFIZ_NULL_HANDLE;
        DenOfIz_RootSignature                m_graphicsRootSignature = DENOFIZ_NULL_HANDLE;
        DenOfIz_RootSignature                m_computeRootSignature  = DENOFIZ_NULL_HANDLE;
        std::vector<DenOfIz_BindGroupLayout> m_graphicsBindGroupLayouts;
        std::vector<DenOfIz_BindGroupLayout> m_computeBindGroupLayouts;
        DenOfIz_Buffer                       m_vertexBuffer         = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer                       m_indexBuffer          = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer                       m_indirectBuffer       = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer                       m_perDrawDataBuffer    = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer                       m_simulationDataBuffer = DENOFIZ_NULL_HANDLE;
        DenOfIz_BindGroup                    m_computeBindGroup     = DENOFIZ_NULL_HANDLE;

        DenOfIz_Float2 m_mousePosition      = { 0.0f, 0.0f };
        DenOfIz_Float2 m_normalizedMousePos = { 0.0f, 0.0f };

        enum class BoidGroup : uint32_t
        {
            MouseFollower = 0,
        };

        struct PerDrawData
        {
            XMFLOAT4X4     Transform;
            DenOfIz_Float4 Color;
            DenOfIz_Float2 Velocity;
            uint32_t       GroupType;
            float          Padding;
        };

        struct SimulationData
        {
            DenOfIz_Float2 MousePosition;
            float          DeltaTime;
            float          ElapsedTime;
            DenOfIz_Float2 ScreenDimensions;
            DenOfIz_Float2 Padding;
        };

        struct Vertex
        {
            DenOfIz_Float4 Position;
            DenOfIz_Float4 Color;
        };

        SimulationData *m_simulationDataMapped = nullptr;

    public:
        ~IndirectRenderingExample( ) override;
        void                     Init( ) override;
        void                     ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference ) override;
        void                     HandleEvent( DenOfIz_Event &event ) override;
        void                     Update( ) override;
        void                     Render( uint32_t frameIndex, DenOfIz_CommandList commandList ) override;
        void                     Quit( ) override;
        struct ExampleWindowDesc WindowDesc( ) override
        {
            auto windowDesc   = ExampleWindowDesc( );
            windowDesc.Title  = "IndirectRenderingExample";
            windowDesc.Width  = 1280;
            windowDesc.Height = 720;
            return windowDesc;
        }

    private:
        void                     CreateBuffers( );
        void                     CreateIndirectBuffer( );
        void                     CreateComputePipeline( );
        void                     CreateGraphicsPipeline( );
        void                     RunComputeShader( DenOfIz_CommandList commandList );
        static DenOfIz_ByteArray ComputeShader( );
        static DenOfIz_ByteArray VertexShader( );
        static DenOfIz_ByteArray PixelShader( );
    };
} // namespace DenOfIz
