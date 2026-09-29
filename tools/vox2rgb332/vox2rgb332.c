/*
  vox2rgb332 - convert a MagicaVoxel (.vox) model into a C header for the
               embedul.ar voxel raytracer (source/core/manager/screen/rt.h).

  Input format
  ------------
  The MagicaVoxel "VOX " container (RIFF style, version 150), as documented
  in MagicaVoxel-file-format-vox.txt, next to this source:

      4 bytes   magic 'VOX '
      4 bytes   version (150)
      chunk MAIN
      {
          chunk PACK : optional (4-byte numModels; used for animations)
          chunk SIZE : (x, y, z) as 3 x uint32
          chunk XYZI : numVoxels (uint32), then numVoxels x (x, y, z, colorIndex)
          ... (repeated SIZE/XYZI pairs for multi-model files; first is used)
          chunk RGBA : optional, 256 x (R, G, B, A)
      }

  Every chunk is: 4-byte id, uint32 content size N, uint32 children size M,
  N bytes of content, M bytes of child chunks.

  Notes on the conversion
  -----------------------
  * XYZI is a *sparse* list: only non-empty voxels are stored, each with
    explicit (x, y, z) coordinates. The tool builds the full dense grid
    X*Y*Z, pre-filled with the renderer's transparent sentinel, and places
    every listed voxel at its position.
  * Color index 0 means "empty" and is dropped (the cell keeps the sentinel).
  * If an RGBA chunk is present, palette index c (1..255) maps to RGBA quad
    c-1; otherwise the built-in MagicaVoxel default palette (index c directly)
    is used. 8-bit (R, G, B) is reduced to the RGB332 byte:
        (R >> 5) << 5 | (G >> 5) << 2 | (B >> 6)
  * The output grid is x-fastest, matching the renderer:
        data[z * Y * X + y * X + x]

  Usage:
      vox2rgb332 <input.vox> [output.h]

  If [output.h] is omitted it is derived from the input name (cube.vox ->
  cube.h) and written next to the input file.

  The generated header contains NO #include lines. It defines:

      const RGB332_Color VOXEL_<name>_data[ X*Y*Z ] = { ... };
      const struct SCREEN_RT_Voxels VOXEL_<name>    = { ... };

  where <name> is the input file name with its extension stripped and
  sanitized to a valid C identifier. The consuming translation unit must
  include rgb332.h and rt.h before this header.

  Build (POSIX, C99, no dependencies):
      cc -O2 -Wall vox2rgb332.c -o vox2rgb332
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


/* Renderer's "skip this voxel" color (purple, 111 000 11).
 * Must match SCREEN_RT_TRANSPARENT_VOXEL in rt.h. */
#define VOX_TRANSPARENT_RGB332  0x63

#define PALETTE_SIZE            256u


/* Default palette used when the file has no RGBA chunk.
 * Stored as uint32 with R in the top byte, matching the spec's listing
 * (0x00000000, 0xffffffff, 0xffccffff, ...). */
