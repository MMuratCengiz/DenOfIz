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

#include <DenOfIzGraphics/Renderer/Graph/RenderGraph.h>

using namespace DenOfIz;
using namespace RenderGraphInternal;

RenderGraph::RenderGraph( const RenderGraphDesc &desc ) : m_presentNode( { } ), m_desc( desc )
{
}

void RenderGraph::Reset( )
{
    m_nodes.clear( );
}

void RenderGraph::AddNode( const NodeDesc &desc )
{
    m_nodes.push_back( desc );
}

void RenderGraph::AddPresentNode( const PresentNodeDesc &desc )
{
    m_presentNode = desc;
}

void RenderGraph::BuildGraph( )
{
    CommandListPoolDesc poolDesc{ };
    poolDesc.NumCommandLists = m_nodes.size( );
    m_commandListPools.clear( );
    for ( int i = 0; i < m_desc.NumFrames; ++i )
    {
        m_commandListPools.push_back( m_desc.LogicalDevice->CreateCommandListPool( poolDesc ) );
    }

    std::unordered_map<std::string, GraphNode> allNodes;
    for ( auto &node : m_nodes )
    {
        GraphNode graphNode{ };
        graphNode.Index = allNodes.size( );

        for ( uint8_t i = 0; i < m_desc.NumFrames; i++ )
        {
            const auto newContext   = std::make_unique<NodeExecutionContext>( );
            newContext->CommandList = m_commandListPools[ i ]->GetCommandLists( )[ graphNode.Index ];

            graphNode.Contexts.push_back( std::move( newContext ) );
        }
    }
    const std::unordered_set<std::string> processedNodes;
    while ( processedNodes.size( ) < m_nodes.size( ) )
    {
        for ( auto &node : m_nodes )
        {
            bool isDependencyResolved = true;
            for ( auto &dependency : node.Dependencies )
            {
                if ( !allNodes.contains( dependency ) )
                {
                    LOG( FATAL ) << "Dependency " << dependency << " not found!";
                }
                if ( !processedNodes.contains( dependency ) )
                {
                    isDependencyResolved = false;
                    break;
                }
            }

            if ( isDependencyResolved )
            {
                GraphNode graphNode;
            }
        }
    }
}

void RenderGraph::Update( )
{
}

void RenderGraph::WaitIdle( )
{
}
