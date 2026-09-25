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

/**
 * Implementation notes
 * ====================
 *
 * Coordinates
 *   cglm is used with its default clip control
 *   (``CGLM_CLIP_CONTROL_RH_NO``): right handed, clip z in [-1, +1]. The
 *   camera looks down -Z and the view matrix is built with ``glm_lookat``.
 *
 * Ray generation (per pixel)
 *   The pixel centre is mapped to NDC (x in [-1, 1], y in [-1, 1] with +y
 *   up, so screen row 0 sits at the top). The NDC point at the near clip
 *   plane is unprojected through the inverse projection matrix (with the
 *   accompanying perspective divide), then the inverse view matrix, giving
 *   a world space point whose direction from the camera eye is the ray
 *   direction.
 *
 * Object setup (per object, once per frame)
 *   The object's effective matrix is (``worldMatrix * object.matrix``).
 *   From it we derive, per frame:
 *
 *   - ``invMatrix``: the inverse, used to bring the world ray into local
 *     (voxel grid) space for the DDA traversal.
 *   - ``worldAABB``: the axis aligned bounding box of the 8 corners of the
 *     local grid [0..dims] transformed by the effective matrix, used for
 *     the rough per-ray test.
 *
 *   The world AABB is also tested against the six view frustum planes to
 *   cull the whole object when it is entirely out of view.
 *
 * Voxel traversal (voxel DDA)
 *   The world ray is transformed into local space by the object inverse
 *   matrix and marched cell by cell through the grid. The first non
 *   transparent voxel hit wins and its colour is returned. A voxel is
 *   transparent (skipped) when its colour equals
 *   :c:macro:`SCREEN_RT_TRANSPARENT_VOXEL`.
 */

#include "embedul.ar/source/core/manager/screen/rt.h"
#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/core/manager/screen.h"

#include <math.h>
#include <string.h>


// Rough per-ray test: does a ray (origin O, direction D) intersect the axis
// aligned box B[2] (min / max)?  Returns true when the ray enters the box
// at a non negative distance t.
static bool RT__rayVsAABB (const vec3 O, const vec3 D, const vec3 B[2])
{
    float tmin = 0.0f;
    float tmax = 1e30f;

    for (int i = 0; i < 3; i++)
    {
        if (D[i] > -1e-10f && D[i] < 1e-10f)
        {
            // Ray is parallel to this slab: it only hits when the origin is
            // already within the slab extent.
            if (O[i] < B[0][i] || O[i] > B[1][i])
            {
                return false;
            }
        }
        else
        {
            const float inv = 1.0f / D[i];
            float t1        = (B[0][i] - O[i]) * inv;
            float t2        = (B[1][i] - O[i]) * inv;

            if (t1 > t2)
            {
                float tmp = t1; t1 = t2; t2 = tmp;
            }

            if (t1 > tmin)
            {
                tmin = t1;
            }
            if (t2 < tmax)
            {
                tmax = t2;
            }

            if (tmin > tmax)
            {
                return false;
            }
        }
    }

    return true;
}


