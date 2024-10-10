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

#include "DenOfIzGraphics/Utilities/BitSet.h"
#include "gtest/gtest.h"

enum class TestEnum : uint32_t
{
    First  = 1 << 1,
    Second = 1 << 2,
    Third  = 1 << 3
};

TEST(BitSetTest, None)
{
    DenOfIz::BitSet<TestEnum> bitset;
    ASSERT_TRUE(bitset.None());
}

TEST(BitSetTest, IsSet)
{
    DenOfIz::BitSet<TestEnum> bitset;
    bitset |= TestEnum::First;
    ASSERT_TRUE(bitset.IsSet(TestEnum::First));
    ASSERT_FALSE(bitset.IsSet(TestEnum::Second));
}

TEST(BitSetTest, All)
{
    DenOfIz::BitSet<TestEnum> bitset;
    bitset |= TestEnum::First;
    bitset |= TestEnum::Second;
    ASSERT_TRUE(bitset.All({ TestEnum::First, TestEnum::Second }));
    ASSERT_FALSE(bitset.All({ TestEnum::First, TestEnum::Third }));
}

TEST(BitSetTest, Any)
{
    DenOfIz::BitSet<TestEnum> bitset;
    bitset |= TestEnum::Second;
    ASSERT_TRUE(bitset.Any({ TestEnum::First, TestEnum::Second }));
    ASSERT_FALSE(bitset.Any({ TestEnum::First, TestEnum::Third }));
}

TEST(BitSetTest, BitwiseOperations)
{
    DenOfIz::BitSet<TestEnum> bitset1;
    bitset1 |= TestEnum::First;
    DenOfIz::BitSet<TestEnum> bitset2;
    bitset2 |= TestEnum::Second;
    auto result = bitset1 | bitset2;
    ASSERT_TRUE(result.IsSet(TestEnum::First));
    ASSERT_TRUE(result.IsSet(TestEnum::Second));
    result &= TestEnum::First;
    ASSERT_TRUE(result.IsSet(TestEnum::First));
    ASSERT_FALSE(result.IsSet(TestEnum::Second));
}
