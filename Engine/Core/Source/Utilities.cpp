// Blazar Engine - 3D Game Engine
// Copyright (c) 2020-2021 Muhammed Murat Cengiz
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <DenOfIzCore/Utilities.h>
#include <fstream>
#include <vector>

using namespace DenOfIz;

std::string Utilities::ReadFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if ( !file.is_open() )
    {
        throw std::runtime_error("failed to open file!");
    }

    std::string data;

    file.seekg(0, std::ios::end);
    data.reserve(file.tellg());
    file.seekg(0, std::ios::beg);
    data.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return std::move(data);
}

std::string Utilities::GetFileDirectory(const std::string &file, bool includeFinalSep)
{
    size_t sepUnixIdx = file.find_last_of("/\\");
    size_t sepWinIdx  = file.find_last_of("\\\\");

    int finalSepSub = includeFinalSep ? 1 : 0;

    if ( sepUnixIdx != -1 )
    {
        return file.substr(0, sepUnixIdx + finalSepSub);
    }
    if ( sepWinIdx != -1 )
    {
        return file.substr(0, sepWinIdx - finalSepSub);
    }

    return file;
}

std::string Utilities::GetFilename(const std::string &file)
{
    size_t sepUnixIdx = file.find_last_of("/\\");
    size_t sepWinIdx  = file.find_last_of("\\\\");

    if ( sepUnixIdx != -1 )
    {
        return file.substr(sepUnixIdx + 1);
    }
    if ( sepWinIdx != -1 )
    {
        return file.substr(0, sepWinIdx + 1);
    }

    return file;
}

std::string Utilities::CombineDirectories(const std::string &directory, const std::string &file)
{
    std::string dir = GetFileDirectory(directory);
    std::string f   = GetFilename(file);

    return dir + f;
}
