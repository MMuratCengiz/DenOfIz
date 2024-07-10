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

#include <DenOfIzGraphics/Data/TextureLoader.h>

using namespace DenOfIz;

TextureData TextureLoader::LoadTexture(const std::string &path)
{
#ifdef DZ_USE_STB_IMAGE
    return LoadTextureSTB(path);
#endif
}

TextureData TextureLoader::LoadTextureSTB(const std::string &path)
{
    int width, height, channels;

    stbi_uc *contents = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if ( contents == nullptr )
    {
        LOG(WARNING) << "Error loading texture: " << path << ", reason:" << stbi_failure_reason();
        return TextureData{};
    }

    return TextureData{
        .Width    = width,
        .Height   = height,
        .Channels = channels,
        .Contents = contents,
    };
}