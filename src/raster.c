/* Raster core imported from AgentArmstrong. See status/menu/raster-import.json.
 * WIP host rendering; game logic is not implemented here. */
#include "psx.h"
#include "raster.h"
#include "types.h"
#include <string.h>
/* Save State 1 context at 800D6CB8: width +8=512, height +12=240;
 * its DR_ENV also ends at (511,239). */
#define FW 512
#define FH 240
#define VRAM_W 1024
#define VRAM_H 512
typedef struct { sint32 x,y,u,v; } RV;
typedef struct {
 sint32 minx,maxx,miny,maxy;
 sint64 e0,e1,e2,e0dx,e1dx,e2dx,e0dy,e1dy,e2dy;
 sint64 u,v,r,g,b,udx,vdx,rdx,gdx,bdx,udy,vdy,rdy,gdy,bdy;
} RasterScan;
static uint32 fb[FW*FH];
static uint16 vram[VRAM_W*VRAM_H],active_tpage;
static uint32 texture_window;
static sint32 raster_clip_x0=0,raster_clip_x1=FW,raster_clip_y0=0,raster_clip_y1=FH;
static sint32 raster_offset_x,raster_offset_y;
static sint32 read_s16_le_at(const uint8 *p,sint32 offset)
{ uint16 v=rrj_u16(p+offset); return v<32768?v:(sint32)v-65536; }
static void pxc(sint32 x, sint32 y, uint32 color)
{
    if (x >= raster_clip_x0 && x < raster_clip_x1 && y >= raster_clip_y0 && y < raster_clip_y1 && (uint32)x < FW && (uint32)y < FH)
        fb[y * FW + x] = color;
}

static uint32 rgb555(uint16 c)
{
    uint32 r = (c & 31) << 3, g = ((c >> 5) & 31) << 3, b = ((c >> 10) & 31) << 3;
    return (r | (r >> 5)) << 16 | (g | (g >> 5)) << 8 | (b | (b >> 5));
}

static uint16 texel_indexed(sint32 u, sint32 v, uint16 tpage, uint16 clut, sint32 *transparent)
{
    sint32 tp = (tpage >> 7) & 3, tx = (tpage & 15) * 64, ty = ((tpage >> 4) & 1) * 256, cx = (clut & 63) * 16, cy = (clut >> 6) & 0x1ff, index;
    uint16 w, color;
    u = ((u & ~(((sint32)texture_window & 31) << 3)) | ((((sint32)texture_window >> 10) & 31) & ((sint32)texture_window & 31)) << 3) & 255;
    v = ((v & ~((((sint32)texture_window >> 5) & 31) << 3)) | ((((sint32)texture_window >> 15) & 31) & (((sint32)texture_window >> 5) & 31)) << 3) & 255;
    *transparent = 1;
    if ((uint32)(ty + v) >= VRAM_H || (uint32)cy >= VRAM_H)
        return 0;
    if (tp == 0)
    {
        w = vram[(ty + v) * VRAM_W + tx + (u >> 2)];
        index = (w >> ((u & 3) * 4)) & 15;
    }
    else if (tp == 1)
    {
        w = vram[(ty + v) * VRAM_W + tx + (u >> 1)];
        index = (w >> ((u & 1) * 8)) & 255;
    }
    else
    {
        if ((uint32)(tx + u) >= VRAM_W)
            return 0;
        color = vram[(ty + v) * VRAM_W + tx + u];
        *transparent = color == 0;
        return color;
    }
    if ((uint32)(cx + index) >= VRAM_W)
        return 0;
    color = vram[cy * VRAM_W + cx + index];
    *transparent = color == 0;
    return color;
}

