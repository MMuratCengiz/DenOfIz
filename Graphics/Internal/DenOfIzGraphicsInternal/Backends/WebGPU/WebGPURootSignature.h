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

#include <vector>
#include <webgpu/webgpu.h>
#include "DenOfIzGraphicsInternal/Backends/Interface/IRootSignature.h"
#include "WebGPUBindGroupLayout.h"

namespace DenOfIz
{
    struct WebGPURootConstantInfo
    {
        uint32_t Binding;
        uint32_t NumBytes;
    };

    class WebGPURootSignature final : public IRootSignature
    {
        WebGPUContext                       *m_context;
        DenOfIz_RootSignatureDesc            m_desc;
        std::vector<WebGPUBindGroupLayout *> m_bindGroupLayouts;
        std::vector<WGPUBindGroupLayout>     m_wgpuBindGroupLayouts;
        WGPUPipelineLayout                   m_pipelineLayout = nullptr;
        WGPUBindGroupLayout                  m_emptyLayout    = nullptr;
        WGPUBindGroup                        m_emptyBindGroup = nullptr;
        std::vector<WebGPURootConstantInfo>  m_rootConstants;

    public:
        WebGPURootSignature( WebGPUContext *context, const DenOfIz_RootSignatureDesc &desc );
        ~WebGPURootSignature( ) override;

        [[nodiscard]] WGPUPipelineLayout                          GetPipelineLayout( ) const;
        [[nodiscard]] const std::vector<WGPUBindGroupLayout>     &GetWGPUBindGroupLayouts( ) const;
        [[nodiscard]] const std::vector<WebGPURootConstantInfo>  &RootConstants( ) const;
        [[nodiscard]] uint32_t                                    NumRootConstants( ) const;
        [[nodiscard]] const std::vector<WebGPUBindGroupLayout *> &BindGroupLayouts( ) const;
        [[nodiscard]] WebGPUBindGroupLayout                      *GetBindGroupLayout( uint32_t registerSpace ) const;
        [[nodiscard]] WGPUBindGroup                               GetEmptyBindGroup( ) const;
        [[nodiscard]] uint32_t                                    GetNumBindGroupSlots( ) const;

    private:
        void CreatePipelineLayout( );
    };

} // namespace DenOfIz