// Voxel DDA: march the local ray (origin OL, direction DL) through the grid
// V and store the colour of the first non transparent voxel hit in
// *HitColor.  Returns true when a voxel was hit, false when the ray does not
// hit any voxel.
//
// The ray origin is the camera eye, which is normally OUTSIDE the grid, so
// the march has to first reach the grid and only then traverse it.  To know
// when to stop, the grid box [0..dims] is intersected with the ray using the
// slab method: the ray is inside that box for the t interval [tEnter, tExit].
// If the line never enters the box the ray misses the grid immediately;
// otherwise the DDA marches only until tExit (the ray leaving the box), which
// bounds the number of steps independently of how far the origin is.
static bool RT__voxelDDA (const struct SCREEN_RT_Voxels *const V,
                          const vec3 OL, const vec3 DL,
                          RGB332_Color *const HitColor)
{
    const uint16_t dx = V->dims[0];
    const uint16_t dy = V->dims[1];
    const uint16_t dz = V->dims[2];

    // Slab intersection of the ray with the grid box [0..dims].
    float tEnter    = 0.0f;
    float tExit     = 1e30f;
    bool  enterable = true;

    for (int i = 0; i < 3 && enterable; i++)
    {
        const float O = OL[i];
        const float D = DL[i];
        const float d = (i == 0) ? (float)dx : (i == 1) ? (float)dy
                                             : (float)dz;

        if (D > -1e-12f && D < 1e-12f)
        {
            // Parallel to this slab: the ray is inside only when the origin is
            // within the slab extent, otherwise the line never enters the box.
            if (O < 0.0f || O > d)
            {
                enterable = false;
            }
        }
        else
        {
            const float inv = 1.0f / D;
            float t1        = (0.0f - O) * inv;
            float t2        = (d - O) * inv;

            if (t1 > t2)
            {
                float tmp = t1; t1 = t2; t2 = tmp;
            }

            if (t1 > tEnter) { tEnter = t1; }
            if (t2 < tExit)  { tExit  = t2; }
        }
    }

    // The line does not cross the grid box at all: the ray misses the grid.
    if (!enterable || tEnter > tExit)
    {
        return false;
    }

    int  ix = (int)floorf (OL[0]);
    int  iy = (int)floorf (OL[1]);
    int  iz = (int)floorf (OL[2]);
    const int stepX = DL[0] > 0.0f ? 1 : -1;
    const int stepY = DL[1] > 0.0f ? 1 : -1;
    const int stepZ = DL[2] > 0.0f ? 1 : -1;

    // Distance (in t units) to the next voxel boundary on each axis.
    const float tDeltaX = fabsf (DL[0]) > 1e-12f ? fabsf (1.0f / DL[0])
                                                 : 1e30f;
    const float tDeltaY = fabsf (DL[1]) > 1e-12f ? fabsf (1.0f / DL[1])
                                                 : 1e30f;
    const float tDeltaZ = fabsf (DL[2]) > 1e-12f ? fabsf (1.0f / DL[2])
                                                 : 1e30f;

    // Distance from the ray origin to the first boundary we will cross.
    float tMaxX = (DL[0] > 0.0f) ? (ix + 1 - OL[0]) / DL[0]
                                 : (OL[0] - ix)      / -DL[0];
    float tMaxY = (DL[1] > 0.0f) ? (iy + 1 - OL[1]) / DL[1]
                                 : (OL[1] - iy)      / -DL[1];
    float tMaxZ = (DL[2] > 0.0f) ? (iz + 1 - OL[2]) / DL[2]
                                 : (OL[2] - iz)      / -DL[2];

    // The march stops the moment a voxel cell would start past tExit (the
    // point where the ray leaves the grid box).  As a backstop against non
    // finite / degenerate inputs, the number of steps is also capped: a ray
    // crosses at most (|DLx|+|Dly|+|DLz|) * t voxel faces over a t interval,
    // so this bound is generous for any realistic tExit.
    const float crossRate =
        fabsf (DL[0]) + fabsf (DL[1]) + fabsf (DL[2]);
    const float maxStepsF =
        (tExit * crossRate) + (float)(dx + dy + dz) + 4.0f;
    const int   maxSteps  =
        (maxStepsF > 1.0e7f) ? (int)1.0e7f : (int)maxStepsF;

    for (int steps = 0; steps <= maxSteps; steps++)
    {
        if ((uint16_t)ix < dx && (uint16_t)iy < dy && (uint16_t)iz < dz)
        {
            const RGB332_Color
                    c = V->data[(size_t)iz * dy * dx
                                 + (size_t)iy * dx
                                 + (size_t)ix];

            if (c != SCREEN_RT_TRANSPARENT_VOXEL)
            {
                *HitColor = c;
                return true;
            }
        }

        // Stop once the cell we are about to advance into starts past the
        // point where the ray leaves the grid box.
        const float tNext =
            (tMaxX < tMaxY) ? (tMaxX < tMaxZ ? tMaxX : tMaxZ)
                            : (tMaxY < tMaxZ ? tMaxY : tMaxZ);
        if (tNext > tExit)
        {
            break;
        }

        // Advance to the next voxel on the nearest boundary.
        if (tMaxX < tMaxY && tMaxX < tMaxZ)
        {
            ix += stepX; tMaxX += tDeltaX;
        }
        else if (tMaxY < tMaxZ)
        {
            iy += stepY; tMaxY += tDeltaY;
        }
        else
        {
            iz += stepZ; tMaxZ += tDeltaZ;
        }
    }

    return false;
}


