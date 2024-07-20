#pragma once

struct UIVertex
{
    float4 position : POSITION;
    float2 texCoords : TEXCOORDS;
    float4 color : COLOR;
    uint imageHandle : IMAGEHANDLE;
    uint widgetId : WIDGETID;
};