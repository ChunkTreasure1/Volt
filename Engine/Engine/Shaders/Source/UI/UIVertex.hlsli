#pragma once

struct UIVertex
{
    float4 position : POSITION;
    float2 texCoords : TEXCOORDS;
    uint imageHandle : IMAGEHANDLE;
    uint widgetId : WIDGETID;
};