/**
 * Initializes an empty :c:struct:`SCREEN_RT` instance.
 *
 * :param R: Renderer instance; this condition is asserted.
 */
void SCREEN_RT_Init (struct SCREEN_RT *const R)
{
    BOARD_AssertParams (R);

    memset (R, 0, sizeof (*R));
    glm_mat4_identity (R->camera.proj);
    glm_mat4_identity (R->camera.view);
    glm_mat4_identity (R->worldMatrix);
    R->backgroundColor = SCREEN_RT_TRANSPARENT_VOXEL;
}


/**
 * Clears the renderer to its initial (empty) state.
 *
 * :param R: Renderer instance; this condition is asserted.
 */
void SCREEN_RT_Clear (struct SCREEN_RT *const R)
{
    BOARD_AssertParams (R);

    R->objects     = NULL;
    R->objectCount = 0;
}


/**
 * Adds the scene objects to render.
 *
 * :param R: Renderer instance.
 * :param Objects: Array of scene objects; this condition is asserted. The
 *                 renderer keeps the pointer; the array must outlive every
 *                 subsequent :c:func:`SCREEN_RT_Render` call.
 * :param Count: Number of objects in ``Objects``; this condition is asserted.
 */
void SCREEN_RT_AddObjects (struct SCREEN_RT *const R,
                           struct SCREEN_RT_Object *const Objects,
                           const uint16_t Count)
{
    BOARD_AssertParams (R && Objects && Count);

    R->objects     = Objects;
    R->objectCount = Count;
}


/**
 * Sets the camera (eye, center, up, FOV, aspect, near, far).
 *
 * :param R: Renderer instance; this condition is asserted.
 * :param Eye: World-space camera position (ray origin).
 * :param Center: World-space point the camera looks at.
 * :param Up: World-space up vector.
 * :param Fovy: Vertical field of view, in radians; must be > 0, this
 *              condition is asserted.
 * :param Aspect: Viewport aspect ratio (width / height); must be > 0, this
 *                condition is asserted.
 * :param Near: Near clip distance; must be > 0, this condition is asserted.
 * :param Far: Far clip distance; must be > ``Near``, this condition is
 *             asserted.
 */
void SCREEN_RT_SetCamera (struct SCREEN_RT *const R,
                          const vec3 Eye, const vec3 Center, const vec3 Up,
                          const float Fovy, const float Aspect,
                          const float Near, const float Far)
{
    BOARD_AssertParams (R && Fovy > 0.0f && Aspect > 0.0f && Near > 0.0f &&
                        Far > Near);

    R->camera.near = Near;
    R->camera.far  = Far;

    // cglm 0.9.6 does not const-qualify its source arguments, so stage the
    // const camera vectors through non const locals.
    vec3 eye    = { Eye[0],    Eye[1],    Eye[2]    };
    vec3 center = { Center[0], Center[1], Center[2] };
    vec3 up     = { Up[0],     Up[1],     Up[2]     };

    glm_vec3_copy (eye, R->camera.eye);

    // Projection (perspective) and view (look at) matrices.
    glm_perspective (Fovy, Aspect, Near, Far, R->camera.proj);
    glm_lookat      (eye, center, up, R->camera.view);

    // View frustum planes in world space (order: left, right, bottom, top,
    // near, far).  Used to cull whole objects that are out of view.
    mat4 vp;
    glm_mat4_mul       (R->camera.proj, R->camera.view, vp);
    glm_frustum_planes (vp, R->camera.planes);
}


