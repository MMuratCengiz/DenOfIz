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

#include "DenOfIzGraphicsInternal/Backends/Metal/MetalPipelineCache.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include <Foundation/Foundation.h>

using namespace DenOfIz;

MetalPipelineCache::MetalPipelineCache( MetalContext *context, const DenOfIz_PipelineCacheDesc &desc )
	: m_context( context )
{
	DZ_NOT_NULL( context );
	
	MTLBinaryArchiveDescriptor *descriptor = [[MTLBinaryArchiveDescriptor alloc] init];
	NSError                    *error      = nil;

	if ( desc.Data != nullptr && desc.DataSize > 0 )
	{
		NSData *data = [NSData dataWithBytes:desc.Data length:desc.DataSize];
		
		m_binaryArchive = [m_context->Device newBinaryArchiveWithDescriptor:descriptor error:&error];
		if ( error != nil || m_binaryArchive == nil )
		{
			spdlog::warn( "Failed to create binary archive from cache data, creating empty archive" );
			m_binaryArchive = [m_context->Device newBinaryArchiveWithDescriptor:descriptor error:&error];
		}
	}
	else
	{
		m_binaryArchive = [m_context->Device newBinaryArchiveWithDescriptor:descriptor error:&error];
	}

	if ( error != nil || m_binaryArchive == nil )
	{
		spdlog::error( "Failed to create Metal binary archive" );
	}
}

MetalPipelineCache::~MetalPipelineCache( )
{
	if ( m_binaryArchive != nil )
	{
		m_binaryArchive = nil;
	}
}

size_t MetalPipelineCache::GetDataNumBytes( )
{
	if ( m_binaryArchive == nil )
	{
		return 0;
	}

	@autoreleasepool
	{
		NSError  *error   = nil;
		NSURL    *tempURL = [NSURL fileURLWithPath:[NSTemporaryDirectory( ) stringByAppendingPathComponent:[[NSUUID UUID] UUIDString]]];

		BOOL success = [m_binaryArchive serializeToURL:tempURL error:&error];

		if ( !success || error != nil )
		{
			return 0;
		}

		NSData *data = [NSData dataWithContentsOfURL:tempURL options:0 error:&error];
		[[NSFileManager defaultManager] removeItemAtURL:tempURL error:nil];

		if ( error != nil || data == nil )
		{
			return 0;
		}

		return [data length];
	}
}

bool MetalPipelineCache::GetData( DenOfIz_ByteArray &data )
{
	if ( m_binaryArchive == nil || !data.Elements )
	{
		return false;
	}

	@autoreleasepool
	{
		NSError  *error   = nil;
		NSURL    *tempURL = [NSURL fileURLWithPath:[NSTemporaryDirectory( ) stringByAppendingPathComponent:[[NSUUID UUID] UUIDString]]];

		BOOL success = [m_binaryArchive serializeToURL:tempURL error:&error];

		if ( !success || error != nil )
		{
			spdlog::error( "Failed to serialize Metal binary archive" );
			return false;
		}

		NSData *archiveData = [NSData dataWithContentsOfURL:tempURL options:0 error:&error];
		[[NSFileManager defaultManager] removeItemAtURL:tempURL error:nil];

		if ( error != nil || archiveData == nil )
		{
			spdlog::error( "Failed to read serialized Metal binary archive" );
			return false;
		}

		size_t dataSize = [archiveData length];
		if ( data.NumElements < dataSize )
		{
			return false;
		}

		std::memcpy( data.Elements, [archiveData bytes], dataSize );
		data.NumElements = dataSize;

		return true;
	}
}

id<MTLBinaryArchive> MetalPipelineCache::GetArchive( ) const
{
	return m_binaryArchive;
}