static const uint32_t default_palette[PALETTE_SIZE] = {
    0x00000000, 0xffffffff, 0xffccffff, 0xff99ffff, 0xff66ffff, 0xff33ffff, 0xff00ffff, 0xffffccff, 0xffccccff, 0xff99ccff, 0xff66ccff, 0xff33ccff, 0xff00ccff, 0xffff99ff, 0xffcc99ff, 0xff9999ff,
    0xff6699ff, 0xff3399ff, 0xff0099ff, 0xffff66ff, 0xffcc66ff, 0xff9966ff, 0xff6666ff, 0xff3366ff, 0xff0066ff, 0xffff33ff, 0xffcc33ff, 0xff9933ff, 0xff6633ff, 0xff3333ff, 0xff0033ff, 0xffff00ff,
    0xffcc00ff, 0xff9900ff, 0xff6600ff, 0xff3300ff, 0xff0000ff, 0xffffffcc, 0xffccffcc, 0xff99ffcc, 0xff66ffcc, 0xff33ffcc, 0xff00ffcc, 0xffffcccc, 0xffcccccc, 0xff99cccc, 0xff66cccc, 0xff33cccc,
    0xff00cccc, 0xffff99cc, 0xffcc99cc, 0xff9999cc, 0xff6699cc, 0xff3399cc, 0xff0099cc, 0xffff66cc, 0xffcc66cc, 0xff9966cc, 0xff6666cc, 0xff3366cc, 0xff0066cc, 0xffff33cc, 0xffcc33cc, 0xff9933cc,
    0xff6633cc, 0xff3333cc, 0xff0033cc, 0xffff00cc, 0xffcc00cc, 0xff9900cc, 0xff6600cc, 0xff3300cc, 0xff0000cc, 0xffffff99, 0xffccff99, 0xff99ff99, 0xff66ff99, 0xff33ff99, 0xff00ff99, 0xffffcc99,
    0xffcccc99, 0xff99cc99, 0xff66cc99, 0xff33cc99, 0xff00cc99, 0xffff9999, 0xffcc9999, 0xff999999, 0xff669999, 0xff339999, 0xff009999, 0xffff6699, 0xffcc6699, 0xff996699, 0xff666699, 0xff336699,
    0xff006699, 0xffff3399, 0xffcc3399, 0xff333399, 0xff003399, 0xffcc0099, 0xff990099, 0xff660099, 0xff330099, 0xff000099, 0xffffff66, 0xffccff66, 0xff99ff66, 0xff66ff66, 0xff33ff66, 0xff00ff66,
    0xffffcc66, 0xffcccc66, 0xff99cc66, 0xff66cc66, 0xff33cc66, 0xff00cc66, 0xffff9966, 0xffcc9966, 0xff999966, 0xff669966, 0xff339966, 0xff009966, 0xffff6666, 0xffcc6666, 0xff996666, 0xff666666,
    0xff336666, 0xff006666, 0xffff3366, 0xffcc3366, 0xff993366, 0xff663366, 0xff333366, 0xff003366, 0xffff0066, 0xffcc0066, 0xff990066, 0xff660066, 0xff330066, 0xff000066, 0xffffff33, 0xffccff33,
    0xff99ff33, 0xff66ff33, 0xff33ff33, 0xff00ff33, 0xffffcc33, 0xffcccc33, 0xff99cc33, 0xff66cc33, 0xff33cc33, 0xff00cc33, 0xffff9933, 0xffcc9933, 0xff999933, 0xff669933, 0xff339933, 0xff009933,
    0xffff6633, 0xffcc6633, 0xff996633, 0xff666633, 0xff336633, 0xff006633, 0xffff3333, 0xffcc3333, 0xff993333, 0xff663333, 0xff333333, 0xff003333, 0xffff0033, 0xffcc0033, 0xff990033, 0xff660033,
    0xff330033, 0xff000033, 0xffffff00, 0xffccff00, 0xff99ff00, 0xff66ff00, 0xff33ff00, 0xff00ff00, 0xffffcc00, 0xffcccc00, 0xff99cc00, 0xff66cc00, 0xff33cc00, 0xff00cc00, 0xffff9900, 0xffcc9900,
    0xff999900, 0xff669900, 0xff339900, 0xff009900, 0xffff6600, 0xffcc6600, 0xff996600, 0xff666600, 0xff336600, 0xff006600, 0xffff3300, 0xffcc3300, 0xff993300, 0xff663300, 0xff333300, 0xff003300,
    0xffff0000, 0xffcc0000, 0xff990000, 0xff660000, 0xff330000, 0xff0000ee, 0xff0000dd, 0xff0000bb, 0xff0000aa, 0xff000088, 0xff000077, 0xff000055, 0xff000044, 0xff000022, 0xff000011, 0xff00ee00,
    0xff00dd00, 0xff00bb00, 0xff00aa00, 0xff008800, 0xff007700, 0xff005500, 0xff004400, 0xff002200, 0xff001100, 0xffee0000, 0xffdd0000, 0xffbb0000, 0xffaa0000, 0xff880000, 0xff770000, 0xff550000,
    0xff440000, 0xff220000, 0xff110000, 0xffeeeeee, 0xffdddddd, 0xffbbbbbb, 0xffaaaaaa, 0xff888888, 0xff777777, 0xff555555, 0xff444444, 0xff222222, 0xff111111
};


