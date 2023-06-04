#include "Defines.hlsli"

#ifndef BINDLESS_H
#define BINDLESS_H

Texture2D u_texture2DTable[1024] : register(BINDING_TEXTURE2DTABLE, SPACE_TEXTURES);
TextureCube u_textureCubeTable[1024] : register(BINDING_TEXTURECUBETABLE, SPACE_TEXTURES);
Texture3D u_texture3DTable[1024] : register(BINDING_TEXTURE3DTABLE, SPACE_TEXTURES);

#endif