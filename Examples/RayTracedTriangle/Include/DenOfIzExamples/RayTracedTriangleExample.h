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
        DenOfIz_Texture m_raytracingOutput[ 3 ] = { DENOFIZ_NULL_HANDLE, DENOFIZ_NULL_HANDLE, DENOFIZ_NULL_HANDLE };

        DenOfIz_ShaderLocalData              m_hgData                  = DENOFIZ_NULL_HANDLE;
        RayGenConstantBuffer                 m_rayGenCB                = { };
        DenOfIz_Buffer                       m_rayGenCBResource        = DENOFIZ_NULL_HANDLE;
        DenOfIz_ShaderProgram                m_rayTracingProgram       = DENOFIZ_NULL_HANDLE;
        DenOfIz_Pipeline                     m_rayTracingPipeline      = DENOFIZ_NULL_HANDLE;
        DenOfIz_RootSignature                m_rayTracingRootSignature = DENOFIZ_NULL_HANDLE;
        std::vector<DenOfIz_BindGroupLayout> m_bindGroupLayouts;
        DenOfIz_LocalRootSignature           m_hgShaderLayout            = DENOFIZ_NULL_HANDLE;
        DenOfIz_BindGroup                    m_rayTracingBindGroups[ 3 ] = { DENOFIZ_NULL_HANDLE, DENOFIZ_NULL_HANDLE, DENOFIZ_NULL_HANDLE };
        DenOfIz_ShaderBindingTable           m_shaderBindingTable        = DENOFIZ_NULL_HANDLE;
        DenOfIz_BottomLevelAS                m_bottomLevelAS             = DENOFIZ_NULL_HANDLE;
        DenOfIz_TopLevelAS                   m_topLevelAS                = DENOFIZ_NULL_HANDLE;

        DenOfIz_Buffer m_vertexBuffer = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer m_indexBuffer  = DENOFIZ_NULL_HANDLE;

    public:
        ~RayTracedTriangleExample( ) override;
        void                     Init( ) override;
        void                     ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference ) override;
        void                     HandleEvent( DenOfIz_Event &event ) override;
        void                     Update( ) override;
        void                     Render( uint32_t frameIndex, DenOfIz_CommandList commandList ) override;
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
        void UpdateRayTracingBindGroup( int i );
        void OnResize( uint32_t width, uint32_t height ) override;
        void CreateRayTracingPipeline( );
        void CreateResources( );
        void CreateAccelerationStructures( );
        void CreateShaderBindingTable( );
    };
} // namespace DenOfIz
