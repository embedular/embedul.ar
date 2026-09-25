/*
  embedul.ar™ embedded systems framework - http://embedul.ar

  [SCREEN MANAGER] voxel raytracer renderer.

  Copyright 2018-2022 Santiago Germino
  <sgermino@embedul.ar> https://www.linkedin.com/in/royconejo

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#pragma once

#include "embedul.ar/source/core/manager/screen/role.h"
#include "embedul.ar/source/core/misc/rgb332.h"
#include "cglm.h"

#include <stdbool.h>
#include <stdint.h>


/**
 * Description
 * ===========
 *
 * Software voxel raytracer renderer for the screen manager. For each target
 * pixel, a ray is generated from the camera and tested against every object
 * in the scene, in order.
 *
 * The test is performed in two stages:
 *
 * Rough AABB test
 *   The ray is tested against the object's world axis-aligned bounding box
 *   (a :c:type:`vec3[2]` min/max in world space). Misses are rejected at a
 *   near-zero cost. This test is deliberately not mathematically exact: the
 *   world AABB is inflated by a small margin to absorb rounding of the voxel
 *   grid, and the traversal below performs the exact containment test.
 *
 * Voxel collision
 *   The ray is brought into the object's local space (by the inverse of the
 *   object world matrix) and marched cell-by-cell through the voxel grid
 *   (voxel DDA). The first non-transparent voxel hit wins; its color is
 *   stored in the frame buffer.
 *
 * A voxel is *transparent* (skipped during traversal) when its
 * :c:type:`RGB332_Color` equals :c:macro:`SCREEN_RT_TRANSPARENT_VOXEL`
 * (purple). Transparent voxels therefore act as "holes" the ray passes
 * straight through.
 *
 * All math is done in single precision float via cglm (see
 * ``source/3rd_party/cglm-0.9.6``). The scene is right-handed; the camera
 * looks down -Z.
 *
 *
 * API guide
 * =========
 *
 * Renderer
 * --------
 *
 * Initializes an empty renderer.
 *
 * | :c:func:`SCREEN_RT_Init`
 *
 * Clears the renderer to its initial (empty) state.
 *
 * | :c:func:`SCREEN_RT_Clear`
 *
 * Adds the scene objects to render.
 *
 * | :c:func:`SCREEN_RT_AddObjects`
 *
 * Scene setup
 * -----------
 *
 * Sets the camera (eye, center, up, FOV, aspect, near, far).
 *
 * | :c:func:`SCREEN_RT_SetCamera`
 *
 * Sets the global scene transform applied on top of every object.
 *
 * | :c:func:`SCREEN_RT_SetWorldMatrix`
 *
 * Sets and gets the background color used for pixels that no object hits.
 *
 * | :c:func:`SCREEN_RT_SetBackgroundColor`
 * | :c:func:`SCREEN_RT_GetBackgroundColor`
 *
 * Render
 * ------
 *
 * Renders the scene into the back buffer of the target screen role.
 *
 * | :c:func:`SCREEN_RT_Render`
 *
 *
 * Design and development status
 * =============================
 *
 * Feature-complete.
 *
 *
 * Changelog
 * =========
 *
 * ======= ========== =================== ======================================
 * Version Date*      Author              Comment
 * ======= ========== =================== ======================================
 * 1.0.0   2026.9.24  sgermino            Initial release.
 * ======= ========== =================== ======================================
 *
 * \* Date format is Year.Month.Day.
 *
 *
 * API reference
 * =============
 */


/**
 * The RGB332 color of a transparent voxel (purple): ``1110 0011``. Voxels
 * holding this color are skipped during traversal.
 */
#define SCREEN_RT_TRANSPARENT_VOXEL     0x63


/**
 * The voxel grid descriptor, typically stored in Flash (ROM).
 */
struct SCREEN_RT_Voxels
{
    /** Pointer to the row-major RGB332 color buffer
     * (``dims.z * dims.y * dims.x``) in Flash. ``dims.x`` is the fastest
     * (inner) axis.
     */
    const RGB332_Color          * const data;
    /** Number of voxels along X / Y / Z. A voxel ``(i,j,k)`` occupies the
     * local cell ``[i, i+1] x [j, j+1] x [k, k+1]``.
     */
    const uint16_t              dims[3];
};


/**
 * A single voxel object in the scene.
 *
 * The renderer derives, each frame, every object's inverse matrix (for the
 * local-space voxel traversal) and its world-space axis-aligned bounding box
 * (for the rough per-ray test) directly from ``matrix``. These are not
 * stored here.
 */
struct SCREEN_RT_Object
{
    /** Pointer to the object's :c:struct:`SCREEN_RT_Voxels` descriptor
     * (in Flash). Read-only to the renderer.
     */
    const struct SCREEN_RT_Voxels
                                * const voxels;
    /** Rigid transform (position / rotation / size) that maps the object's
     * local voxel grid into world space. This is the "position / rotation /
     * size matrix" for the object.
     */
    mat4                        matrix;
};


/**
 * The camera.
 */
struct SCREEN_RT_Camera
{
    /** World-space camera position (ray origin). */
    vec3                        eye;
    /** Perspective projection matrix (``glm_perspective``). */
    mat4                        proj;
    /** View matrix (``glm_lookat``) that maps world -> eye space. */
    mat4                        view;
    /** Near clip distance. */
    float                       near;
    /** Far clip distance. */
    float                       far;
    /** The six view frustum planes in world space
     * (left, right, bottom, top, near, far), derived from ``proj * view``.
     * Used to cull whole objects that are fully outside the view.
     */
    vec4                        planes[6];
};


/**
 * The raytracer renderer. The user should treat this as an opaque
 * structure. No member should be directly accessed or modified.
 */
struct SCREEN_RT
{
    /** The active camera. */
    struct SCREEN_RT_Camera     camera;
    /** A global scene transform applied on top of every object (effective
     * object matrix = ``worldMatrix * object.matrix``).
     */
    mat4                        worldMatrix;
    /** The RGB332 color used for pixels that no object hits (sky /
     * background). Set with :c:func:`SCREEN_RT_SetBackgroundColor`.
     */
    RGB332_Color                backgroundColor;
    /** The array of scene objects, provided by the user. */
    struct SCREEN_RT_Object     * objects;
    /** Number of objects in ``objects``. */
    uint16_t                    objectCount;
};


void            SCREEN_RT_Init                (struct SCREEN_RT *const R);
void            SCREEN_RT_Clear               (struct SCREEN_RT *const R);
void            SCREEN_RT_AddObjects          (struct SCREEN_RT *const R,
                                               struct SCREEN_RT_Object
                                               *const Objects,
                                               const uint16_t Count);
void            SCREEN_RT_SetCamera           (struct SCREEN_RT *const R,
                                               const vec3 Eye,
                                               const vec3 Center,
                                               const vec3 Up,
                                               const float Fovy,
                                               const float Aspect,
                                               const float Near,
                                               const float Far);
void            SCREEN_RT_SetWorldMatrix      (struct SCREEN_RT *const R,
                                               const mat4 WorldMatrix);
void            SCREEN_RT_SetBackgroundColor  (struct SCREEN_RT *const R,
                                               const RGB332_Color Color);
RGB332_Color    SCREEN_RT_GetBackgroundColor  (const struct SCREEN_RT *const R);
uint32_t        SCREEN_RT_Render              (struct SCREEN_RT *const R,
                                               const enum SCREEN_Role Role);
