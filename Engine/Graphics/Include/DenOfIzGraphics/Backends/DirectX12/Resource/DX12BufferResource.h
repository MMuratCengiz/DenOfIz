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

#include <DenOfIzGraphics/Backends/Interface/IResource.h>
#include "../DX12Context.h"
#include "../DX12EnumConverter.h"

namespace DenOfIz
{

    class DX12BufferResource : public IBufferResource
    {
    private:
        DX12Context *m_context;
        BufferCreateInfo m_createInfo;
        ID3D12Resource2 *m_resource;
        D3D12MA::Allocation *m_allocation;

    public:
        DX12BufferResource(DX12Context *context, const BufferCreateInfo &createInfo);
        ID3D12Resource2 *GetResource() { return m_resource; }
        ~DX12BufferResource() override;
        void Deallocate() override;

    protected:
        void Allocate(const void *data) override;
        void CreateBufferView();
    };

} // namespace DenOfIz
