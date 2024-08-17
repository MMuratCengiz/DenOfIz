#include <DenOfIzGraphics/Backends/Metal/MetalHeap.h>

using namespace DenOfIz;

MetalHeap::MetalHeap( MetalContext *context, const MetalHeapDesc &desc ) : m_context( context )
{
    MTLHeapDescriptor *heapDesc = [[MTLHeapDescriptor alloc] init];
    heapDesc.size               = 4 * 1024;
    heapDesc.storageMode        = MTLStorageModeShared;
    heapDesc.hazardTrackingMode = MTLHazardTrackingModeTracked;
    heapDesc.type               = MTLHeapTypeAutomatic;
    m_heap                      = [m_context->Device newHeapWithDescriptor:heapDesc];
}

uint64_t MetalHeap::GetNextHandle( uint32_t count )
{

    return 0;
}
