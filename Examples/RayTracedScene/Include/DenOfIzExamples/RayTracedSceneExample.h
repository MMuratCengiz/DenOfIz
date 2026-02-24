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

#include <array>
#include <unordered_map>
#include <vector>

#include "Bistro.h"
#include "DenOfIzExamples/IExample.h"
#include "DenOfIzExamples/QuadPipeline.h"
#include "DenOfIzGraphics/Support/RingBuffer.h"

namespace DenOfIz
{
    struct SceneConstantBuffer
    {
        DenOfIz_Float4x4 ProjectionToWorld;
        DenOfIz_Float4   CameraPosition;
        DenOfIz_Float4   SunDirection;
        DenOfIz_Float4   SunColor;
        DenOfIz_Float4   AmbientColor;
        float            Reflectance;
        float            ElapsedTime;
        uint32_t         UseShadowRays;
        uint32_t         FrameCount;
    };

    class RayTracedSceneExample final : public IExample
    {
        std::array<DenOfIz_Texture, 3>         m_raytracingOutput{ DENOFIZ_NULL_HANDLE, DENOFIZ_NULL_HANDLE, DENOFIZ_NULL_HANDLE };
        std::array<DenOfIz_Texture, 3>         m_accumulationBuffer{ DENOFIZ_NULL_HANDLE, DENOFIZ_NULL_HANDLE, DENOFIZ_NULL_HANDLE };
        DenOfIz_RingBuffer                     m_sceneCBRingBuffer       = DENOFIZ_NULL_HANDLE;
        DenOfIz_ShaderProgram                  m_rayTracingProgram       = DENOFIZ_NULL_HANDLE;
        DenOfIz_Pipeline                       m_rayTracingPipeline      = DENOFIZ_NULL_HANDLE;
        DenOfIz_RootSignature                  m_rayTracingRootSignature = DENOFIZ_NULL_HANDLE;
        std::vector<DenOfIz_BindGroupLayout>   m_bindGroupLayouts;
        std::unordered_map<uint32_t, uint32_t> m_spaceToLayoutIndex;
        DenOfIz_Sampler                        m_sampler            = DENOFIZ_NULL_HANDLE;
        DenOfIz_ShaderBindingTable             m_shaderBindingTable = DENOFIZ_NULL_HANDLE;
        DenOfIz_TopLevelAS                     m_topLevelAS         = DENOFIZ_NULL_HANDLE;
        std::array<DenOfIz_BindGroup, 3>       m_rayTracingBindGroups{ DENOFIZ_NULL_HANDLE, DENOFIZ_NULL_HANDLE, DENOFIZ_NULL_HANDLE };
        std::array<DenOfIz_BindGroup, 3>       m_bindlessBindGroups{ DENOFIZ_NULL_HANDLE, DENOFIZ_NULL_HANDLE, DENOFIZ_NULL_HANDLE };

        std::unique_ptr<Bistro>            m_bistro;
        std::vector<DenOfIz_BottomLevelAS> m_bottomLevelASInstances;

        uint32_t         m_frameCount               = 0;
        DenOfIz_Float4x4 m_lastViewProjectionMatrix = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        bool             m_cameraMovedThisFrame     = false;

    public:
        ~RayTracedSceneExample( ) override;
        void                     Init( ) override;
        void                     ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference ) override;
        void                     HandleEvent( DenOfIz_Event &event ) override;
        void                     Update( ) override;
        void                     Render( uint32_t frameIndex, DenOfIz_CommandList commandList ) override;
        void                     Quit( ) override;
        struct ExampleWindowDesc WindowDesc( ) override
        {
            auto windowDesc  = DenOfIz::ExampleWindowDesc( );
            windowDesc.Title = "RayTracedSceneExample";
            return windowDesc;
        }

    private:
        void CreateRenderTargets( );
        void CreateSceneConstantBuffer( );
        void CreateRayTracingPipeline( );
        void CreateAccelerationStructures( );
        void CreateShaderBindingTable( );
        void UpdateCamera( uint32_t frameIndex ) const;
    };
} // namespace DenOfIz