static uint32 modulate(uint16 c, sint32 r, sint32 g, sint32 b, sint32 raw)
{
    uint32 q = rgb555(c), tr = (q >> 16) & 255, tg = (q >> 8) & 255, tb = q & 255;
    if (raw)
        return q;
    tr = tr * r / 128;
    tg = tg * g / 128;
    tb = tb * b / 128;
    if (tr > 255)
        tr = 255;
    if (tg > 255)
        tg = 255;
    if (tb > 255)
        tb = 255;
    return tr << 16 | tg << 8 | tb;
}

static uint32 semi_blend(uint32 back, uint32 front, sint32 abr)
{
    sint32 br = (back >> 19) & 31, bg = (back >> 11) & 31, bb = (back >> 3) & 31, fr = (front >> 19) & 31, fg = (front >> 11) & 31, fbv = (front >> 3) & 31, r, g, b;
    if (abr == 0)
    {
        r = (br + fr) >> 1;
        g = (bg + fg) >> 1;
        b = (bb + fbv) >> 1;
    }
    else if (abr == 1)
    {
        r = br + fr;
        g = bg + fg;
        b = bb + fbv;
    }
    else if (abr == 2)
    {
        r = br - fr;
        g = bg - fg;
        b = bb - fbv;
    }
    else
    {
        r = br + (fr >> 2);
        g = bg + (fg >> 2);
        b = bb + (fbv >> 2);
    }
    if (r < 0)
        r = 0;
    if (g < 0)
        g = 0;
    if (b < 0)
        b = 0;
    if (r > 31)
        r = 31;
    if (g > 31)
        g = 31;
    if (b > 31)
        b = 31;
    r = (r << 3) | (r >> 2);
    g = (g << 3) | (g >> 2);
    b = (b << 3) | (b >> 2);
    return (uint32)(r << 16 | g << 8 | b);
}

static uint32 packet_rgb(uint8 *p, sint32 o)
{
    return (uint32)p[o] << 16 | (uint32)p[o + 1] << 8 | p[o + 2];
}

static void raster_bounds(RV a, RV b, RV c, sint32 *minx, sint32 *maxx, sint32 *miny, sint32 *maxy)
{
    *minx = a.x;
    *maxx = a.x;
    *miny = a.y;
    *maxy = a.y;
    if (b.x < *minx)
        *minx = b.x;
    if (c.x < *minx)
        *minx = c.x;
    if (b.x > *maxx)
        *maxx = b.x;
    if (c.x > *maxx)
        *maxx = c.x;
    if (b.y < *miny)
        *miny = b.y;
    if (c.y < *miny)
        *miny = c.y;
    if (b.y > *maxy)
        *maxy = b.y;
    if (c.y > *maxy)
        *maxy = c.y;
    if (*minx < raster_clip_x0)
        *minx = raster_clip_x0;
    if (*maxx >= raster_clip_x1)
        *maxx = raster_clip_x1 - 1;
    if (*miny < raster_clip_y0)
        *miny = raster_clip_y0;
    if (*maxy >= raster_clip_y1)
        *maxy = raster_clip_y1 - 1;
    if (*minx < 0)
        *minx = 0;
    if (*miny < 0)
        *miny = 0;
    if (*maxx >= FW)
        *maxx = FW - 1;
    if (*maxy >= FH)
        *maxy = FH - 1;
}

static sint64 edge2(const RV *a, const RV *b, sint32 sx, sint32 sy)
{
    return (sint64)(sx - a->x * 2) * (b->y - a->y) - (sint64)(sy - a->y * 2) * (b->x - a->x);
}

static sint64 fixed_value(sint32 aa, sint32 bb, sint32 cc, sint64 e0, sint64 e1, sint64 e2, sint64 denominator)
{
    return ((sint64)aa * e0 + (sint64)bb * e1 + (sint64)cc * e2) * 65536 / denominator;
}

