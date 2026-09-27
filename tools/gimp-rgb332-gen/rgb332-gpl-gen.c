#include <stdio.h>
#include <stdint.h>


int main ()
{
    uint32_t i;

    printf ("GIMP Palette\n");
    printf ("Name: embedul.ar RGB332\n");
    printf ("Columns: 32\n");

    for (i = 0; i < 256; ++i)
    {
        uint8_t r3 = (i >> 5) & 0x7;
        uint8_t g3 = (i >> 2) & 0x7;
        uint8_t b2 =  i       & 0x3;

        uint8_t r = (r3 / 7.0f) * 255.0f;
        uint8_t g = (g3 / 7.0f) * 255.0f;
        uint8_t b = (b2 / 3.0f) * 255.0f;

        printf ("%3u %3u %3u\t0x%02x (%u,%u,%u)\n", r, g, b, i, r3, g3, b2);
    }
    
    return 0;
}
