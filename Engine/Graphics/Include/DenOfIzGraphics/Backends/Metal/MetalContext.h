#pragma once

#import "AppKit/AppKit.h"
#import "Metal/Metal.h"
#import "MetalKit/MetalKit.h"

namespace DenOfIz
{

    struct MetalContext
    {
        id<MTLDevice> Device;
        id<MTLCommandQueue> CommandQueue;
    };

} // namespace DenOfIz
