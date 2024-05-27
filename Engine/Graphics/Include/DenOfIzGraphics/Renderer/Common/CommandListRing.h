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

#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>

namespace DenOfIz
{

class CommandListRing
{
private:
	std::vector<std::unique_ptr<ICommandList>> m_commandLists;
	size_t m_nextInFlight = 0;
	ILogicalDevice* m_logicalDevice;
public:
	CommandListRing(ILogicalDevice* logicalDevice) : m_logicalDevice(logicalDevice) {}

	inline void NewCommandList(CommandListCreateInfo createInfo)
	{
		m_commandLists.push_back(m_logicalDevice->CreateCommandList(createInfo));
	}

	ICommandList* GetNextInFlight()
	{
		auto next = m_commandLists[m_nextInFlight].get();
		m_nextInFlight = (m_nextInFlight + 1) % m_commandLists.size();
		return next;
	}
};

}