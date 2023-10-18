struct Input
{
    float4 position : SV_Position;
};

float4 main(Input input) : SV_Target
{
    return 1.f;
}