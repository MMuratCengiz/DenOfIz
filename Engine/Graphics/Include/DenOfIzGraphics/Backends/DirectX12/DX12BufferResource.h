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

#include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
#include "DX12Context.h"
#include "DX12EnumConverter.h"
#include <DenOfIzCore/Utilities.h>

namespace DenOfIz
{

    class DX12BufferResource : public IBufferResource
    {
    private:
        DX12Context                      *m_context;
        BufferDesc                        m_desc;
        wil::com_ptr<ID3D12Resource2>     m_resource;
        wil::com_ptr<D3D12MA::Allocation> m_allocation;
        D3D12_CPU_DESCRIPTOR_HANDLE       m_cpuHandle;
        D3D12_ROOT_PARAMETER_TYPE         m_rootParameterType;

        bool     allocated = false;
        uint32_t m_stride  = 0;

    public:
        DX12BufferResource(DX12Context *context, const BufferDesc &desc);
        void CreateView(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle);

        ID3D12Resource2 *GetResource() const
        {
            return m_resource.get();
        }
        void *MapMemory() override;
        void  UnmapMemory() override;

        ~DX12BufferResource() override;

        const D3D12_ROOT_PARAMETER_TYPE &GetRootParameterType() const
        {
            return m_rootParameterType;
        }

        uint32_t GetStride() const
        {
            return m_stride;
        }
    };

} // namespace DenOfIz
