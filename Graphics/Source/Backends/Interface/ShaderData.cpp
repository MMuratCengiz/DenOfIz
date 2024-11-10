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

#include <DenOfIzGraphics/Backends/Interface/ShaderData.h>

using namespace DenOfIz;

void LocalSignatureDesc::AddCbv( uint32_t binding, uint32_t registerSpace )
{
    Bindings.AddElement( { .Type = ResourceBindingType::ConstantBuffer, .Binding = binding, .RegisterSpace = registerSpace } );
}

void LocalSignatureDesc::AddSrv( uint32_t binding, uint32_t registerSpace )
{
    Bindings.AddElement( { .Type = ResourceBindingType::ShaderResource, .Binding = binding, .RegisterSpace = registerSpace } );
}

void LocalSignatureDesc::AddUav( uint32_t binding, uint32_t registerSpace )
{
    Bindings.AddElement( { .Type = ResourceBindingType::UnorderedAccess, .Binding = binding, .RegisterSpace = registerSpace } );
}

void LocalSignatureDesc::AddSampler( uint32_t binding, uint32_t registerSpace )
{
    Bindings.AddElement( { .Type = ResourceBindingType::Sampler, .Binding = binding, .RegisterSpace = registerSpace } );
}
