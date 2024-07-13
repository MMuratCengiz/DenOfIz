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

#include <DenOfIzGraphics/Backends/Interface/IResourceBindGroup.h>
#include "DX12BufferResource.h"
#include "DX12Context.h"
#include "DX12RootSignature.h"
#include "DX12TextureResource.h"

namespace DenOfIz
{
    struct RootParameterHandle
    {
        D3D12_ROOT_PARAMETER_TYPE   Type;
        int                         Index;
        D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle;
    };

    enum class BindingType
    {
        RootConstant,
        DescriptorTable
    };

    class DX12ResourceBindGroup : public IResourceBindGroup
    {
    private:
        DX12Context                     *m_context;
        std::vector<ID3D12Resource2 *>   m_resources;
        DX12RootSignature               *m_rootSignature;
        std::vector<RootParameterHandle> m_samplerParams;
        std::vector<RootParameterHandle> m_cbvSrvUavParams;
        std::vector<RootParameterHandle> m_rootConstants;

    public:
        DX12ResourceBindGroup(DX12Context *context, ResourceBindGroupDesc desc);

        const std::vector<RootParameterHandle> &GetRootConstantHandles() const
        {
            return m_rootConstants;
        }

        const std::vector<RootParameterHandle> &GetSamplerHandles() const
        {
            return m_samplerParams;
        }

        const std::vector<RootParameterHandle> &GetDescriptorTableHandles() const
        {
            return m_cbvSrvUavParams;
        }

        const std::vector<ID3D12Resource2 *> &GetResources() const
        {
            return m_resources;
        }

        ID3D12RootSignature *GetRootSignature() const
        {
            return m_rootSignature->GetRootSignature();
        }

        void Update(UpdateDesc desc) override;
    protected:
        void BindTexture(ITextureResource *resource) override;
        void BindBuffer(IBufferResource *resource) override;
        void BindSampler(ISampler *sampler) override;

    };

} // namespace DenOfIz
