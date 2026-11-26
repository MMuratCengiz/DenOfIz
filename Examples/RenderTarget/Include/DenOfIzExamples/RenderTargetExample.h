/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/
#pragma once

#include "DenOfIzExamples/IExample.h"

#include "DenOfIzExamples/Assets/SphereAsset.h"
#include "DenOfIzExamples/DefaultRenderPipeline.h"
#include "DenOfIzExamples/QuadPipeline.h"

namespace DenOfIz
{
    class RenderTargetExample final : public IExample
    {
        std::unique_ptr<QuadPipeline>          m_quadPipeline;
        std::unique_ptr<DefaultRenderPipeline> m_renderPipeline;
        std::unique_ptr<SphereAsset>           m_sphere;

        std::vector<DenOfIz_Texture> m_deferredRenderTargets;
        DenOfIz_Sampler              m_defaultSampler = DENOFIZ_NULL_HANDLE;

        DenOfIz_CommandListPool m_deferredCommandListPool   = DENOFIZ_NULL_HANDLE;
        DenOfIz_CommandList     m_deferredCommandLists[ 3 ] = { };
        DenOfIz_Semaphore       m_deferredSemaphores[ 3 ]   = { };

    public:
        ~RenderTargetExample( ) override;
        void                     Init( ) override;
        void                     RenderDeferredImage( uint32_t frameIndex );
        void                     ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference ) override;
        void                     HandleEvent( DenOfIz_Event &event ) override;
        void                     Update( ) override;
        void                     Render( uint32_t frameIndex, DenOfIz_CommandList commandList ) override;
        void                     Quit( ) override;
        void                     OnResize( uint32_t width, uint32_t height ) override;
        void                     CreateDeferredRenderTargets( );
        struct ExampleWindowDesc WindowDesc( ) override
        {
            auto windowDesc  = DenOfIz::ExampleWindowDesc( );
            windowDesc.Title = "RenderTargetExample";
            return windowDesc;
        }
    };
} // namespace DenOfIz
