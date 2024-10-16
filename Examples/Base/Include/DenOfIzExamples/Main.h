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

#include "IExample.h"

namespace DenOfIz
{
    // Note this function will delete example object to ensure leaked objects are reported correctly
    DZ_EXAMPLES_API int Main( IExample *example );
} // namespace DenOfIz

#define DZ_EXAMPLE_MAIN( ExampleType )                                                                                                                                             \
    int main( )                                                                                                                                                                    \
    {                                                                                                                                                                              \
        ExampleType *example = new ExampleType();                                                                                                                                                      \
        DenOfIz::Main( example );                                                                                                                                                  \
    }
