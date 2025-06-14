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

#include "DenOfIzExamples/IExample.h"
#include "DenOfIzExamples/QuadPipeline.h"

namespace DenOfIz
{
    struct NormalizedViewport
    {
        float Left;
        float Top;
        float Right;
        float Bottom;
    };

    struct RayGenConstantBuffer
    {
        NormalizedViewport Viewport;
        NormalizedViewport Stencil;
    };

    class RayTracedTriangleExample final : public IExample
    {
        std::array<std::unique_ptr<ITextureResource>, 3> m_raytracingOutput;
        std::unique_ptr<IResourceBindGroup>              m_rootConstantBindGroup;
        // Raytracing:
        std::unique_ptr<IShaderLocalData>                  m_hgData;
        RayGenConstantBuffer                               m_rayGenCB = { };
        std::unique_ptr<IBufferResource>                   m_rayGenCBResource;
        std::unique_ptr<ShaderProgram>                     m_rayTracingProgram;
        std::unique_ptr<IPipeline>                         m_rayTracingPipeline;
        std::unique_ptr<IRootSignature>                    m_rayTracingRootSignature;
        std::unique_ptr<ILocalRootSignature>               m_hgShaderLayout;
        std::array<std::unique_ptr<IResourceBindGroup>, 3> m_rayTracingBindGroups;
        std::unique_ptr<IShaderBindingTable>               m_shaderBindingTable;
        std::unique_ptr<IBottomLevelAS>                    m_bottomLevelAS;
        std::unique_ptr<ITopLevelAS>                       m_topLevelAS;

        // RayTraced Triangle:
        std::unique_ptr<IBufferResource> m_vertexBuffer;
        std::unique_ptr<IBufferResource> m_indexBuffer;
        //

    public:
        ~RayTracedTriangleExample( ) override = default;
        void              Init( ) override;
        void              ModifyApiPreferences( APIPreference &defaultApiPreference ) override;
        void              HandleEvent( Event &event ) override;
        void              Update( ) override;
        void              Render( uint32_t frameIndex, ICommandList *commandList ) override;
        void              Quit( ) override;
        struct ExampleWindowDesc WindowDesc( ) override
        {
            auto windowDesc   = DenOfIz::ExampleWindowDesc( );
            windowDesc.Title  = "RayTracedTriangleExample";
            windowDesc.Width  = 1280;
            windowDesc.Height = 720;
            return windowDesc;
        }

    private:
        void CreateRenderTargets( );
        void CreateRayTracingPipeline( );
        void CreateResources( );
        void CreateAccelerationStructures( );
        void CreateShaderBindingTable( );
    };
} // namespace DenOfIz