static sint32 setup_scan(RV a, RV b, RV c, uint32 ca, uint32 cb, uint32 cc, RasterScan *s)
{
    sint64 area, denominator, sign;
    sint32 sx, sy;
    raster_bounds(a, b, c, &s->minx, &s->maxx, &s->miny, &s->maxy);
    if (s->minx > s->maxx || s->miny > s->maxy)
        return 0;
    area = (sint64)(c.x - a.x) * (b.y - a.y) - (sint64)(c.y - a.y) * (b.x - a.x);
    if (area == 0)
        return 0;
    sign = area < 0 ? -1 : 1;
    denominator = area * 2 * sign;
    sx = s->minx * 2 + 1;
    sy = s->miny * 2 + 1;
    s->e0 = edge2(&b, &c, sx, sy) * sign;
    s->e1 = edge2(&c, &a, sx, sy) * sign;
    s->e2 = edge2(&a, &b, sx, sy) * sign;
    s->e0dx = (sint64)2 * (c.y - b.y) * sign;
    s->e1dx = (sint64)2 * (a.y - c.y) * sign;
    s->e2dx = (sint64)2 * (b.y - a.y) * sign;
    s->e0dy = (sint64)-2 * (c.x - b.x) * sign;
    s->e1dy = (sint64)-2 * (a.x - c.x) * sign;
    s->e2dy = (sint64)-2 * (b.x - a.x) * sign;
    s->u = fixed_value(a.u, b.u, c.u, s->e0, s->e1, s->e2, denominator);
    s->v = fixed_value(a.v, b.v, c.v, s->e0, s->e1, s->e2, denominator);
    s->r = fixed_value((ca >> 16) & 255, (cb >> 16) & 255, (cc >> 16) & 255, s->e0, s->e1, s->e2, denominator);
    s->g = fixed_value((ca >> 8) & 255, (cb >> 8) & 255, (cc >> 8) & 255, s->e0, s->e1, s->e2, denominator);
    s->b = fixed_value(ca & 255, cb & 255, cc & 255, s->e0, s->e1, s->e2, denominator);
    s->udx = fixed_value(a.u, b.u, c.u, s->e0dx, s->e1dx, s->e2dx, denominator);
    s->vdx = fixed_value(a.v, b.v, c.v, s->e0dx, s->e1dx, s->e2dx, denominator);
    s->rdx = fixed_value((ca >> 16) & 255, (cb >> 16) & 255, (cc >> 16) & 255, s->e0dx, s->e1dx, s->e2dx, denominator);
    s->gdx = fixed_value((ca >> 8) & 255, (cb >> 8) & 255, (cc >> 8) & 255, s->e0dx, s->e1dx, s->e2dx, denominator);
    s->bdx = fixed_value(ca & 255, cb & 255, cc & 255, s->e0dx, s->e1dx, s->e2dx, denominator);
    s->udy = fixed_value(a.u, b.u, c.u, s->e0dy, s->e1dy, s->e2dy, denominator);
    s->vdy = fixed_value(a.v, b.v, c.v, s->e0dy, s->e1dy, s->e2dy, denominator);
    s->rdy = fixed_value((ca >> 16) & 255, (cb >> 16) & 255, (cc >> 16) & 255, s->e0dy, s->e1dy, s->e2dy, denominator);
    s->gdy = fixed_value((ca >> 8) & 255, (cb >> 8) & 255, (cc >> 8) & 255, s->e0dy, s->e1dy, s->e2dy, denominator);
    s->bdy = fixed_value(ca & 255, cb & 255, cc & 255, s->e0dy, s->e1dy, s->e2dy, denominator);
    return 1;
}

