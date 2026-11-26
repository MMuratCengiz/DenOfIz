/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/
#pragma once

#include "DenOfIzExamples/IExample.h"

#include <DirectXMath.h>
#include "DenOfIzExamples/ColoredSphereAsset.h"
#include "DenOfIzExamples/QuadPipeline.h"

#include "ColoredSpherePipeline.h"

namespace DenOfIz
{

    class TransparencyExample final : public IExample
    {
        std::vector<ColoredSphereAsset *> m_spheres;
        std::vector<XMFLOAT4X4>           m_sphereTransforms;
        ColoredSpherePipeline            *m_opaquePipeline      = nullptr;
        ColoredSpherePipeline            *m_transparentPipeline = nullptr;

        float m_alphaValue     = 0.0f;
        int   m_alphaDirection = 0;

    public:
        ~TransparencyExample( ) override;
        void                     Init( ) override;
        void                     ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference ) override;
        void                     HandleEvent( DenOfIz_Event &event ) override;
        void                     Update( ) override;
        void                     Render( uint32_t frameIndex, DenOfIz_CommandList commandList ) override;
        void                     Quit( ) override;
        struct ExampleWindowDesc WindowDesc( ) override
        {
            auto windowDesc  = DenOfIz::ExampleWindowDesc( );
            windowDesc.Title = "Custom Binding Transparency Example";
            return windowDesc;
        }
    };
} // namespace DenOfIz