/**
 * Sets the global scene transform applied on top of every object.
 *
 * :param R: Renderer instance; this condition is asserted.
 * :param WorldMatrix: Global scene transform (effective object matrix =
 *                     ``worldMatrix * object.matrix``).
 */
void SCREEN_RT_SetWorldMatrix (struct SCREEN_RT *const R,
                               const mat4 WorldMatrix)
{
    BOARD_AssertParams (R);

    // cglm 0.9.6 does not const-qualify its source arguments, so stage the
    // const matrix through a non const local.
    mat4 m;
    memcpy (m, WorldMatrix, sizeof (m));

    glm_mat4_copy (m, R->worldMatrix);
}


/**
 * Sets the background color used for pixels that no object hits (sky /
 * miss).
 *
 * :param R: Renderer instance; this condition is asserted.
 * :param Color: The new background color.
 */
void SCREEN_RT_SetBackgroundColor (struct SCREEN_RT *const R,
                                   const RGB332_Color Color)
{
    BOARD_AssertParams (R);

    R->backgroundColor = Color;
}


/**
 * Returns the background color.
 *
 * :param R: Renderer instance; this condition is asserted.
 * :return: The background color.
 */
RGB332_Color SCREEN_RT_GetBackgroundColor (const struct SCREEN_RT *const R)
{
    BOARD_AssertParams (R);

    return R->backgroundColor;
}


/**
 * Renders the scene into the back buffer of the target screen role and
 * flushes it to the display.
 *
 * :param R: Renderer instance holding the objects to render; the objects
 *           must have been set with :c:func:`SCREEN_RT_AddObjects`, this
 *           condition is asserted.
 * :param Role: Screen role to render to; the role must be available, this
 *              condition is asserted.
 * :return: Number of rays that hitted objects.
 */
