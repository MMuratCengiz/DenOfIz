/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <vector>
#include "DenOfIzGraphics/Backends/Interface/BindGroupLayout.h"
#include "../Common/DeferredDestroyQueue.h"

namespace DenOfIz
{

    class IBindGroupLayout : public IDeviceObject
    {
    protected:
        DenOfIz_BindGroupLayoutDesc      m_storedDesc;
        std::vector<DenOfIz_BindingDesc> m_bindingsCopy;

    public:
        explicit IBindGroupLayout( const DenOfIz_BindGroupLayoutDesc &desc );
        virtual ~IBindGroupLayout( ) = default;

        [[nodiscard]] const DenOfIz_BindGroupLayoutDesc &Desc( ) const;
    };

} // namespace DenOfIz
