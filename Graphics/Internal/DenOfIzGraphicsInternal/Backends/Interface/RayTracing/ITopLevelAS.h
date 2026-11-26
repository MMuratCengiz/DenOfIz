/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/RayTracing/TopLevelAS.h"
#include "../../Common/DeferredDestroyQueue.h"

namespace DenOfIz
{
    class ITopLevelAS : public IDeviceObject
    {
    public:
        virtual ~ITopLevelAS( )                                                             = default;
        virtual void   UpdateInstanceTransforms( const DenOfIz_UpdateTransformsDesc &desc ) = 0;
        virtual size_t BuildNumBytes( ) const                                               = 0;
    };
} // namespace DenOfIz
