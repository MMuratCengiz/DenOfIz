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

#include "DenOfIzExamples/IExample.h"
#include "DenOfIzExamples/QuadPipeline.h"

namespace DenOfIz
{
    class RootConstantExample final : public IExample
    {
        std::array<float, 4>          m_color = { 0.3f, 0.1f, 0.7f, 1.0f };
        std::unique_ptr<QuadPipeline> m_quadPipeline;
        uint32_t                      m_rgbIterator = 0;

    public:
        ~RootConstantExample( ) override;
        void                     Init( ) override;
        void                     ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference ) override;
        void                     HandleEvent( DenOfIz_Event &event ) override;
        void                     Update( ) override;
        void                     Quit( ) override;
        void                     Render( uint32_t frameIndex, DenOfIz_CommandList commandList ) override;
        struct ExampleWindowDesc WindowDesc( ) override
        {
            auto windowDesc  = DenOfIz::ExampleWindowDesc( );
            windowDesc.Title = "PushConstantExample";
            return windowDesc;
        }
    };
} // namespace DenOfIz
