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

#include <DenOfIzGraphics/Backends/Interface/IRayTracingAccelerationStructure.h>
#include "DX12Context.h"

namespace DenOfIz
{
    class DX12RayTracingAccelerationStructure : public IRayTracingAccelerationStructure
    {
    private:
        DX12Context *m_context;
    public:
        DX12RayTracingAccelerationStructure( DX12Context * context );
        void Build( const AccelerationStructureDesc &desc ) override;
        void Update( const AccelerationStructureDesc &desc ) override;
    private:
        void BuildTopLevel( const AccelerationStructureTopLevelDesc &desc );
        void BuildBottomLevel( const AccelerationStructureBottomLevelDesc &desc );
    };
} // namespace DenOfIz
