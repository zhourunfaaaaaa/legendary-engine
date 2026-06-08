// ============================================================
// VisualEffects.cpp - shared EasyX drawing helpers
// ============================================================

#include "../include/Graphics/VisualEffects.h"
#include "../include/System/MapGenerator.h"

#include <graphics.h>
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {

int ClampByte(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return v;
}

float Clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

void DrawHorizontalGlowLine(int x1, int y, int x2, COLORREF color) {
    setlinecolor(VisualFX::ScaleColor(color, 0.38f));
    line(x1, y - 1, x2, y - 1);
    line(x1, y + 1, x2, y + 1);
    setlinecolor(color);
    line(x1, y, x2, y);
}

void DrawFloorGrid(COLORREF gridColor) {
    setlinestyle(PS_SOLID, 1);
    setlinecolor(gridColor);
    for (int x = TILE_SIZE; x < ROOM_WIDTH; x += TILE_SIZE) {
        line(x, 12, x, ROOM_HEIGHT - 12);
    }
    for (int y = TILE_SIZE; y < ROOM_HEIGHT; y += TILE_SIZE) {
        line(12, y, ROOM_WIDTH - 12, y);
    }
}

void DrawForestDetails(float elapsedTime) {
    (void)elapsedTime;
    for (int i = 0; i < 18; ++i) {
        int x = 48 + (i * 157) % (ROOM_WIDTH - 96);
        int y = 44 + (i * 91) % (ROOM_HEIGHT - 88);
        int r = 8 + (i * 7) % 18;
        COLORREF moss = (i % 2 == 0) ? RGB(43, 95, 45) : RGB(31, 76, 39);
        setfillcolor(moss);
        solidcircle(x, y, r);
        setfillcolor(VisualFX::ScaleColor(moss, 1.25f));
        solidcircle(x - r / 3, y - r / 3, r / 3);
    }

    setlinestyle(PS_SOLID, 1);
    for (int i = 0; i < 10; ++i) {
        int x = 64 + (i * 211) % (ROOM_WIDTH - 128);
        int y = 70 + (i * 73) % (ROOM_HEIGHT - 140);
        setlinecolor(RGB(68, 112, 57));
        line(x - 16, y + 5, x + 18, y - 7);
        line(x - 6, y - 8, x + 12, y + 9);
    }
}

void DrawIceDetails(float elapsedTime) {
    float shimmer = 0.5f + 0.5f * std::sin(elapsedTime * 1.8f);
    COLORREF crack = VisualFX::MixColor(RGB(78, 130, 160), RGB(162, 230, 255), shimmer * 0.45f);

    setlinestyle(PS_SOLID, 1);
    setlinecolor(crack);
    for (int i = 0; i < 11; ++i) {
        int x = 70 + (i * 173) % (ROOM_WIDTH - 140);
        int y = 62 + (i * 103) % (ROOM_HEIGHT - 124);
        line(x - 20, y, x - 4, y + 8);
        line(x - 4, y + 8, x + 18, y - 6);
        line(x + 2, y + 3, x + 8, y + 18);
    }

    for (int i = 0; i < 26; ++i) {
        int x = 26 + (i * 97) % (ROOM_WIDTH - 52);
        int y = 30 + (i * 67) % (ROOM_HEIGHT - 60);
        setfillcolor(i % 3 == 0 ? RGB(185, 225, 244) : RGB(122, 181, 214));
        solidcircle(x, y, 1 + (i % 2));
    }
}

void DrawVolcanoDetails(float elapsedTime) {
    float pulse = 0.5f + 0.5f * std::sin(elapsedTime * 2.4f);
    COLORREF lava = VisualFX::MixColor(RGB(170, 38, 18), RGB(255, 146, 34), pulse);

    for (int i = 0; i < 9; ++i) {
        int x = 68 + (i * 199) % (ROOM_WIDTH - 136);
        int y = 86 + (i * 127) % (ROOM_HEIGHT - 172);
        DrawHorizontalGlowLine(x - 28, y, x + 24, lava);
        setlinecolor(VisualFX::ScaleColor(lava, 0.55f));
        line(x - 5, y, x + 12, y + 16);
        line(x + 14, y, x + 29, y - 13);
    }

    for (int i = 0; i < 14; ++i) {
        int x = 38 + (i * 151) % (ROOM_WIDTH - 76);
        int y = 34 + (i * 61) % (ROOM_HEIGHT - 68);
        int r = 3 + (i % 4);
        VisualFX::DrawGlowCircle(x, y, r, lava, 2);
        setfillcolor(RGB(255, 190, 70));
        solidcircle(x, y, r / 2 + 1);
    }
}

void DrawRoomFrame(const VisualFX::BiomePalette& p, const RoomData* room) {
    (void)room;
    const int wall = (int)ROOM_WALL_MARGIN;
    setfillcolor(p.wall);
    solidrectangle(0, 0, ROOM_WIDTH - 1, wall);
    solidrectangle(0, ROOM_HEIGHT - wall, ROOM_WIDTH - 1, ROOM_HEIGHT - 1);
    solidrectangle(0, 0, wall, ROOM_HEIGHT - 1);
    solidrectangle(ROOM_WIDTH - wall, 0, ROOM_WIDTH - 1, ROOM_HEIGHT - 1);

    const int brick = 56;
    for (int x = 4; x < ROOM_WIDTH - 4; x += brick) {
        COLORREF topColor = ((x / brick) % 2) ? VisualFX::ScaleColor(p.wall, 1.08f) : p.wall;
        setfillcolor(topColor);
        solidrectangle(x, 6, (std::min)(x + brick - 8, ROOM_WIDTH - 7), wall - 10);
        solidrectangle(x, ROOM_HEIGHT - wall + 10,
                       (std::min)(x + brick - 8, ROOM_WIDTH - 7), ROOM_HEIGHT - 7);
    }
    for (int y = 6; y < ROOM_HEIGHT - 6; y += brick) {
        COLORREF sideColor = ((y / brick) % 2) ? VisualFX::ScaleColor(p.wall, 0.92f) : VisualFX::ScaleColor(p.wall, 1.04f);
        setfillcolor(sideColor);
        solidrectangle(6, y, wall - 10, (std::min)(y + brick - 8, ROOM_HEIGHT - 7));
        solidrectangle(ROOM_WIDTH - wall + 10, y,
                       ROOM_WIDTH - 7, (std::min)(y + brick - 8, ROOM_HEIGHT - 7));
    }

    setlinecolor(RGB(5, 7, 10));
    setlinestyle(PS_SOLID, 5);
    rectangle(wall, wall, ROOM_WIDTH - wall, ROOM_HEIGHT - wall);
    setlinecolor(VisualFX::ScaleColor(p.wallEdge, 1.12f));
    setlinestyle(PS_SOLID, 2);
    rectangle(wall + 5, wall + 5, ROOM_WIDTH - wall - 5, ROOM_HEIGHT - wall - 5);
    setlinestyle(PS_SOLID, 1);
}

} // namespace