static void textured_triangle(RV a, RV b, RV c, uint16 tp, uint16 cl, uint32 ca, uint32 cb, uint32 cc, sint32 raw, sint32 semi)
{
    RasterScan s;
    sint32 x, y;
    if (!setup_scan(a, b, c, ca, cb, cc, &s))
        return;
    for (y = s.miny; y <= s.maxy; y++)
    {
        sint64 e0 = s.e0, e1 = s.e1, e2 = s.e2, u = s.u, v = s.v, rr = s.r, gg = s.g, bb = s.b;
        sint32 entered = 0;
        for (x = s.minx; x <= s.maxx; x++)
        {
            if (e0 >= 0 && e1 >= 0 && e2 >= 0)
            {
                sint32 transparent, r, g, bl;
                uint16 t;
                uint32 color;
                entered = 1;
                t = texel_indexed((sint32)(u >> 16), (sint32)(v >> 16), tp, cl, &transparent);
                if (!transparent)
                {
                    r = raw ? 128 : (sint32)(rr >> 16);
                    g = raw ? 128 : (sint32)(gg >> 16);
                    bl = raw ? 128 : (sint32)(bb >> 16);
                    color = modulate(t, r, g, bl, 0);
                    if (semi && (t & 0x8000))
                        color = semi_blend(fb[y * FW + x], color, (tp >> 5) & 3);
                    fb[y * FW + x] = color;
                }
            }
            else if (entered)
                break;
            e0 += s.e0dx;
            e1 += s.e1dx;
            e2 += s.e2dx;
            u += s.udx;
            v += s.vdx;
            rr += s.rdx;
            gg += s.gdx;
            bb += s.bdx;
        }
        s.e0 += s.e0dy;
        s.e1 += s.e1dy;
        s.e2 += s.e2dy;
        s.u += s.udy;
        s.v += s.vdy;
        s.r += s.rdy;
        s.g += s.gdy;
        s.b += s.bdy;
    }
}

static void colored_triangle(RV a, RV b, RV c, uint32 ca, uint32 cb, uint32 cc, sint32 semi)
{
    RasterScan s;
    sint32 x, y;
    if (!setup_scan(a, b, c, ca, cb, cc, &s))
        return;
    for (y = s.miny; y <= s.maxy; y++)
    {
        sint64 e0 = s.e0, e1 = s.e1, e2 = s.e2, rr = s.r, gg = s.g, bb = s.b;
        sint32 entered = 0;
        for (x = s.minx; x <= s.maxx; x++)
        {
            if (e0 >= 0 && e1 >= 0 && e2 >= 0)
            {
                uint32 color;
                sint32 r = (sint32)(rr >> 16), g = (sint32)(gg >> 16), bl = (sint32)(bb >> 16);
                entered = 1;
                color = (uint32)r << 16 | (uint32)g << 8 | (uint32)bl;
                if (semi)
                    color = semi_blend(fb[y * FW + x], color, (active_tpage >> 5) & 3);
                fb[y * FW + x] = color;
            }
            else if (entered)
                break;
            e0 += s.e0dx;
            e1 += s.e1dx;
            e2 += s.e2dx;
            rr += s.rdx;
            gg += s.gdx;
            bb += s.bdx;
        }
        s.e0 += s.e0dy;
        s.e1 += s.e1dy;
        s.e2 += s.e2dy;
        s.r += s.rdy;
        s.g += s.gdy;
        s.b += s.bdy;
    }
}

static RV rv(uint8 *p, sint32 xy, sint32 uv)
{
    RV v;
    v.x = read_s16_le_at(p, xy) + raster_offset_x;
    v.y = read_s16_le_at(p, xy + 2) + raster_offset_y;
    v.u = p[uv];
    v.v = p[uv + 1];
    return v;
}

