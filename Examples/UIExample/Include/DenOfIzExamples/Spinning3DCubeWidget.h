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

#include <DenOfIzGraphics/Backends/Common/ShaderProgram.h>
#include <DenOfIzGraphics/Backends/Interface/BindGroup.h>
#include <DenOfIzGraphics/Backends/Interface/Buffer.h>
#include <DenOfIzGraphics/Backends/Interface/CommandList.h>
#include <DenOfIzGraphics/Backends/Interface/CommandListPool.h>
#include <DenOfIzGraphics/Backends/Interface/CommandQueue.h>
#include <DenOfIzGraphics/Backends/Interface/InputLayout.h>
#include <DenOfIzGraphics/Backends/Interface/LogicalDevice.h>
#include <DenOfIzGraphics/Backends/Interface/Pipeline.h>
#include <DenOfIzGraphics/Backends/Interface/RootSignature.h>
#include <DenOfIzGraphics/Backends/Interface/Semaphore.h>
#include <DenOfIzGraphics/Backends/Interface/Texture.h>
#include <DenOfIzGraphics/Support/ResourceTracking.h>
#include <DirectXMath.h>
#include <array>
#include <vector>

namespace DenOfIz
{
    struct CubeVertex
    {
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT4 Color;
    };

    struct alignas( 16 ) CubeUniforms
    {
        DirectX::XMFLOAT4X4 MVP;
        DirectX::XMFLOAT4   LightDirection;
    };

    class Spinning3DCubeWidget
    {
        DenOfIz_ShaderProgram                m_shaderProgram = DENOFIZ_NULL_HANDLE;
        DenOfIz_Pipeline                     m_pipeline      = DENOFIZ_NULL_HANDLE;
        DenOfIz_RootSignature                m_rootSignature = DENOFIZ_NULL_HANDLE;
        DenOfIz_InputLayout                  m_inputLayout   = DENOFIZ_NULL_HANDLE;
        std::vector<DenOfIz_BindGroupLayout> m_bindGroupLayouts;
        DenOfIz_BindGroup                    m_bindGroup = DENOFIZ_NULL_HANDLE;

        DenOfIz_Buffer m_vertexBuffer  = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer m_indexBuffer   = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer m_uniformBuffer = DENOFIZ_NULL_HANDLE;
        CubeUniforms  *m_uniformData   = nullptr;
        uint32_t       m_indexCount    = 0;

        float             m_rotation      = 0.0f;
        float             m_rotationSpeed = 1.0f;
        DirectX::XMFLOAT4 m_cubeColor     = DirectX::XMFLOAT4( 0.2f, 0.5f, 0.9f, 1.0f );

        DenOfIz_LogicalDevice    m_device           = DENOFIZ_NULL_HANDLE;
        DenOfIz_ResourceTracking m_resourceTracking = DENOFIZ_NULL_HANDLE;

        std::array<DenOfIz_Texture, 3> m_depthBuffers{ };
        std::array<DenOfIz_Texture, 3> m_renderTargets{ };

        DenOfIz_CommandListPool m_commandListPool = DENOFIZ_NULL_HANDLE;
        DenOfIz_Semaphore       m_renderSemaphore = DENOFIZ_NULL_HANDLE;
        DenOfIz_CommandQueue    m_commandQueue    = DENOFIZ_NULL_HANDLE;

        uint32_t m_id = 0;

        static constexpr uint32_t m_numFrames = 3;

        void CreateShaderProgram( );
        void CreatePipeline( );
        void CreateGeometry( );
        void UpdateUniforms( uint32_t width, uint32_t height ) const;

    public:
        explicit Spinning3DCubeWidget( uint32_t id );
        ~Spinning3DCubeWidget( );

        void Update( float deltaTime );

        void InitializeRenderResources( DenOfIz_LogicalDevice device, DenOfIz_CommandQueue commandQueue, uint32_t width, uint32_t height );
        void ResizeRenderResources( uint32_t width, uint32_t height );
        void RenderToTexture( uint32_t frameIndex );

        void SetRotationSpeed( float speed );
        void SetCubeColor( const DirectX::XMFLOAT4 &color );

        DenOfIz_Texture   GetRenderTarget( uint32_t frameIndex ) const;
        DenOfIz_Semaphore GetRenderSemaphore( ) const;
    };

} // namespace DenOfIz
