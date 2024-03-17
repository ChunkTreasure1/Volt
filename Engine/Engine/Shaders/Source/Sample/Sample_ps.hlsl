struct Output
{
    [[vt::rgba16f]] float4 albedo : SV_Target0;
    [[vt::rgba16f]] float4 materialEmissive : SV_Target1;
    [[vt::rgba16f]] float4 normalEmissive : SV_Target2;
    [[vt::d32f]];
};

struct Input
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

Output main(Input input)
{
    Output output;
    output.albedo = input.color;
    output.materialEmissive = float4(0.f, 1.f, 0.f, 0.f);
    output.normalEmissive = float4(0.f, 1.f, 0.f, 0.f);
    return output;
}