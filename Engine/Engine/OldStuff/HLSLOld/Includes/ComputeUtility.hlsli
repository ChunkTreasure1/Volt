#ifndef COMPUTEUTILITY_H
#define COMPUTEUTILITY_H

// Source: https://github.com/GPUOpen-Effects/FidelityFX-Denoiser/blob/master/ffx-shadows-dnsr/ffx_denoiser_shadows_util.h
//  LANE TO 8x8 MAPPING
//  ===================
//  00 01 08 09 10 11 18 19 
//  02 03 0a 0b 12 13 1a 1b
//  04 05 0c 0d 14 15 1c 1d
//  06 07 0e 0f 16 17 1e 1f 
//  20 21 28 29 30 31 38 39 
//  22 23 2a 2b 32 33 3a 3b
//  24 25 2c 2d 34 35 3c 3d
//  26 27 2e 2f 36 37 3e 3f 
uint bitfield_extract(uint src, uint off, uint bits)
{
    uint mask = (1u << bits) - 1;
    return (src >> off) & mask;
} // ABfe
uint bitfield_insert(uint src, uint ins, uint bits)
{
    uint mask = (1u << bits) - 1;
    return (ins & mask) | (src & (~mask));
} // ABfiM
 
uint2 remap_lane_8x8(uint lane)
{
    return uint2(bitfield_insert(bitfield_extract(lane, 2u, 3u), lane, 1u)
        , bitfield_insert(bitfield_extract(lane, 3u, 3u)
            , bitfield_extract(lane, 1u, 2u), 2u));
}

uint2 RemapThreadIDToPixel(uint groupIndex, uint2 groupId)
{
    const uint2 groupThreadId = remap_lane_8x8(groupIndex);
    const uint2 pixelCoords = groupId.xy * 8 + groupThreadId;
    
    return pixelCoords;
}

#endif