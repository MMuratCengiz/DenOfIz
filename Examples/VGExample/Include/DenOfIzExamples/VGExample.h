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

#include <DenOfIzExamples/IExample.h>

#include "DenOfIzGraphics/Utilities/FrameDebugRenderer.h"
#include "DenOfIzGraphics/Utilities/Time.h"
#include "DenOfIzGraphics/Vector2d/VectorGraphics.h"
#include "DenOfIzGraphics/Vector2d/VGPipeline.h"
#include "DenOfIzGraphics/Vector2d/VGTransform.h"

#include <DirectXMath.h>
#include <memory>

namespace DenOfIz
{
    class VGExample final : public IExample
    {
        std::unique_ptr<FrameDebugRenderer> m_debugRenderer;
        Time                                m_time;

        // Vector Graphics components
        std::unique_ptr<VectorGraphics>     m_vectorGraphics;
        std::unique_ptr<VGPipeline>         m_vgPipeline;
        std::unique_ptr<VGTransform>        m_vgTransform;

        float      m_animationTime;
        XMFLOAT4X4 m_projectionMatrix;

        // Animation states for different demos
        float      m_rotationAngle;
        float      m_scaleAnimTime;
        float      m_colorAnimTime;

    public:
        VGExample( )           = default;
        ~VGExample( ) override = default;

        void              Init( ) override;
        void              ModifyApiPreferences( APIPreference &defaultApiPreference ) override;
        void              HandleEvent( Event &event ) override;
        void              Update( ) override;
        void              Render( uint32_t frameIndex, ICommandList *commandList ) override;
        ExampleWindowDesc WindowDesc( ) override
        {
            auto windowDesc   = ExampleWindowDesc( );
            windowDesc.Title  = "Vector Graphics Example - 2D Drawing Framework";
            windowDesc.Width  = 1280;
            windowDesc.Height = 720;
            return windowDesc;
        }

    private:
        void UpdateProjectionMatrix( );
        void InitializeVectorGraphics( );
        
        // Demo rendering functions
        void RenderBasicShapes( ) const;
        void RenderAnimatedShapes( ) const;
        void RenderGradientShapes( ) const;
        void RenderCurveDemo( ) const;
        void RenderTransformDemo( ) const;
        void RenderComplexDemo( ) const;
        
        // Helper functions
        static Float_4 GetAnimatedColor( float time, float offset = 0.0f );
        static Float_2 GetCircularPosition( float radius, float angle, const Float_2& center );
    };
} // namespace DenOfIz
