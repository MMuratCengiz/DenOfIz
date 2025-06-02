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
#include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include <DenOfIzGraphics/Backends/Interface/IResourceBindGroup.h>
#include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
#include <DenOfIzGraphics/Renderer/Sync/ResourceTracking.h>
#include <DenOfIzGraphics/UI/Widgets/Widget.h>
#include <DirectXMath.h>
#include <array>
#include <memory>

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

    class Spinning3DCubeWidget : public Widget
    {
        std::unique_ptr<ShaderProgram>      m_shaderProgram;
        std::unique_ptr<IPipeline>          m_pipeline;
        std::unique_ptr<IRootSignature>     m_rootSignature;
        std::unique_ptr<IInputLayout>       m_inputLayout;
        std::unique_ptr<IResourceBindGroup> m_resourceBindGroup;

        std::unique_ptr<IBufferResource> m_vertexBuffer;
        std::unique_ptr<IBufferResource> m_indexBuffer;
        std::unique_ptr<IBufferResource> m_uniformBuffer;
        CubeUniforms                    *m_uniformData = nullptr;
        uint32_t                         m_indexCount  = 0;

        float             m_rotation      = 0.0f;
        float             m_rotationSpeed = 1.0f;
        DirectX::XMFLOAT4 m_cubeColor     = DirectX::XMFLOAT4( 0.2f, 0.5f, 0.9f, 1.0f );
        ClayBoundingBox   m_bounds{ };
        ILogicalDevice   *m_device = nullptr;

        ResourceTracking                                 m_resourceTracking;
        std::array<std::unique_ptr<ITextureResource>, 3> m_depthBuffers;

        void CreateShaderProgram( );
        void CreatePipeline( );
        void CreateGeometry( );
        void UpdateUniforms( uint32_t width, uint32_t height ) const;

    public:
        Spinning3DCubeWidget( IClayContext *clayContext, uint32_t id );
        ~Spinning3DCubeWidget( ) override;

        void Update( float deltaTime ) override;
        void CreateLayoutElement( ) override;
        void Render( const ClayBoundingBox &boundingBox, IRenderBatch *renderBatch ) override;
        void HandleEvent( const Event &event ) override;

        void InitializeRenderResources( ILogicalDevice *device, uint32_t width, uint32_t height ) override;
        void ResizeRenderResources( uint32_t width, uint32_t height ) override;
        void ExecuteCustomPipeline( const WidgetExecutePipelineDesc &context ) override;

        void SetRotationSpeed( float speed );
        void SetCubeColor( const DirectX::XMFLOAT4 &color );
    };

} // namespace DenOfIz
