/*
  embedul.ar™ embedded systems framework - http://embedul.ar

  Example: voxel raytracer renderer.

  Renders three procedurally-built voxel objects (a cube, a corner pyramid
  and an octahedron) that spin in place around a fixed camera.  Each object
  is a small RGB332 voxel grid filled once at startup; per frame we only
  recompute its rigid transform (translate * rotate * scale, centered) and
  let SCREEN_RT_Render cast one ray per pixel.

  Press GP1[A] to exit.
*/

#include "embedul.ar/source/core/main.h"
#include "embedul.ar/source/core/manager/screen/rt.h"

#include <math.h>


/* ------------------------------------------------------------------ */
/* Voxel grid size (cells per axis) and per-shape build helpers.       */
/* ------------------------------------------------------------------ */

#define VGRID       10          /* voxels along X / Y / Z              */
#define VCENTER     ((float)VGRID / 2.0f)

#define C_RED       ((RGB332_Color) 0xF8)
#define C_GREEN     ((RGB332_Color) 0x3E)
#define C_BLUE      ((RGB332_Color) 0x03)
#define C_BG        ((RGB332_Color) 0xFF)

/* Flat voxel buffers (row-major, x fastest) matching the renderer layout:
   data[z * dy * dx + y * dx + x].  Filled once, then treated as ROM. */
static RGB332_Color cubeData   [VGRID * VGRID * VGRID];

static const struct SCREEN_RT_Voxels cubeVoxels = { cubeData,   { VGRID, VGRID, VGRID } };

/* The voxels pointer is `const * const`, so it must be fixed at declaration;
   only the matrix is updated at runtime (in initScene / updateObjects). */
static struct SCREEN_RT_Object objects[1] = {
    { &cubeVoxels, { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } } }
};

static struct SCREEN_RT        renderer;


static void buildCube (RGB332_Color *const d)
{
    for (int i = 0; i < VGRID * VGRID * VGRID; i++)
    {
        d[i] = (RANDOM_GetUint32() % 2 == 0)? RANDOM_GetUint32() % 0xFF : SCREEN_RT_TRANSPARENT_VOXEL;
    }
}


/* Rigid transform: place the object at `pos`, scaled by `scale`, spinning by
   `angle` around `axis` about its own grid centre.  Result:
   M = T(pos) * R(angle,axis) * S(scale) * T(-center).  The grid centre
   must map to `pos`, so there is no extra T(center) term. */
static void buildObjectMatrix (mat4 *const out, const vec3 pos,
                               const float angle, const vec3 axis,
                               const float scale)
{
    vec3  negC   = { -VCENTER, -VCENTER, -VCENTER };
    vec3  sv     = { scale,     scale,     scale     };
    vec3  p      = { pos[0],    pos[1],    pos[2]    };
    vec3  a      = { axis[0],   axis[1],   axis[2]   };
    mat4  tP, rot, s, tNc, m;

    // cglm 0.9.6 does not const-qualify its source arguments, so stage the
    // const pos / axis vectors through non const locals (as rt.c does).
    glm_translate_make (tP,  p);
    glm_rotate_make    (rot, angle, a);
    glm_scale_make     (s,   sv);
    glm_translate_make (tNc, negC);

    // Bring the grid centre to the origin, scale, spin about that origin,
    // then place the (origin) grid centre at `pos`.  The grid centre must
    // map exactly to `pos`, hence NO extra T(center) translate here.
    glm_mat4_mul (tP,  rot, m);
    glm_mat4_mul (m,   s,   m);
    glm_mat4_mul (m,   tNc, m);

    glm_mat4_copy (m, *out);
}


/* ------------------------------------------------------------------ */
/* Scene wiring.                                                       */
/* ------------------------------------------------------------------ */

static void initVoxels (void)
{
    buildCube (cubeData);
}


static void initScene (void)
{
    vec3 aY = { 0.0f, 1.0f, 0.0f };
    vec3 pos0 = { -4.0f, 3.0f, 0.0f };

    buildObjectMatrix (&objects[0].matrix, pos0, 0.0f, aY, 0.30f);

    SCREEN_RT_Init                  (&renderer);
    SCREEN_RT_AddObjects            (&renderer, objects, 1);
    SCREEN_RT_SetBackgroundColor    (&renderer, C_BG);
}


/* Update the three object transforms for time t (seconds). */
static void updateObjects (const float t)
{
    vec3 aY   = { 0.0f, 1.0f, 0.0f };
    vec3 pos0 = { 0.0f, 3.4f + 0.4f * sinf (t * 0.9f), 0.0f };

    buildObjectMatrix (&objects[0].matrix, pos0, t * 1.0f, aY, 0.30f);
}


static void updateCamera (void)
{
    vec3 eye    = { 0.0f, 5.0f, 4.0f };
    vec3 center = { 0.0f, 3.0f, 0.0f };
    vec3 up     = { 0.0f, 1.0f, 0.0f };

    const float aspect = (float)SCREEN_Width (SCREEN_Role_Primary) /
                         (float)SCREEN_Height (SCREEN_Role_Primary);

    SCREEN_RT_SetCamera (&renderer, eye, center, up,
                         (float)(70.0 * 3.14159265 / 180.0), /* 70 deg fov */
                         aspect, 0.1f, 100.0f);
}


void EMBEDULAR_Main (void *param)
{
    (void) param;

    initVoxels ();
    initScene  ();

    if (! SCREEN_IsAvailable (SCREEN_Role_Primary))
    {
        LOG (NOBJ, "No primary screen available; nothing to render.");
        return;
    }

    LOG (NOBJ, "Screen Raytracing Example, Press GP1[A] to exit");

    const uint64_t start = TICKS_Now ();

    while (! MIO_GetInputBuffer (INPUT_PROFILE_Group_GP1, IO_Type_Bit,
                                 INPUT_PROFILE_GP1_Bit_A))
    {
        const float t =
            (float)(TICKS_Now () - start) * 0.001f;  /* seconds */

        updateObjects (t);
        updateCamera  ();

        uint32_t raysHitCount = SCREEN_RT_Render (&renderer, SCREEN_Role_Primary);

        SCREEN_FONT_DrawStringLite (SCREEN_Role_Primary, 0, 0, 0b00000011, "RAYTRACER DEMO");
        SCREEN_FONT_DrawParsedStringLite (SCREEN_Role_Primary, 0, 8, 0b00000011, "RAYS HIT: `0", raysHitCount);

        BOARD_Sync ();
    }
}
