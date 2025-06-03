#pragma once

#include <DenOfIzGraphics/Backends/Interface/CommonData.h>
#import "AppKit/AppKit.h"
#import "Metal/Metal.h"
#import "MetalKit/MetalKit.h"

#import <metal_irconverter/metal_irconverter.h>
#import <metal_irconverter_runtime/metal_irconverter_runtime.h>

namespace DenOfIz
{

    struct MetalContext
    {
        id<MTLDevice>       Device;
        id<MTLCommandQueue> CommandQueue;
        PhysicalDevice      SelectedDeviceInfo;

        id<MTLHeap>   ReadOnlyHeap;
    };

} // namespace DenOfIz

// clang-format off
#define DZ_NS_STRING( nsString ) ( nsString == nil ? "" : std::string( [ nsString cStringUsingEncoding: NSUTF8StringEncoding ] ) )
#define DZ_LOG_NS_ERROR( prefix, error ) LOG( ERROR ) << prefix \
                                                    << " error.localizedDescription (" << DZ_NS_STRING(error.localizedDescription) << ")," \
                                                    << " error.code (" << error.code << ")," \
                                                    << " error.domain (" << DZ_NS_STRING(error.domain) << ")," \
                                                    << " error.userInfo (" << DZ_NS_STRING(error.userInfo.description) << ")," \
                                                    << " error.localizedFailureReason (" << DZ_NS_STRING(error.localizedFailureReason) << ")," \
                                                    << " error.localizedRecoverySuggestion (" << DZ_NS_STRING(error.localizedRecoverySuggestion) << "),";
// clang-format on
