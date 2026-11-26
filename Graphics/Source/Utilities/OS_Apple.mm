/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphics/Utilities/OS.h"

#import <AppKit/AppKit.h>

extern "C"
{

float DenOfIz_OS_GetDisplayScale( int display )
{
    return 1.0f;
}

float DenOfIz_OS_GetWindowSizeScaleFactor( int display )
{
    @autoreleasepool {
        NSArray *screens = [NSScreen screens];
        if ( display >= 0 && display < (int)[screens count] )
        {
            NSScreen *screen = [screens objectAtIndex:display];
            return (float)[screen backingScaleFactor];
        }
        else if ( [screens count] > 0 )
        {
            NSScreen *mainScreen = [NSScreen mainScreen];
            return (float)[mainScreen backingScaleFactor];
        }
    }
    return 1.0f;
}

}
