#pragma once

#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphicsInternal/Backends/Common/InternalAutoSync.h"
#import "AppKit/AppKit.h"
#import "Metal/Metal.h"
#import "QuartzCore/CAMetalLayer.h"

#import <metal_irconverter/metal_irconverter.h>
#import <metal_irconverter_runtime/metal_irconverter_runtime.h>

#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include <mutex>
#include <vector>

#ifdef __APPLE__
#import <CoreFoundation/CoreFoundation.h>
#endif

namespace DenOfIz
{

    struct MetalContext
    {
        id<MTLDevice>          Device;
        id<MTLCommandQueue>    CommandQueue;
        DenOfIz_PhysicalDevice SelectedDeviceInfo;

        id<MTLHeap>   ReadOnlyHeap;

        InternalAutoSync *AutoSync;

        id<MTLCaptureScope> CaptureScope;
        bool                CaptureEnabled;
        bool                IsCapturing;
        std::string         CaptureOutputPath;

        std::mutex                        InFlightMutex;
        std::vector<id<MTLCommandBuffer>> InFlightCommandBuffers;

        static bool IsFinished( const id<MTLCommandBuffer> &commandBuffer )
        {
            const MTLCommandBufferStatus status = commandBuffer.status;
            return status == MTLCommandBufferStatusCompleted || status == MTLCommandBufferStatusError;
        }

        // Call right after committing a command buffer.
        void TrackCommandBuffer( const id<MTLCommandBuffer> &commandBuffer )
        {
            if ( commandBuffer == nil )
            {
                return;
            }
            std::lock_guard<std::mutex> lock( InFlightMutex );
            RemoveFinishedLocked( );
            InFlightCommandBuffers.push_back( commandBuffer );
        }

        std::vector<id<MTLCommandBuffer>> InFlightSnapshot( )
        {
            std::lock_guard<std::mutex> lock( InFlightMutex );
            RemoveFinishedLocked( );
            return InFlightCommandBuffers;
        }

        void WaitIdle( )
        {
            const std::vector<id<MTLCommandBuffer>> inFlight = InFlightSnapshot( );
            for ( const id<MTLCommandBuffer> &commandBuffer : inFlight )
            {
                [commandBuffer waitUntilCompleted];
            }
        }

        // InFlightMutex must be held.
        void RemoveFinishedLocked( )
        {
            std::vector<id<MTLCommandBuffer>> remaining;
            remaining.reserve( InFlightCommandBuffers.size( ) );
            for ( const id<MTLCommandBuffer> &commandBuffer : InFlightCommandBuffers )
            {
                if ( !IsFinished( commandBuffer ) )
                {
                    remaining.push_back( commandBuffer );
                }
            }
            InFlightCommandBuffers.swap( remaining );
        }
    };

} // namespace DenOfIz

// clang-format off
#define DZ_NS_STRING( nsString ) ( nsString == nil ? "" : std::string( [ nsString cStringUsingEncoding: NSUTF8StringEncoding ] ) )
inline void LogNSError(const char* prefix, NSError* error) {
    spdlog::error("{} error.localizedDescription ({}), error.code ({}), error.domain ({}), error.userInfo ({}), error.localizedFailureReason ({}), error.localizedRecoverySuggestion ({})",
        prefix,
        DZ_NS_STRING(error.localizedDescription),
        error.code,
        DZ_NS_STRING(error.domain),
        DZ_NS_STRING(error.userInfo.description),
        DZ_NS_STRING(error.localizedFailureReason),
        DZ_NS_STRING(error.localizedRecoverySuggestion));
}
#define DZ_LOG_NS_ERROR(prefix, error) LogNSError(prefix, error)
// clang-format on

#define DENOFIZ_METAL_TRIANGLE_INTERSECTION_SHADER 0
#define DENOFIZ_METAL_PROCEDURAL_INTERSECTION_SHADER 1