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

#include <DirectXMath.h>
#include "DenOfIzExamples/ColoredSphereAsset.h"
#include "DenOfIzExamples/IExample.h"
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
        DenOfIz_Texture                   m_depthBuffer         = DENOFIZ_NULL_HANDLE;

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
