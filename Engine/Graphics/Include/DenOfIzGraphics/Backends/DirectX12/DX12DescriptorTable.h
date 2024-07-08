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

#include <DenOfIzGraphics/Backends/Interface/IDescriptorTable.h>
#include "DX12BufferResource.h"
#include "DX12Context.h"
#include "DX12RootSignature.h"
#include "DX12TextureResource.h"

namespace DenOfIz
{

    class DX12DescriptorTable : public IDescriptorTable
    {
    private:
        std::vector<ID3D12Resource2 *> m_resources;
        ID3D12RootSignature           *m_rootSignature;

    public:
        DX12DescriptorTable(DX12Context *context, DescriptorTableDesc desc);

        void BindImage(ITextureResource *resource) override;
        void BindBuffer(IBufferResource *resource) override;

        const std::vector<ID3D12Resource2 *> &GetResources() const
        {
            return m_resources;
        }
        ID3D12RootSignature *GetRootSignature() const
        {
            return m_rootSignature;
        }
    };

} // namespace DenOfIz
