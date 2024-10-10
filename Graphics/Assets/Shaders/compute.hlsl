RWStructuredBuffer<float> buffer : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
    buffer[dispatchThreadID.x] = dispatchThreadID.x * 10.0f;
}