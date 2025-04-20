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

#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAssetWriter.h>
#include <unordered_map>

using namespace DenOfIz;

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

#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAssetWriter.h>
#include <DenOfIzGraphics/Assets/Serde/Common/AssetWriterHelpers.h>

using namespace DenOfIz;

SkeletonAssetWriter::SkeletonAssetWriter(const SkeletonAssetWriterDesc& desc) : m_writer(desc.Writer)
{
    if (!m_writer)
    {
        LOG(FATAL) << "BinaryWriter cannot be null for SkeletonAssetWriter";
    }
}

SkeletonAssetWriter::~SkeletonAssetWriter() = default;

void SkeletonAssetWriter::Write(const SkeletonAsset& skeletonAsset ) const
{
    m_writer->WriteUInt64(skeletonAsset.Magic);
    m_writer->WriteUInt32(skeletonAsset.Version);
    m_writer->WriteUInt64(skeletonAsset.NumBytes);
    m_writer->WriteString(skeletonAsset.Uri.ToString());
    m_writer->WriteString(skeletonAsset.Name);

    m_writer->WriteUInt32(skeletonAsset.Joints.NumElements());
    for (size_t i = 0; i < skeletonAsset.Joints.NumElements(); ++i)
    {
        const Joint& joint = skeletonAsset.Joints.GetElement(i);

        m_writer->WriteString(joint.Name);
        m_writer->WriteFloat_4x4(joint.InverseBindMatrix);
        m_writer->WriteFloat_3( joint.LocalTranslation );
        m_writer->WriteFloat_4( joint.LocalRotationQuat );
        m_writer->WriteFloat_3( joint.LocalScale );
        m_writer->WriteUInt32(joint.Index);
        m_writer->WriteInt32(joint.ParentIndex);
        m_writer->WriteUInt32(joint.ChildIndices.NumElements());

        for (size_t j = 0; j < joint.ChildIndices.NumElements(); ++j)
        {
            m_writer->WriteUInt32(joint.ChildIndices.GetElement(j));
        }
    }

    m_writer->Flush();
}