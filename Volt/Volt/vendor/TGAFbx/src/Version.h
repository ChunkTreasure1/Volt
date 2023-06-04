#pragma once

#define TGA_FBX_VERSION_MAJOR 2
#define TGA_FBX_VERSION_MINOR 0
#define TGA_FBX_VERSION_BUILD 5

/**
 * TGA FBX Toolkit
 * by Daniel Borgshammar @ TGA
 * ---------------------------
 *
 * Intended as an efficient and lightweight wrapper around the Autodesk FBX SDK
 * tailored specifically for the workflow used at TGA. If you require a feature
 * please let me know and I will see if something can be added.
 *
 * Supported Features:
 * * Multi-Mesh model loading (multiple meshes will be treated as one, single model).
 *
 * * Material Info - Material data saved in the FBX file including texture names.
 *					Keep in mind that the information stored here is arbitrary and
 *					depends on the generating program!
 *
 * * Animation Loading - The package can load FBX animations with animation events.
 *
 * Version 2.0.0
 * -------------
 * * NEW: Rebuilt the importer from the ground up to allow for more efficient memory
 * usage and give the programmer control over the life cycle of the importer.
 * This should allow better flow and lead to less peculiarities while threading.
 * Keep in mind that only so much can be done since the FBX SDK isn't intended
 * to be thread safe and therefore my hands are sort of tied :)
 *
 * * NEW: Restructured the the various objects that exist in the FBX world to have
 * a slightly better layout.
 *
 * * NEW: Support for TGA-style Animation events! These are created by the animators
 * and can be imported as a series of bool values by programmers. Animators define
 * events by making a bone that has the suffix "_evt" or "_event". When this bone
 * has any translation other than Identity it will flag the event as Triggered in the
 * animation loaded by this package.
 *
 * Version 2.0.4
 * -------------
 * * BUG: There is a possible bug in the FBX SDK 2020 which causes issuses when loading
 * vertex colors. It appears to only happen in certain mapping modes and more testing
 * is required to determine under which exact circumstances this happens. I have switched
 * the fix off for now since it appears to happen less frequently and sometimes causes
 * issues with other mapping modes.
 * * FIX: Fixed missing UVs issue due to mapping mode corrections.
 *
 * Version 2.0.5
 * -------------
 * FIX: Fixed a bug where non LOD-grouped meshes did not participate to the whole model bounds.
 * FIX: Material data was not set on the resulting FBXModel.
 * FIX: Error in boxspherebounds calculation for z component.
 *
 * Version 2.0.6
 * -------------
 * NEW: Vertex duplicate check now takes Bone Weights and IDs into account.
 * NEW: You can now tell the importer to not merge duplicate vertices when loading a model.
 */