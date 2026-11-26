/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/RayTracing/ShaderBindingTable.h"
#include "../../Common/DeferredDestroyQueue.h"

namespace DenOfIz
{

    class IShaderBindingTable : public IDeviceObject
    {
    public:
        virtual void Resize( const DenOfIz_SBTSizeDesc &resizeDesc )                         = 0;
        virtual void BindRayGenerationShader( const DenOfIz_RayGenerationBindingDesc &desc ) = 0;
        virtual void BindHitGroup( const DenOfIz_HitGroupBindingDesc &desc )                 = 0;
        virtual void BindMissShader( const DenOfIz_MissBindingDesc &desc )                   = 0;
        virtual void Build( )                                                                = 0;
        virtual ~IShaderBindingTable( )                                                      = default;
    };
} // namespace DenOfIz