static void draw_prim(void *raw)
{
    uint8 *p = (uint8 *)raw;
    sint32 code = p[7] & 0xfc;
    uint32 *w = (uint32 *)p;
    uint32 ca, cb, cc, cd;
    if ((w[1] & 0xff000000) == 0xe1000000)
    {
        active_tpage = (uint16)(w[1] & 0x9ff);
        if ((w[2] & 0xff000000) == 0xe2000000)
            texture_window = w[2] & 0xfffff;
        return;
    }
    if ((w[1] & 0xff000000) == 0xe2000000)
    {
        texture_window = w[1] & 0xfffff;
        return;
    }
    ca = packet_rgb(p, 4);
    if (code == 0x20)
    {
        RV a = rv(p, 8, 0), b = rv(p, 12, 0), c = rv(p, 16, 0);
        colored_triangle(a, b, c, ca, ca, ca, (p[7] & 2) != 0);
        return;
    }
    if (code == 0x28)
    {
        RV a = rv(p, 8, 0), b = rv(p, 12, 0), c = rv(p, 16, 0), d = rv(p, 20, 0);
        colored_triangle(a, b, c, ca, ca, ca, (p[7] & 2) != 0);
        colored_triangle(b, c, d, ca, ca, ca, (p[7] & 2) != 0);
        return;
    }
    if (code == 0x30)
    {
        RV a = rv(p, 8, 0), b = rv(p, 16, 0), c = rv(p, 24, 0);
        cb = packet_rgb(p, 12);
        cc = packet_rgb(p, 20);
        colored_triangle(a, b, c, ca, cb, cc, (p[7] & 2) != 0);
        return;
    }
    if (code == 0x38)
    {
        RV a = rv(p, 8, 0), b = rv(p, 16, 0), c = rv(p, 24, 0), d = rv(p, 32, 0);
        cb = packet_rgb(p, 12);
        cc = packet_rgb(p, 20);
        cd = packet_rgb(p, 28);
        colored_triangle(a, b, c, ca, cb, cc, (p[7] & 2) != 0);
        colored_triangle(b, c, d, cb, cc, cd, (p[7] & 2) != 0);
        return;
    }
    if (code == 0x24)
    {
        RV a = rv(p, 8, 12), b = rv(p, 16, 20), c = rv(p, 24, 28);
        textured_triangle(a, b, c, *(uint16 *)(p + 22), *(uint16 *)(p + 14), ca, ca, ca, p[7] & 1, (p[7] & 2) != 0);
        return;
    }
    if (code == 0x2c)
    {
        RV a = rv(p, 8, 12), b = rv(p, 16, 20), c = rv(p, 24, 28), d = rv(p, 32, 36);
        uint16 tp = *(uint16 *)(p + 22), cl = *(uint16 *)(p + 14);
        textured_triangle(a, b, c, tp, cl, ca, ca, ca, p[7] & 1, (p[7] & 2) != 0);
        textured_triangle(b, c, d, tp, cl, ca, ca, ca, p[7] & 1, (p[7] & 2) != 0);
        return;
    }
    if (code == 0x34)
    {
        RV a = rv(p, 8, 12), b = rv(p, 20, 24), c = rv(p, 32, 36);
        uint16 tp = *(uint16 *)(p + 26), cl = *(uint16 *)(p + 14);
        cb = packet_rgb(p, 16);
        cc = packet_rgb(p, 28);
        textured_triangle(a, b, c, tp, cl, ca, cb, cc, p[7] & 1, (p[7] & 2) != 0);
        return;
    }
    if (code == 0x3c)
    {
        RV a = rv(p, 8, 12), b = rv(p, 20, 24), c = rv(p, 32, 36), d = rv(p, 44, 48);
        uint16 tp = *(uint16 *)(p + 26), cl = *(uint16 *)(p + 14);
        cb = packet_rgb(p, 16);
        cc = packet_rgb(p, 28);
        cd = packet_rgb(p, 40);
        textured_triangle(a, b, c, tp, cl, ca, cb, cc, p[7] & 1, (p[7] & 2) != 0);
        textured_triangle(b, c, d, tp, cl, cb, cc, cd, p[7] & 1, (p[7] & 2) != 0);
        return;
    }
    if (code == 0x64 || code == 0x74 || code == 0x7c)
    {
        sint32 x, y, x0 = read_s16_le_at(p, 8) + raster_offset_x, y0 = read_s16_le_at(p, 10) + raster_offset_y, ww = code == 0x64 ? read_s16_le_at(p, 16) : (code == 0x74 ? 8 : 16), hh = code == 0x64 ? read_s16_le_at(p, 18) : (code == 0x74 ? 8 : 16);
        for (y = 0; y < hh; y++)
            for (x = 0; x < ww; x++)
            {
                sint32 transparent;
                uint16 t = texel_indexed(p[12] + x, p[13] + y, active_tpage, *(uint16 *)(p + 14), &transparent);
                if (!transparent && x0 + x >= raster_clip_x0 && x0 + x < raster_clip_x1 && y0 + y >= raster_clip_y0 && y0 + y < raster_clip_y1 && (uint32)(x0 + x) < FW && (uint32)(y0 + y) < FH)
                {
                    uint32 color = modulate(t, p[4], p[5], p[6], p[7] & 1);
                    if ((p[7] & 2) && (t & 0x8000))
                        color = semi_blend(fb[(y0 + y) * FW + x0 + x], color, (active_tpage >> 5) & 3);
                    fb[(y0 + y) * FW + x0 + x] = color;
                }
            }
        return;
    }
    if (code == 0x60 || code == 0x68 || code == 0x70 || code == 0x78)
    {
        sint32 x, y, x0 = read_s16_le_at(p, 8) + raster_offset_x, y0 = read_s16_le_at(p, 10) + raster_offset_y, ww = code == 0x60 ? read_s16_le_at(p, 12) : (code == 0x68 ? 1 : code == 0x70 ? 8 : 16), hh = code == 0x60 ? read_s16_le_at(p, 14) : (code == 0x68 ? 1 : code == 0x70 ? 8 : 16);
        for (y = 0; y < hh; y++)
            for (x = 0; x < ww; x++)
            {
                uint32 color = ca;
                /* RRJ host adaptation: clip before reading the destination
                 * for semi-blending. AA clipped only the later write. */
                if (x0+x < raster_clip_x0 || x0+x >= raster_clip_x1 ||
                    y0+y < raster_clip_y0 || y0+y >= raster_clip_y1 ||
                    (uint32)(x0+x) >= FW || (uint32)(y0+y) >= FH) continue;
                if (p[7] & 2)
                    color = semi_blend(fb[(y0 + y) * FW + x0 + x], color, (active_tpage >> 5) & 3);
                pxc(x0 + x, y0 + y, color);
            }
        return;
    }
    pxc(read_s16_le_at(p, 8) + raster_offset_x, read_s16_le_at(p, 10) + raster_offset_y, 0xff00ff);
}