namespace VisualFX {

COLORREF MixColor(COLORREF a, COLORREF b, float t) {
    t = Clamp01(t);
    int r = ClampByte((int)(GetRValue(a) + (GetRValue(b) - GetRValue(a)) * t));
    int g = ClampByte((int)(GetGValue(a) + (GetGValue(b) - GetGValue(a)) * t));
    int bl = ClampByte((int)(GetBValue(a) + (GetBValue(b) - GetBValue(a)) * t));
    return RGB(r, g, bl);
}

COLORREF ScaleColor(COLORREF color, float factor) {
    return RGB(ClampByte((int)(GetRValue(color) * factor)),
               ClampByte((int)(GetGValue(color) * factor)),
               ClampByte((int)(GetBValue(color) * factor)));
}

BiomePalette GetBiomePalette(BiomeType biome) {
    switch (biome) {
        case BiomeType::ICE_DUNGEON:
            return { RGB(48, 73, 90), RGB(36, 58, 76), RGB(61, 91, 108),
                     RGB(29, 45, 58), RGB(102, 174, 203), RGB(117, 214, 244),
                     RGB(70, 132, 164), RGB(29, 42, 54) };
        case BiomeType::VOLCANO:
            return { RGB(78, 43, 36), RGB(52, 32, 34), RGB(93, 56, 47),
                     RGB(45, 28, 29), RGB(207, 83, 44), RGB(255, 131, 45),
                     RGB(154, 55, 35), RGB(44, 31, 35) };
        case BiomeType::FOREST:
        default:
            return { RGB(56, 83, 55), RGB(42, 65, 50), RGB(66, 95, 64),
                     RGB(34, 51, 40), RGB(126, 176, 78), RGB(164, 215, 93),
                     RGB(80, 134, 75), RGB(32, 46, 38) };
    }
}

const char* GetBiomeName(BiomeType biome) {
    switch (biome) {
        case BiomeType::FOREST: return "暮光森林";
        case BiomeType::ICE_DUNGEON: return "寒霜地牢";
        case BiomeType::VOLCANO: return "熔岩火山";
        default: return "未知领域";
    }
}

const char* GetRoomTypeName(RoomType type) {
    switch (type) {
        case RoomType::START: return "起始房";
        case RoomType::NORMAL: return "战斗房";
        case RoomType::ELITE: return "精英房";
        case RoomType::REWARD: return "奖励房";
        case RoomType::SHOP: return "商店";
        case RoomType::BOSS: return "首领房";
        case RoomType::EXIT: return "出口房";
        default: return "房间";
    }
}

void FillVerticalGradient(int left, int top, int right, int bottom,
                          COLORREF topColor, COLORREF bottomColor,
                          int step) {
    if (step < 1) step = 1;
    int height = bottom - top;
    if (height <= 0) return;

    for (int y = top; y <= bottom; y += step) {
        float t = (float)(y - top) / (float)height;
        COLORREF c = MixColor(topColor, bottomColor, t);
        setfillcolor(c);
        solidrectangle(left, y, right, (std::min)(y + step - 1, bottom));
    }
}

void DrawFilledRoundRect(int left, int top, int right, int bottom,
                         int radius, COLORREF color) {
    if (right < left) { int tmp = right; right = left; left = tmp; }
    if (bottom < top) { int tmp = bottom; bottom = top; top = tmp; }
    int w = right - left;
    int h = bottom - top;
    int r = radius;
    if (r < 0) r = 0;
    if (r * 2 > w) r = w / 2;
    if (r * 2 > h) r = h / 2;

    setfillcolor(color);
    if (r <= 1) {
        solidrectangle(left, top, right, bottom);
        return;
    }

    solidrectangle(left + r, top, right - r, bottom);
    solidrectangle(left, top + r, right, bottom - r);
    solidcircle(left + r, top + r, r);
    solidcircle(right - r, top + r, r);
    solidcircle(left + r, bottom - r, r);
    solidcircle(right - r, bottom - r, r);
}

void DrawRoundFrame(int left, int top, int right, int bottom,
                    int radius, COLORREF fill, COLORREF border,
                    int borderWidth) {
    if (borderWidth < 1) borderWidth = 1;
    DrawFilledRoundRect(left, top, right, bottom, radius, border);
    DrawFilledRoundRect(left + borderWidth, top + borderWidth,
                        right - borderWidth, bottom - borderWidth,
                        (std::max)(0, radius - borderWidth), fill);
}

void DrawSoftPanel(const AABB& rect, COLORREF fill, COLORREF border,
                   int radius, bool shadow) {
    (void)radius;
    int l = (int)rect.Left();
    int t = (int)rect.Top();
    int r = (int)rect.Right();
    int b = (int)rect.Bottom();
    DrawPixelPanel(l, t, r, b, fill, border, shadow);
}

void DrawButtonSurface(const AABB& rect, COLORREF fill, COLORREF border,
                       bool hovered, bool enabled, int radius) {
    (void)radius;
    DrawPixelButton(rect, fill, border, hovered, enabled);
}

void DrawPixelPanel(int left, int top, int right, int bottom,
                    COLORREF fill, COLORREF border, bool shadow) {
    if (shadow) {
        setfillcolor(RGB(8, 10, 13));
        solidrectangle(left + 4, top + 4, right + 4, bottom + 4);
    }

    setfillcolor(RGB(12, 14, 18));
    solidrectangle(left, top, right, bottom);
    setfillcolor(border);
    solidrectangle(left + 3, top + 3, right - 3, bottom - 3);
    setfillcolor(fill);
    solidrectangle(left + 6, top + 6, right - 6, bottom - 6);

    setlinecolor(ScaleColor(fill, 1.28f));
    line(left + 8, top + 8, right - 8, top + 8);
    line(left + 8, top + 8, left + 8, bottom - 8);
    setlinecolor(ScaleColor(fill, 0.55f));
    line(left + 8, bottom - 8, right - 8, bottom - 8);
    line(right - 8, top + 8, right - 8, bottom - 8);
}

void DrawPixelButton(const AABB& rect, COLORREF fill, COLORREF border,
                     bool hovered, bool enabled) {
    COLORREF base = enabled ? fill : RGB(44, 48, 55);
    COLORREF edge = enabled ? border : RGB(70, 73, 78);
    if (hovered && enabled) {
        base = MixColor(base, RGB(255, 238, 160), 0.18f);
        edge = RGB(244, 205, 82);
    }
    DrawPixelPanel((int)rect.Left(), (int)rect.Top(), (int)rect.Right(), (int)rect.Bottom(),
                   base, edge, true);
    if (hovered && enabled) {
        setfillcolor(RGB(255, 226, 95));
        solidrectangle((int)rect.Left() + 12, (int)rect.Top() + 12,
                       (int)rect.Left() + 20, (int)rect.Bottom() - 12);
    }
}

void DrawProgressBar(int left, int top, int width, int height, float ratio,
                     COLORREF fill, COLORREF back, COLORREF border) {
    DrawPixelBar(left, top, width, height, ratio, fill, back, border);
}

void DrawPixelBar(int left, int top, int width, int height, float ratio,
                  COLORREF fill, COLORREF back, COLORREF border) {
    ratio = Clamp01(ratio);
    int right = left + width;
    int bottom = top + height;
    setfillcolor(RGB(9, 10, 12));
    solidrectangle(left, top, right, bottom);
    setfillcolor(border);
    solidrectangle(left + 2, top + 2, right - 2, bottom - 2);
    setfillcolor(back);
    solidrectangle(left + 5, top + 5, right - 5, bottom - 5);

    int innerPad = 6;
    int fillW = (int)((width - innerPad * 2) * ratio);
    if (fillW > 0) {
        int fl = left + innerPad;
        int ft = top + innerPad;
        int fr = fl + fillW;
        int fb = bottom - innerPad;
        setfillcolor(fill);
        solidrectangle(fl, ft, fr, fb);
        setlinecolor(ScaleColor(fill, 1.25f));
        line(fl, ft, fr, ft);
    }
}

void DrawPixelRect(int left, int top, int right, int bottom,
                   COLORREF fill, COLORREF border, int borderWidth) {
    if (borderWidth < 1) borderWidth = 1;
    setfillcolor(border);
    solidrectangle(left, top, right, bottom);
    setfillcolor(fill);
    solidrectangle(left + borderWidth, top + borderWidth,
                   right - borderWidth, bottom - borderWidth);
    setlinecolor(ScaleColor(fill, 1.28f));
    line(left + borderWidth + 1, top + borderWidth + 1,
         right - borderWidth - 1, top + borderWidth + 1);
    line(left + borderWidth + 1, top + borderWidth + 1,
         left + borderWidth + 1, bottom - borderWidth - 1);
}

void DrawPixelShadow(int cx, int cy, int halfW, int halfH) {
    setfillcolor(RGB(8, 9, 11));
    solidrectangle(cx - halfW, cy - halfH / 2, cx + halfW, cy + halfH / 2);
}

void DrawPixelDiamond(int cx, int cy, int radius,
                      COLORREF fill, COLORREF border) {
    for (int y = -radius; y <= radius; ++y) {
        int half = radius - std::abs(y);
        setlinecolor(border);
        line(cx - half - 2, cy + y, cx + half + 2, cy + y);
    }
    for (int y = -radius + 4; y <= radius - 4; ++y) {
        int half = radius - 4 - std::abs(y);
        if (half < 0) continue;
        setlinecolor(fill);
        line(cx - half, cy + y, cx + half, cy + y);
    }
    setlinecolor(ScaleColor(fill, 1.35f));
    line(cx - radius / 2, cy - radius / 2, cx + radius / 3, cy - radius / 2);
}

void DrawPixelProjectile(int cx, int cy, float dirX, float dirY,
                         int length, int thickness,
                         COLORREF fill, COLORREF border) {
    float len = std::sqrt(dirX * dirX + dirY * dirY);
    if (len < 0.01f) { dirX = 1.0f; dirY = 0.0f; len = 1.0f; }
    dirX /= len;
    dirY /= len;
    int tailX = cx - (int)(dirX * length);
    int tailY = cy - (int)(dirY * length);
    int headX = cx + (int)(dirX * (length / 3));
    int headY = cy + (int)(dirY * (length / 3));

    setlinestyle(PS_SOLID, thickness + 4);
    setlinecolor(border);
    line(tailX, tailY, headX, headY);
    setlinestyle(PS_SOLID, thickness);
    setlinecolor(fill);
    line(tailX, tailY, headX, headY);
    setlinestyle(PS_SOLID, 1);

    DrawPixelDiamond(headX, headY, thickness + 4, fill, border);
}

void DrawGlowCircle(int cx, int cy, int radius, COLORREF color,
                    int layers) {
    if (layers < 1) layers = 1;
    for (int i = layers; i >= 1; --i) {
        float factor = 0.16f + 0.12f * (float)i;
        COLORREF c = ScaleColor(color, factor);
        setfillcolor(c);
        solidcircle(cx, cy, radius + i * 4);
    }
}

void DrawDiamond(int cx, int cy, int radius, COLORREF fill,
                 COLORREF border) {
    for (int y = -radius - 1; y <= radius + 1; ++y) {
        int half = radius + 1 - std::abs(y);
        setlinecolor(border);
        line(cx - half, cy + y, cx + half, cy + y);
    }
    for (int y = -radius + 1; y <= radius - 1; ++y) {
        int half = radius - std::abs(y);
        setlinecolor(fill);
        line(cx - half, cy + y, cx + half, cy + y);
    }
    setlinecolor(ScaleColor(fill, 1.45f));
    line(cx - radius / 3, cy - radius / 3, cx + radius / 2, cy - radius / 2);
}

void DrawTextShadow(const char* text, int x, int y, COLORREF color,
                    int fontSize, const char* fontName) {
    setbkmode(TRANSPARENT);
    settextstyle(fontSize, 0, fontName);
    settextcolor(RGB(2, 4, 8));
    outtextxy(x + 2, y + 2, text);
    settextcolor(color);
    outtextxy(x, y, text);
}

void DrawDimOverlay(COLORREF color, int density) {
    if (density < 2) density = 2;
    setfillcolor(ScaleColor(color, 0.7f));
    for (int y = 0; y < WINDOW_HEIGHT; y += density) {
        solidrectangle(0, y, WINDOW_WIDTH - 1, y + density / 2);
    }
    setlinecolor(ScaleColor(color, 1.35f));
    for (int x = 0; x < WINDOW_WIDTH; x += density * 3) {
        line(x, 0, x, WINDOW_HEIGHT - 1);
    }
}

void DrawRoomBackdrop(BiomeType biome, const RoomData* room,
                      float elapsedTime) {
    (void)elapsedTime;
    BiomePalette p = GetBiomePalette(biome);

    setfillcolor(p.floorBottom);
    solidrectangle(0, 0, ROOM_WIDTH - 1, ROOM_HEIGHT - 1);

    const int tile = 56;
    const int roomPad = (int)ROOM_WALL_MARGIN;
    for (int y = roomPad; y < ROOM_HEIGHT - roomPad; y += tile) {
        for (int x = roomPad; x < ROOM_WIDTH - roomPad; x += tile) {
            int salt = (x / tile * 17 + y / tile * 31) % 5;
            COLORREF c = salt == 0 ? ScaleColor(p.floorTop, 1.08f) :
                         salt == 1 ? p.floorTop :
                         salt == 2 ? MixColor(p.floorTop, p.floorBottom, 0.35f) :
                         ScaleColor(p.floorBottom, 0.95f);
            setfillcolor(c);
            solidrectangle(x, y, x + tile - 3, y + tile - 3);
            setlinecolor(ScaleColor(c, 0.72f));
            rectangle(x, y, x + tile - 3, y + tile - 3);

            if (salt == 0 || salt == 3) {
                setfillcolor(ScaleColor(c, 0.82f));
                solidrectangle(x + 9, y + 9, x + 17, y + 13);
                solidrectangle(x + 31, y + 34, x + 42, y + 38);
            }
        }
    }

    setlinecolor(p.grid);
    for (int x = roomPad; x < ROOM_WIDTH - roomPad; x += tile) line(x, roomPad, x, ROOM_HEIGHT - roomPad);
    for (int y = roomPad; y < ROOM_HEIGHT - roomPad; y += tile) line(roomPad, y, ROOM_WIDTH - roomPad, y);

    if (biome == BiomeType::FOREST) {
        for (int i = 0; i < 18; ++i) {
            int x = 54 + (i * 149) % (ROOM_WIDTH - 108);
            int y = 54 + (i * 97) % (ROOM_HEIGHT - 108);
            setfillcolor(i % 2 ? RGB(70, 110, 68) : RGB(86, 128, 70));
            solidrectangle(x - 4, y - 4, x + 8, y + 8);
        }
    } else if (biome == BiomeType::ICE_DUNGEON) {
        setlinecolor(RGB(122, 184, 210));
        for (int i = 0; i < 14; ++i) {
            int x = 60 + (i * 157) % (ROOM_WIDTH - 120);
            int y = 70 + (i * 89) % (ROOM_HEIGHT - 140);
            line(x - 18, y, x + 16, y + 10);
            line(x + 2, y + 5, x + 12, y - 12);
        }
    } else {
        for (int i = 0; i < 12; ++i) {
            int x = 70 + (i * 181) % (ROOM_WIDTH - 140);
            int y = 78 + (i * 113) % (ROOM_HEIGHT - 156);
            setfillcolor(RGB(180, 61, 36));
            solidrectangle(x - 14, y - 3, x + 14, y + 3);
            setfillcolor(RGB(255, 143, 46));
            solidrectangle(x - 8, y - 2, x + 8, y + 2);
        }
    }

    DrawRoomFrame(p, room);
}

} // namespace VisualFX
