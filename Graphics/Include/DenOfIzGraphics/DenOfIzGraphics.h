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

#include "DenOfIzGraphics/Data/AlignedDataWriter.h"
#include "DenOfIzGraphics/Data/Geometry.h"
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"
#include "DenOfIzGraphics/Renderer/Sync/FrameSync.h"
#include "DenOfIzGraphics/Renderer/Sync/ResourceTracking.h"
#include "DenOfIzGraphics/Assets/Assets.h"

// Input System
#include "DenOfIzGraphics/Input/Controller.h"
#include "DenOfIzGraphics/Input/Event.h"
#include "DenOfIzGraphics/Input/InputData.h"
#include "DenOfIzGraphics/Input/InputSystem.h"
#include "DenOfIzGraphics/Input/Window.h"

#include "DenOfIzGraphics/Animation/AnimationStateManager.h"
#include "DenOfIzGraphics/Animation/OzzAnimation.h"
#include "DenOfIzGraphics/Utilities/InteropUtilities.h"
#include "DenOfIzGraphics/Utilities/FrameDebugRenderer.h"
#include "DenOfIzGraphics/Utilities/Time.h"
#include "DenOfIzGraphics/Utilities/StepTimer.h"

#include "DenOfIzGraphics/Assets/Vector2d/QuadRenderer.h"
#include "DenOfIzGraphics/Assets/Vector2d/ThorVGWrapper.h"

#include "DenOfIzGraphics/Assets/Import/AssetScanner.h"
#include "DenOfIzGraphics/Assets/Import/AssimpImporter.h"
#include "DenOfIzGraphics/Assets/Import/FontImporter.h"
#include "DenOfIzGraphics/Assets/Import/ShaderImporter.h"
#include "DenOfIzGraphics/Assets/Import/TextureImporter.h"
#include "DenOfIzGraphics/Assets/Import/VGImporter.h"

#include "DenOfIzGraphics/UI/Clay.h"
#include "DenOfIzGraphics/UI/ClayData.h"
#include "DenOfIzGraphics/UI/IClayContext.h"

#include "DenOfIzGraphics/UI/Widgets/Widget.h"
#include "DenOfIzGraphics/UI/Widgets/CheckboxWidget.h"
#include "DenOfIzGraphics/UI/Widgets/SliderWidget.h"
#include "DenOfIzGraphics/UI/Widgets/TextFieldWidget.h"
#include "DenOfIzGraphics/UI/Widgets/DropdownWidget.h"
#include "DenOfIzGraphics/UI/Widgets/ColorPickerWidget.h"
#include "DenOfIzGraphics/UI/Widgets/ResizableContainerWidget.h"
#include "DenOfIzGraphics/UI/Widgets/DockableContainerWidget.h"

#ifdef SWIGJAVA
#define NULL 0  // Not sure where we lose this on MacOS to be investigated
#endif