void rrj_raster_clear(void) { memset(fb,0,sizeof fb); }
void rrj_raster_packet(void *packet) { draw_prim(packet); }
const uint32_t *rrj_raster_pixels(void) { return fb; }
uint16_t *rrj_raster_vram(void) { return vram; }
int rrj_raster_width(void) { return FW; }
int rrj_raster_height(void) { return FH; }

/* PSX draw coordinates are in VRAM space. The host framebuffer is relative
 * to the selected display origin; texture fetches still address all VRAM. */
void rrj_raster_environment(uint32_t word, int origin_x, int origin_y)
{
    switch (word >> 24) {
    case 0xE1: active_tpage = (uint16)(word & 0x9ff); break;
    case 0xE2: texture_window = word & 0xfffff; break;
    case 0xE3:
        raster_clip_x0 = (int)(word & 1023) - origin_x;
        raster_clip_y0 = (int)((word >> 10) & 511) - origin_y;
        break;
    case 0xE4:
        raster_clip_x1 = (int)(word & 1023) + 1 - origin_x;
        raster_clip_y1 = (int)((word >> 10) & 511) + 1 - origin_y;
        break;
    case 0xE5:
        raster_offset_x = (int)(word & 2047);
        raster_offset_y = (int)((word >> 11) & 2047);
        if (raster_offset_x & 1024) raster_offset_x -= 2048;
        if (raster_offset_y & 1024) raster_offset_y -= 2048;
        raster_offset_x -= origin_x; raster_offset_y -= origin_y;
        break;
    }
}
