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

#import <DenOfIzGraphics/Backends/Metal/MetalCommandList.h>

using namespace DenOfIz;

MetalCommandList::MetalCommandList( MetalContext *context, CommandListDesc desc ) : m_context( context ), m_desc( desc )
{
    m_commandBuffer = [m_context->CommandQueue commandBuffer];
}

MetalCommandList::~MetalCommandList( ) = default;

void MetalCommandList::Begin( )
{
}

void MetalCommandList::BeginRendering( const RenderingDesc &renderingInfo )
{

}

void MetalCommandList::EndRendering( )
{
}

void MetalCommandList::Execute( const ExecuteDesc &executeInfo )
{

    [m_commandBuffer commit];
}

void MetalCommandList::Present( ISwapChain *swapChain, uint32_t imageIndex, std::vector<ISemaphore *> waitOnLocks )
{
}

void MetalCommandList::BindPipeline( IPipeline *pipeline )
{
}

void MetalCommandList::BindVertexBuffer( IBufferResource *buffer )
{

}

void MetalCommandList::BindIndexBuffer( IBufferResource *buffer, const IndexType &indexType )
{
}

void MetalCommandList::BindViewport( float x, float y, float width, float height )
{
    return Viewport{0, 0, static_cast<float>(m_desc.width), static_cast<float>(m_desc.height)};
}

void MetalCommandList::BindScissorRect( float x, float y, float width, float height )
{
}

void MetalCommandList::BindResourceGroup( IResourceBindGroup *bindGroup )
{
}

void MetalCommandList::SetDepthBias( float constantFactor, float clamp, float slopeFactor )
{
}

void MetalCommandList::PipelineBarrier( const PipelineBarrierDesc &barrier )
{
}

void MetalCommandList::DrawIndexed( uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance )
{
}

void MetalCommandList::Draw( uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance )
{
}

void MetalCommandList::Dispatch( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ )
{
}

void MetalCommandList::CopyBufferRegion( const CopyBufferRegionDesc &copyBufferRegionInfo )
{
}

void MetalCommandList::CopyTextureRegion( const CopyTextureRegionDesc &copyTextureRegionInfo )
{
}

void MetalCommandList::CopyBufferToTexture( const CopyBufferToTextureDesc &copyBufferToTexture )
{
}

void MetalCommandList::CopyTextureToBuffer( const CopyTextureToBufferDesc &copyTextureToBuffer )
{
}

