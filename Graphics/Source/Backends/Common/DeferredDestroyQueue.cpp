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

#include "DenOfIzGraphicsInternal/Backends/Common/DeferredDestroyQueue.h"

using namespace DenOfIz;

DeferredDestroyQueue::DeferredDestroyQueue( CaptureCompletion captureCompletion ) : m_captureCompletion( std::move( captureCompletion ) )
{
}

DeferredDestroyQueue::~DeferredDestroyQueue( )
{
    ReleaseAll( );
}

void DeferredDestroyQueue::Attach( DeferredDestroyQueue *queue, IDeviceObject *object )
{
    if ( object != nullptr )
    {
        object->m_destroyQueue = queue;
    }
}

void DeferredDestroyQueue::Destroy( IDeviceObject *object )
{
    if ( object == nullptr )
    {
        return;
    }
    DeferredDestroyQueue *queue = object->m_destroyQueue;
    if ( queue == nullptr )
    {
        delete object;
        return;
    }
    object->m_destroyQueue = nullptr;

    std::lock_guard lock( queue->m_mutex );
    queue->m_open.push_back( object );
}

void DeferredDestroyQueue::EndFrame( )
{
    std::vector<IDeviceObject *> completed;
    {
        std::lock_guard lock( m_mutex );
        if ( !m_open.empty( ) )
        {
            auto &[ Objects, IsComplete ]     = m_closed.emplace_back( );
            Objects    = std::move( m_open );
            IsComplete = m_captureCompletion ? m_captureCompletion( ) : Completion{ };
            m_open.clear( );
        }
        TakeCompleted( completed );
    }
    for ( const IDeviceObject *object : completed )
    {
        delete object;
    }
}

void DeferredDestroyQueue::Collect( )
{
    std::vector<IDeviceObject *> completed;
    {
        std::lock_guard lock( m_mutex );
        TakeCompleted( completed );
    }
    for ( const IDeviceObject *object : completed )
    {
        delete object;
    }
}

void DeferredDestroyQueue::ReleaseAll( )
{
    while ( true )
    {
        std::vector<IDeviceObject *> objects;
        {
            std::lock_guard lock( m_mutex );
            for ( auto &[ Objects, IsComplete ] : m_closed )
            {
                objects.insert( objects.end( ), Objects.begin( ), Objects.end( ) );
            }
            m_closed.clear( );
            objects.insert( objects.end( ), m_open.begin( ), m_open.end( ) );
            m_open.clear( );
        }
        if ( objects.empty( ) )
        {
            return;
        }
        for ( const IDeviceObject *object : objects )
        {
            delete object;
        }
    }
}

void DeferredDestroyQueue::TakeCompleted( std::vector<IDeviceObject *> &outObjects )
{
    while ( !m_closed.empty( ) )
    {
        auto &[ Objects, IsComplete ] = m_closed.front( );
        if ( IsComplete && !IsComplete( ) )
        {
            return;
        }
        outObjects.insert( outObjects.end( ), Objects.begin( ), Objects.end( ) );
        m_closed.pop_front( );
    }
}
