#pragma once

#include "../Core/Common.h"
#include <windows.h>

struct RoomData;

namespace VisualFX {

struct BiomePalette {
    COLORREF floorTop;
    COLORREF floorBottom;
    COLORREF grid;
    COLORREF wall;
    COLORREF wallEdge;
    COLORREF accent;
    COLORREF accentSoft;
    COLORREF panel;
};

COLORREF MixColor(COLORREF a, COLORREF b, float t);
COLORREF ScaleColor(COLORREF color, float factor);

BiomePalette GetBiomePalette(BiomeType biome);
const char* GetBiomeName(BiomeType biome);
const char* GetRoomTypeName(RoomType type);

void FillVerticalGradient(int left, int top, int right, int bottom,
                          COLORREF topColor, COLORREF bottomColor,
                          int step = 2);
void DrawFilledRoundRect(int left, int top, int right, int bottom,
                         int radius, COLORREF color);
void DrawRoundFrame(int left, int top, int right, int bottom,
                    int radius, COLORREF fill, COLORREF border,
                    int borderWidth = 2);
void DrawSoftPanel(const AABB& rect, COLORREF fill, COLORREF border,
                   int radius = 14, bool shadow = true);
void DrawButtonSurface(const AABB& rect, COLORREF fill, COLORREF border,
                       bool hovered, bool enabled, int radius = 16);
void DrawProgressBar(int left, int top, int width, int height, float ratio,
                     COLORREF fill, COLORREF back, COLORREF border);
void DrawPixelPanel(int left, int top, int right, int bottom,
                    COLORREF fill, COLORREF border, bool shadow = true);
void DrawPixelButton(const AABB& rect, COLORREF fill, COLORREF border,
                     bool hovered, bool enabled);
void DrawPixelBar(int left, int top, int width, int height, float ratio,
                  COLORREF fill, COLORREF back, COLORREF border);
void DrawPixelRect(int left, int top, int right, int bottom,
                   COLORREF fill, COLORREF border, int borderWidth = 4);
void DrawPixelShadow(int cx, int cy, int halfW, int halfH);
void DrawPixelDiamond(int cx, int cy, int radius,
                      COLORREF fill, COLORREF border);
void DrawPixelProjectile(int cx, int cy, float dirX, float dirY,
                         int length, int thickness,
                         COLORREF fill, COLORREF border);
void DrawGlowCircle(int cx, int cy, int radius, COLORREF color,
                    int layers = 4);
void DrawDiamond(int cx, int cy, int radius, COLORREF fill,
                 COLORREF border);
void DrawTextShadow(const char* text, int x, int y, COLORREF color,
                    int fontSize, const char* fontName = "Microsoft YaHei UI");

void DrawDimOverlay(COLORREF color, int density = 4);
void DrawRoomBackdrop(BiomeType biome, const RoomData* room,
                      float elapsedTime);

} // namespace VisualFX
