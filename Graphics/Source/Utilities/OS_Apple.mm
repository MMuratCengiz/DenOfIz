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
