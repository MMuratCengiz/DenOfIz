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

#include <atomic>
#include "DenOfIzGraphicsInternal/Backends/Interface/ISemaphore.h"
#include "WebGPUContext.h"

using namespace DenOfIz;

class WebGPUSemaphore final : public ISemaphore
{
    WebGPUContext    *m_context;
    std::atomic<bool> m_notified{ false };

public:
    explicit WebGPUSemaphore( WebGPUContext *context );
    ~WebGPUSemaphore( ) override;

    void Notify( ) override;
    bool IsCompleted( ) const override;

    bool IsNotified( ) const;
    void Reset( );
};