static void die (const char *msg)
{
    fprintf (stderr, "vox2rgb332: %s\n", msg);
    exit (1);
}


/* Read a little-endian uint32 from a byte buffer. */
static uint32_t rd32 (const unsigned char *p)
{
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}


/* Reduce 8-bit R/G/B to the RGB332 byte: R[7:5] G[4:2] B[1:0]. */
static uint8_t rgba_to_rgb332 (uint8_t r, uint8_t g, uint8_t b)
{
    return (uint8_t)(((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6));
}


/* Resolve a palette color index to an RGB332 byte.
 * pal is 256 RGB332 bytes (already shifted for the RGBA c-1 convention). */
static uint8_t palette_to_rgb332 (const uint8_t *pal, uint8_t colorIndex)
{
    return pal[colorIndex];
}


/* Derive a valid C identifier from a file path: take the base name, drop the
 * extension, and replace anything that is not [A-Za-z0-9_] with '_'. */
static void make_ident (const char *path, char *out, size_t outsz)
{
    const char *base = strrchr (path, '/');
    base = base ? base + 1 : path;

    const char *dot = strrchr (base, '.');
    size_t len = (dot && dot != base) ? (size_t)(dot - base) : strlen (base);
    if (len >= outsz) len = outsz - 1;
    memcpy (out, base, len);
    out[len] = 0;

    for (char *p = out; *p; ++p)
    {
        int ok = (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
                 (*p >= '0' && *p <= '9') || (*p == '_');
        if (!ok) *p = '_';
    }
    /* An identifier may not start with a digit. */
    if (out[0] >= '0' && out[0] <= '9')
    {
        memmove (out + 1, out, strlen (out) + 1);
        out[0] = 'V';
    }
    if (out[0] == 0) strcpy (out, "vox");
}


/* Copy the input path into out, replacing its extension with ".h". */
static void make_outpath (const char *in, char *out, size_t outsz)
{
    strncpy (out, in, outsz - 1);
    out[outsz - 1] = 0;
    char *dot = strrchr (out, '.');
    if (dot) strcpy (dot, ".h");
    else     strcat (out, ".h");
}


int main (int argc, char **argv)
{
    if (argc < 2 || argc > 3)
    {
        fprintf (stderr, "usage: %s <input.vox> [output.h]\n", argv[0]);
        return 1;
    }

    const char *in = argv[1];
    char outpath[1024];
    if (argc == 3)
    {
        strncpy (outpath, argv[2], sizeof outpath);
        outpath[sizeof outpath - 1] = 0;
    }
    else make_outpath (in, outpath, sizeof outpath);

    /* ---- read the whole file ---- */
    FILE *f = fopen (in, "rb");
    if (!f) { perror ("vox2rgb332: open"); return 1; }
    fseek (f, 0, SEEK_END);
    long flen = ftell (f);
    fseek (f, 0, SEEK_SET);
    if (flen < 20) die ("file too small to be a .vox");

    unsigned char *buf = (unsigned char *)malloc ((size_t)flen);
    if (!buf) die ("out of memory");
    if (fread (buf, 1, (size_t)flen, f) != (size_t)flen) die ("short read");
    fclose (f);

    /* ---- parse the container header ---- */
    if (memcmp (buf, "VOX ", 4) != 0)
        die ("bad magic (expected 'VOX '; not a MagicaVoxel file)");
    const uint32_t version = rd32 (buf + 4);

    /* MAIN chunk: 12-byte header, then M bytes of children. */
    if (flen < 32 || memcmp (buf + 8, "MAIN", 4) != 0)
        die ("missing MAIN chunk");
    const uint32_t mainContent = rd32 (buf + 12);
    const uint32_t mainChildren = rd32 (buf + 16);
    if (mainContent != 0)
        fprintf (stderr,
                 "vox2rgb332: warning: MAIN content size %u (expected 0)\n",
                 mainContent);
    const size_t childrenStart = (size_t)8 + 12 + mainContent;
    if (childrenStart + mainChildren > (size_t)flen)
        die ("MAIN children extend past end of file");

    /* ---- walk MAIN's children ---- */
    uint32_t sx = 0, sy = 0, sz = 0;
    int      haveModel = 0;
    const unsigned char *xyzi = NULL;
    uint32_t numVoxels = 0;
    uint32_t sizeCount = 0, xyziCount = 0;

    uint32_t rgba[PALETTE_SIZE * 4];
    int      haveRgba = 0;
    uint32_t numModelsDeclared = 0;

    size_t pos = childrenStart;
    const size_t end = childrenStart + mainChildren;

    while (pos + 12 <= end)
    {
        const unsigned char *id = buf + pos;
        const uint32_t n = rd32 (buf + pos + 4);   /* content bytes  */
        const uint32_t m = rd32 (buf + pos + 8);   /* children bytes */
        const unsigned char *content = buf + pos + 12;

        if (pos + 12 + n + m > end)
            die ("malformed chunk (extends past MAIN children)");

        if (memcmp (id, "SIZE", 4) == 0)
        {
            if (n < 12) die ("SIZE chunk too small");
            if (!haveModel)
            {
                sx = rd32 (content);
                sy = rd32 (content + 4);
                sz = rd32 (content + 8);
                haveModel = 1;
            }
            sizeCount++;
        }
        else if (memcmp (id, "XYZI", 4) == 0)
        {
            if (n < 4 || (n - 4) % 4 != 0)
                die ("XYZI chunk size inconsistent");
            if (!haveModel) die ("XYZI before its SIZE chunk");
            if (xyzi == NULL)
            {
                numVoxels = rd32 (content);
                if (4 + (size_t)numVoxels * 4 != n)
                    die ("XYZI numVoxels inconsistent with chunk size");
                xyzi = content + 4;
            }
            xyziCount++;
        }
        else if (memcmp (id, "RGBA", 4) == 0)
        {
            if (n != PALETTE_SIZE * 4)
                die ("RGBA chunk must be 1024 bytes");
            memcpy (rgba, content, PALETTE_SIZE * 4);
            haveRgba = 1;
        }
        else if (memcmp (id, "PACK", 4) == 0)
        {
            if (n < 4) die ("PACK chunk too small");
            numModelsDeclared = rd32 (content);
        }
        /* other chunks (NAME, LAYR, ...) are ignored */

        pos += 12 + n + m;
    }

    if (sizeCount > 1 || xyziCount > 1)
        fprintf (stderr,
                 "vox2rgb332: warning: %u model(s) in file, using the first\n",
                 sizeCount);
    if (numModelsDeclared > 1)
        fprintf (stderr,
                 "vox2rgb332: warning: PACK declares %u model(s), using the first\n",
                 numModelsDeclared);
    if (!haveModel) die ("no SIZE/XYZI model found in file");
    if (sx == 0 || sy == 0 || sz == 0) die ("zero dimension in SIZE chunk");
    if (sx > 65535 || sy > 65535 || sz > 65535)
        die ("dimension exceeds uint16_t range");

    const uint32_t total = sx * sy * sz;
    if (total != (uint64_t)sx * (uint64_t)sy * sz)
        die ("grid too large");

    /* ---- build the dense grid, pre-filled with the transparent sentinel ---- */
    unsigned char *grid = (unsigned char *)malloc (total);
    if (!grid) die ("out of memory");
    memset (grid, VOX_TRANSPARENT_RGB332, total);

    /* ---- resolve the palette ---- */
    uint8_t pal[PALETTE_SIZE];
    if (haveRgba)
    {
        /* Index 0 is transparent; index c (1..255) -> RGBA quad c-1.
         * On-disk order is R G B A, i.e. the raw bytes, so read them
         * directly (the rgba[] words are just a memcpy'd view of them). */
        const unsigned char *rq = (const unsigned char *)rgba;
        pal[0] = VOX_TRANSPARENT_RGB332;
        for (uint32_t c = 1; c < PALETTE_SIZE; c++)
        {
            const size_t q = (size_t)(c - 1) * 4;
            const uint8_t r = rq[q + 0];
            const uint8_t g = rq[q + 1];
            const uint8_t b = rq[q + 2];
            /* alpha byte ignored: index 0 already covers "empty" */
            pal[c] = rgba_to_rgb332 (r, g, b);
        }
    }
    else
    {
        fprintf (stderr,
                 "vox2rgb332: note: no RGBA chunk, using the default palette\n");
        for (uint32_t c = 0; c < PALETTE_SIZE; c++)
        {
            const uint32_t quad = default_palette[c];
            const uint8_t r = (uint8_t)(quad >> 24);
            const uint8_t g = (uint8_t)(quad >> 16);
            const uint8_t b = (uint8_t)(quad >> 8);
            pal[c] = (c == 0)
                    ? VOX_TRANSPARENT_RGB332
                    : rgba_to_rgb332 (r, g, b);
        }
    }

    /* ---- place each XYZI voxel into the grid ---- */
    uint32_t placed = 0, skippedTransparent = 0, skippedRange = 0;
    for (uint32_t i = 0; i < numVoxels; i++)
    {
        const uint8_t x = xyzi[i * 4 + 0];
        const uint8_t y = xyzi[i * 4 + 1];
        const uint8_t z = xyzi[i * 4 + 2];
        const uint8_t c = xyzi[i * 4 + 3];

        if (x >= sx || y >= sy || z >= sz)
        {
            skippedRange++;
            continue;
        }
        if (c == 0)
        {
            skippedTransparent++;
            continue;
        }
        grid[(size_t)z * sy * sx + (size_t)y * sx + x] =
            palette_to_rgb332 (pal, c);
        placed++;
    }

    if (skippedRange > 0)
        fprintf (stderr,
                 "vox2rgb332: warning: %u voxel(s) out of range, skipped\n",
                 skippedRange);
    if (skippedTransparent > 0)
        fprintf (stderr,
                 "vox2rgb332: note: %u voxel(s) had color index 0 (empty), skipped\n",
                 skippedTransparent);

    /* ---- emit the header ---- */
    char ident[256];
    make_ident (in, ident, sizeof ident);

    FILE *o = fopen (outpath, "wb");
    if (!o) { perror ("vox2rgb332: open output"); return 1; }

    fprintf (o, "/* Auto-generated by vox2rgb332 from %s (VOX v%u). Do not edit.\n",
             in, version);
    fprintf (o, " * Grid %ux%ux%u = %u cells, %u solid, x fastest: idx = z*(%u) + y*(%u) + x.\n",
             sx, sy, sz, total, placed, sy * sx, sx);
    fprintf (o, " * This header has no #includes; include rgb332.h and rt.h first.\n");
    fprintf (o, " */\n\n");

    fprintf (o, "const RGB332_Color VOXEL_%s_data[%u] = {\n", ident, total);
    for (uint32_t z = 0; z < sz; z++)
    {
        fprintf (o, "    /* z = %u */\n", z);
        for (uint32_t y = 0; y < sy; y++)
        {
            fprintf (o, "    ");
            for (uint32_t x = 0; x < sx; x++)
            {
                fprintf (o, "0x%02X", grid[(size_t)z * sy * sx
                                           + (size_t)y * sx + x]);
                if (x + 1 < sx)
                    fprintf (o, ", ");
            }
            if (y + 1 < sy || z + 1 < sz)
                fprintf (o, ",");
            fprintf (o, "\n");
        }
    }
    fprintf (o, "};\n\n");

    fprintf (o, "const struct SCREEN_RT_Voxels VOXEL_%s = {\n", ident);
    fprintf (o, "    .data = VOXEL_%s_data,\n", ident);
    fprintf (o, "    .dX = %u,\n", sx);
    fprintf (o, "    .dY = %u,\n", sy);
    fprintf (o, "    .dZ = %u\n", sz);
    fprintf (o, "};\n");

    fclose (o);
    printf ("vox2rgb332: wrote %s  (VOXEL_%s, %ux%ux%u, %u/%u solid voxels)\n",
            outpath, ident, sx, sy, sz, placed, total);

    free (buf);
    free (grid);
    return 0;
}
