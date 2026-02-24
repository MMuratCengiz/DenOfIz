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

#import "Metal/Metal.h"
#import "DenOfIzGraphics/Backends/MetalCapture.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/MetalLogicalDevice.h"
#import "DenOfIzGraphicsInternal/Utilities/Logging.h"

#define METAL_LOGICAL_DEVICE_IMPL( handle ) static_cast<DenOfIz::MetalLogicalDevice *>( DENOFIZ_FROM_HANDLE( DenOfIz::ILogicalDevice, handle ) )

extern "C"
{

    bool DenOfIz_Metal_BeginGpuCapture( DenOfIz_LogicalDevice device )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return false;
        }

        DenOfIz::MetalLogicalDevice *metalDevice = METAL_LOGICAL_DEVICE_IMPL( device );
        DenOfIz::MetalContext       *context     = metalDevice->Context( );

        if ( !context->CaptureEnabled )
        {
            spdlog::warn( "Metal GPU capture is not enabled. Make sure EnableValidationLayers is true." );
            return false;
        }

        if ( context->IsCapturing )
        {
            spdlog::warn( "Metal GPU capture is already in progress. Call EndGpuCapture first." );
            return false;
        }

        MTLCaptureManager *captureManager = [MTLCaptureManager sharedCaptureManager];

        NSString        *tempDir   = NSTemporaryDirectory( );
        NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
        [formatter setDateFormat:@"yyyy-MM-dd_HH-mm-ss"];
        NSString *timestamp  = [formatter stringFromDate:[NSDate date]];
        NSString *filename   = [NSString stringWithFormat:@"DenOfIz_Capture_%@.gputrace", timestamp];
        NSString *outputPath = [tempDir stringByAppendingPathComponent:filename];
        NSURL    *outputURL  = [NSURL fileURLWithPath:outputPath];

        if ( [[NSFileManager defaultManager] fileExistsAtPath:outputPath] )
        {
            [[NSFileManager defaultManager] removeItemAtPath:outputPath error:nil];
        }

        MTLCaptureDescriptor *captureDescriptor = [[MTLCaptureDescriptor alloc] init];
        captureDescriptor.captureObject         = context->Device;
        captureDescriptor.destination           = MTLCaptureDestinationGPUTraceDocument;
        captureDescriptor.outputURL             = outputURL;

        NSError *error = nil;
        if ( ![captureManager startCaptureWithDescriptor:captureDescriptor error:&error] )
        {
            if ( error )
            {
                DZ_LOG_NS_ERROR( "Failed to start Metal GPU capture", error );
            }
            else
            {
                spdlog::error( "Failed to start Metal GPU capture for unknown reason." );
            }
            return false;
        }

        context->IsCapturing       = true;
        context->CaptureOutputPath = [outputPath UTF8String];
        spdlog::info( "Metal GPU capture started. Capturing to: {}", context->CaptureOutputPath );
        return true;
    }

    void DenOfIz_Metal_EndGpuCapture( DenOfIz_LogicalDevice device )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return;
        }

        DenOfIz::MetalLogicalDevice *metalDevice = METAL_LOGICAL_DEVICE_IMPL( device );
        DenOfIz::MetalContext       *context     = metalDevice->Context( );

        if ( !context->CaptureEnabled )
        {
            return;
        }

        if ( !context->IsCapturing )
        {
            spdlog::warn( "No Metal GPU capture is in progress." );
            return;
        }

        MTLCaptureManager *captureManager = [MTLCaptureManager sharedCaptureManager];
        [captureManager stopCapture];
        context->IsCapturing = false;
        spdlog::info( "Metal GPU capture saved to: {}", context->CaptureOutputPath );
    }

    bool DenOfIz_Metal_IsCapturing( DenOfIz_LogicalDevice device )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return false;
        }

        DenOfIz::MetalLogicalDevice *metalDevice = METAL_LOGICAL_DEVICE_IMPL( device );
        DenOfIz::MetalContext       *context     = metalDevice->Context( );

        return context->IsCapturing;
    }
}