uint32_t SCREEN_RT_Render (struct SCREEN_RT *const R,
                           const enum SCREEN_Role Role)
{
    BOARD_AssertParams (R && R->objects && R->objectCount > 0);

    const struct SCREEN_Context *const C = SCREEN_GetContext (Role);

    uint32_t        hitCount    = 0;
    const uint16_t  Width       = C->driver->iface->Width;
    const uint16_t  Height      = C->driver->iface->Height;
    uint8_t         *const bb   = SCREEN_Context__backbufferXY (C, 0, 0);

    // Clear the framebuffer to the background colour (sky / miss).
    SCREEN_ClearBack (Role, R->backgroundColor);

    // Inverse view and inverse projection, used to unproject NDC -> world.
    mat4 invView, invProj;
    glm_mat4_inv (R->camera.view, invView);
    glm_mat4_inv (R->camera.proj, invProj);

    // Per object, per frame derived data (see file header).
    const uint16_t  N           = R->objectCount;
    const float     eps         = 1e-3f;

    mat4            eff[N];
    mat4            inv[N];
    vec3            worldAABB[N][2];
    bool            culled[N];

    for (uint16_t o = 0; o < N; o++)
    {
        const struct SCREEN_RT_Object *const Obj = &R->objects[o];

        // Effective matrix = worldMatrix * object.matrix, and its inverse.
        // cglm 0.9.6 does not const-qualify its source arguments, so stage
        // the const object matrix through a non const local.
        mat4 m;
        memcpy (m, Obj->matrix, sizeof (m));

        glm_mat4_mul (R->worldMatrix, m, eff[o]);
        glm_mat4_inv (eff[o], inv[o]);

        // World AABB of the 8 corners of the local grid [0..dims].
        const uint16_t dx = Obj->voxels->dims[0];
        const uint16_t dy = Obj->voxels->dims[1];
        const uint16_t dz = Obj->voxels->dims[2];

        float minX = FLT_MAX,  minY = FLT_MAX,  minZ = FLT_MAX;
        float maxX = -FLT_MAX, maxY = -FLT_MAX, maxZ = -FLT_MAX;

        for (int cx = 0; cx <= 1; cx++)
        for (int cy = 0; cy <= 1; cy++)
        for (int cz = 0; cz <= 1; cz++)
        {
            vec4 v, w;
            v[0] = cx ? (float)dx : 0.0f;
            v[1] = cy ? (float)dy : 0.0f;
            v[2] = cz ? (float)dz : 0.0f;
            v[3] = 1.0f;

            glm_mat4_mulv (eff[o], v, w);
            if (w[0] < minX) { minX = w[0]; }
            if (w[1] < minY) { minY = w[1]; }
            if (w[2] < minZ) { minZ = w[2]; }
            if (w[0] > maxX) { maxX = w[0]; }
            if (w[1] > maxY) { maxY = w[1]; }
            if (w[2] > maxZ) { maxZ = w[2]; }
        }

        // Inflate slightly to absorb voxel grid rounding.
        worldAABB[o][0][0] = minX - eps;
        worldAABB[o][0][1] = minY - eps;
        worldAABB[o][0][2] = minZ - eps;
        worldAABB[o][1][0] = maxX + eps;
        worldAABB[o][1][1] = maxY + eps;
        worldAABB[o][1][2] = maxZ + eps;

        // Cull the whole object when it is entirely outside the frustum.
        culled[o] = ! glm_aabb_frustum (worldAABB[o], R->camera.planes);
    }

    // Per pixel: cast a ray and find the first voxel hit.
    for (uint16_t py = 0; py < Height; py++)
    {
        uint8_t *const row = bb + (size_t)py * Width;

        const float ndcY = 1.0f - (2.0f * (py + 0.5f)) / Height;

        for (uint16_t px = 0; px < Width; px++)
        {
            const float ndcX = (2.0f * (px + 0.5f)) / Width - 1.0f;

            // NDC (near clip plane) -> world (unproject).
            //
            // invProj maps NDC to *homogeneous* view space: the w component is
            // not 1 (for a perspective projection it is -viewZ). The point must
            // therefore be perspective divided (divide xyz by w) BEFORE being
            // brought to world space by invView, otherwise invView scales the
            // camera translation by w and the unprojected point lands far
            // behind the eye.
            vec4 c, p, e;
            c[0] = ndcX; c[1] = ndcY; c[2] = -1.0f; c[3] = 1.0f;
            glm_mat4_mulv (invProj, c, p);
            const float wInv = 1.0f / p[3];
            p[0] *= wInv; p[1] *= wInv; p[2] *= wInv;
            p[3] = 1.0f;
            glm_mat4_mulv (invView, p, e);

            // World ray origin (eye) and direction.
            vec3 dir;
            dir[0] = e[0] - R->camera.eye[0];
            dir[1] = e[1] - R->camera.eye[1];
            dir[2] = e[2] - R->camera.eye[2];
            glm_vec3_normalize (dir);

            // Test every object in order; the first hit wins.
            RGB332_Color color = R->backgroundColor;
            bool         hit   = false;

            for (uint16_t o = 0; o < N && !hit; o++)
            {
                // Skip objects already culled, then do the rough per-ray
                // test against the object world AABB.
                if (culled[o] ||
                    ! RT__rayVsAABB (R->camera.eye, dir, worldAABB[o]))
                {
                    continue;
                }

                // Bring the world ray into the object's local space and
                // march the voxel grid (DDA).
                vec4  wp, wd;
                vec3  OL, DL;
                wp[0] = R->camera.eye[0]; wp[1] = R->camera.eye[1];
                wp[2] = R->camera.eye[2];  wp[3] = 1.0f;
                wd[0] = dir[0]; wd[1] = dir[1]; wd[2] = dir[2]; wd[3] = 0.0f;
                glm_mat4_mulv (inv[o], wp, wp);
                glm_mat4_mulv (inv[o], wd, wd);
                OL[0] = wp[0]; OL[1] = wp[1]; OL[2] = wp[2];
                DL[0] = wd[0]; DL[1] = wd[1]; DL[2] = wd[2];

                if ((hit = RT__voxelDDA(R->objects[o].voxels, OL, DL, &color)))
                {
                    ++ hitCount;
                }
            }

            row[px] = (uint8_t)color;
        }
    }

    return hitCount;
}
