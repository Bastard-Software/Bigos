struct PushConstants
{
    uint bufferSize;
};

RWStructuredBuffer<uint> values : register(u0, space0);
[[vk::push_constant]]
PushConstants constants;

uint fibonacci(uint n)
{
    if (n <= 1)
    {
        return n;
    }
    uint curr = 1;
    uint prev = 1;
    for (uint i = 2; i < n; ++i)
    {
        uint temp = curr;
        curr += prev;
        prev = temp;
    }
    return curr;
}

[numthreads(1, 1, 1)]
void CSMain(uint3 GlobalInvocationID : SV_DispatchThreadID)
{
    uint index = GlobalInvocationID.x;
    if (index >= constants.bufferSize)
        return;
    values[index] = fibonacci(values[index]);
}