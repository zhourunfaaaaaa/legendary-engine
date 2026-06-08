// ============================================================
// UIManager.cpp - UI 管理器（HUD / 菜单 / 商店 / 天赋选择）
// ============================================================

#include "../include/UI/UIManager.h"
#include "../include/Entity/Player.h"
#include "../include/Entity/Boss.h"
#include "../include/Weapon/Weapon.h"
#include "../include/Weapon/WeaponTypes.h"
#include "../include/System/MapGenerator.h"
#include "../include/System/BuffManager.h"
#include "../include/Graphics/VisualEffects.h"
#include <graphics.h>
#include <windows.h>
#include <algorithm>
#include <cstdio>
#include <cstring>

// Windows.h (via graphics.h) defines DrawText as DrawTextA/W macro.
// Undefine it so our UIManager method names don't get corrupted.
#ifdef DrawText
#undef DrawText
#endif

namespace {

COLORREF RoomColor(RoomType type) {
    switch (type) {
        case RoomType::START:  return RGB(118, 178, 255);
        case RoomType::NORMAL: return RGB(105, 180, 116);
        case RoomType::ELITE:  return RGB(236, 170, 75);
        case RoomType::REWARD: return RGB(245, 215, 89);
        case RoomType::SHOP:   return RGB(112, 215, 186);
        case RoomType::BOSS:   return RGB(238, 84, 84);
        case RoomType::EXIT:   return RGB(180, 152, 255);
        default:               return RGB(70, 78, 92);
    }
}

const char* SkillLabelFor(Player* player) {
    if (!player) return "技能";
    switch (player->GetProfession()) {
        case Profession::KNIGHT: return "火力";
        case Profession::ROGUE:  return "翻滚";
        case Profession::MAGE:   return "闪电";
        default: return "技能";
    }
}

void DrawMenuBackdrop() {
    setfillcolor(RGB(32, 39, 48));
    solidrectangle(0, 0, WINDOW_WIDTH - 1, WINDOW_HEIGHT - 1);

    const int tile = 56;
    for (int y = 0; y < WINDOW_HEIGHT; y += tile) {
        for (int x = 0; x < WINDOW_WIDTH; x += tile) {
            int salt = (x / tile * 11 + y / tile * 19) % 4;
            COLORREF c = salt == 0 ? RGB(42, 50, 61) :
                         salt == 1 ? RGB(36, 44, 55) :
                         salt == 2 ? RGB(48, 55, 64) : RGB(31, 38, 47);
            setfillcolor(c);
            solidrectangle(x, y, x + tile - 2, y + tile - 2);
            setlinecolor(RGB(22, 27, 35));
            rectangle(x, y, x + tile - 2, y + tile - 2);
        }
    }

    // 像素地牢门厅
    int cx = WINDOW_WIDTH / 2;
    setfillcolor(RGB(18, 20, 26));
    solidrectangle(cx - 170, 104, cx + 170, 250);
    setfillcolor(RGB(74, 58, 48));
    solidrectangle(cx - 144, 132, cx + 144, 250);
    setfillcolor(RGB(34, 24, 25));
    solidrectangle(cx - 72, 160, cx + 72, 250);
    setfillcolor(RGB(224, 173, 72));
    solidrectangle(cx - 8, 196, cx + 8, 212);
}

void DrawClassEmblem(int cx, int cy, Profession prof, bool hovered) {
    COLORREF mainColor = RGB(82, 136, 255);
    COLORREF edgeColor = RGB(175, 204, 255);
    if (prof == Profession::ROGUE) {
        mainColor = RGB(74, 205, 128);
        edgeColor = RGB(180, 248, 196);
    } else if (prof == Profession::MAGE) {
        mainColor = RGB(186, 101, 255);
        edgeColor = RGB(232, 202, 255);
    }

    if (hovered) {
        setfillcolor(VisualFX::ScaleColor(mainColor, 0.35f));
        solidrectangle(cx - 50, cy - 50, cx + 50, cy + 50);
    }
    VisualFX::DrawDiamond(cx, cy, 40, VisualFX::ScaleColor(mainColor, 0.82f), edgeColor);

    setlinecolor(RGB(255, 245, 190));
    setlinestyle(PS_SOLID, 3);
    if (prof == Profession::KNIGHT) {
        line(cx, cy - 24, cx, cy + 22);
        line(cx - 12, cy - 8, cx + 12, cy - 8);
        line(cx - 8, cy + 22, cx + 8, cy + 22);
    } else if (prof == Profession::ROGUE) {
        line(cx - 16, cy + 18, cx + 18, cy - 18);
        line(cx - 4, cy + 18, cx + 18, cy - 4);
    } else {
        circle(cx, cy, 15);
        line(cx, cy - 24, cx, cy + 24);
        line(cx - 20, cy, cx + 20, cy);
    }
    setlinestyle(PS_SOLID, 1);
}

void DrawSmallIcon(int cx, int cy, COLORREF color, const char* label) {
    VisualFX::DrawPixelPanel(cx - 13, cy - 13, cx + 13, cy + 13,
                             color, RGB(12, 14, 18), false);
    VisualFX::DrawTextShadow(label, cx + 18, cy - 8, RGB(226, 234, 242), 14);
}

COLORREF WeaponAccent(WeaponType type) {
    int tier = GetWeaponTier(type);
    switch (type) {
        case WeaponType::ASSAULT_RIFLE:    return RGB(255, 190, 72);
        case WeaponType::SHOTGUN:          return RGB(255, 218, 86);
        case WeaponType::SNIPER_RIFLE:     return RGB(240, 100, 235);
        case WeaponType::ROCKET_LAUNCHER:  return RGB(255, 98, 58);
        case WeaponType::FLAME_THROWER:    return RGB(255, 135, 46);
        case WeaponType::MAGIC_STAFF:      return RGB(188, 106, 255);
        case WeaponType::REBOUND_CROSSBOW: return RGB(132, 178, 255);
        case WeaponType::VAMPIRE_CODEX:    return RGB(190, 82, 210);
        default:
            return tier == 3 ? RGB(255, 218, 102) :
                   tier == 2 ? RGB(112, 220, 255) :
                               RGB(210, 220, 230);
    }
}

const char* WeaponShortName(WeaponType type) {
    return GetWeaponShortName(type);
}

void DrawWeaponGlyph(WeaponType type, int cx, int cy, COLORREF accent) {
    COLORREF outline = RGB(12, 14, 18);
    const WeaponSpec& spec = GetWeaponSpec(type);
    setfillcolor(RGB(30, 36, 46));
    solidrectangle(cx - 25, cy - 22, cx + 25, cy + 22);
    setlinecolor(RGB(80, 98, 112));
    rectangle(cx - 25, cy - 22, cx + 25, cy + 22);

    switch (type) {
        case WeaponType::SHOTGUN:
            VisualFX::DrawPixelRect(cx - 19, cy - 5, cx + 16, cy + 7, RGB(54, 56, 58), outline, 3);
            VisualFX::DrawPixelRect(cx - 3, cy - 11, cx + 25, cy - 5, accent, outline, 2);
            VisualFX::DrawPixelRect(cx - 3, cy + 5, cx + 25, cy + 11, RGB(255, 232, 112), outline, 2);
            setfillcolor(RGB(121, 80, 42));
            solidrectangle(cx - 20, cy + 9, cx - 4, cy + 17);
            break;
        case WeaponType::SNIPER_RIFLE:
            VisualFX::DrawPixelRect(cx - 23, cy - 4, cx + 17, cy + 6, RGB(63, 66, 76), outline, 3);
            VisualFX::DrawPixelRect(cx + 12, cy - 2, cx + 29, cy + 2, accent, outline, 2);
            VisualFX::DrawPixelRect(cx - 10, cy - 15, cx + 7, cy - 8, RGB(190, 225, 255), outline, 2);
            break;
        case WeaponType::ROCKET_LAUNCHER:
            VisualFX::DrawPixelRect(cx - 23, cy - 10, cx + 18, cy + 10, RGB(77, 82, 76), outline, 4);
            VisualFX::DrawPixelRect(cx + 13, cy - 13, cx + 28, cy + 13, accent, outline, 3);
            setfillcolor(RGB(255, 222, 104));
            solidrectangle(cx - 20, cy - 5, cx - 12, cy + 7);
            break;
        case WeaponType::FLAME_THROWER:
            VisualFX::DrawPixelRect(cx - 20, cy - 5, cx + 15, cy + 7, RGB(76, 70, 59), outline, 3);
            VisualFX::DrawPixelRect(cx + 8, cy - 3, cx + 28, cy + 4, accent, outline, 2);
            setfillcolor(RGB(255, 224, 74));
            solidrectangle(cx + 15, cy - 7, cx + 25, cy + 5);
            break;
        case WeaponType::MAGIC_STAFF:
            VisualFX::DrawPixelProjectile(cx + 4, cy + 15, 0.2f, -1.0f, 38, 4, accent, outline);
            VisualFX::DrawPixelDiamond(cx + 10, cy - 17, 9, RGB(224, 188, 255), outline);
            break;
        case WeaponType::REBOUND_CROSSBOW:
            VisualFX::DrawPixelProjectile(cx + 8, cy + 2, 1.0f, 0.0f, 32, 4, accent, outline);
            setlinecolor(outline);
            setlinestyle(PS_SOLID, 5);
            line(cx - 14, cy - 14, cx - 14, cy + 14);
            setlinestyle(PS_SOLID, 2);
            setlinecolor(accent);
            line(cx - 14, cy - 12, cx - 14, cy + 12);
            setlinestyle(PS_SOLID, 1);
            break;
        case WeaponType::VAMPIRE_CODEX:
            VisualFX::DrawPixelRect(cx - 14, cy - 18, cx + 16, cy + 18,
                                    RGB(76, 32, 96), outline, 4);
            VisualFX::DrawPixelDiamond(cx + 2, cy, 8, accent, outline);
            break;
        case WeaponType::ASSAULT_RIFLE:
        default:
            if (spec.pattern == WeaponPattern::EXPLOSIVE) {
                VisualFX::DrawPixelRect(cx - 23, cy - 10, cx + 18, cy + 10, RGB(77, 82, 76), outline, 4);
                VisualFX::DrawPixelRect(cx + 13, cy - 13, cx + 28, cy + 13, accent, outline, 3);
            } else if (spec.pattern == WeaponPattern::HOMING) {
                VisualFX::DrawPixelProjectile(cx + 4, cy + 15, 0.2f, -1.0f, 38, 4, accent, outline);
                VisualFX::DrawPixelDiamond(cx + 10, cy - 17, spec.tier >= 3 ? 11 : 8, accent, outline);
            } else if (spec.pattern == WeaponPattern::SPREAD) {
                VisualFX::DrawPixelRect(cx - 19, cy - 5, cx + 16, cy + 7, RGB(54, 56, 58), outline, 3);
                VisualFX::DrawPixelRect(cx - 3, cy - 11, cx + 25, cy - 5, accent, outline, 2);
                VisualFX::DrawPixelRect(cx - 3, cy + 5, cx + 25, cy + 11, RGB(255, 232, 112), outline, 2);
            } else if (spec.pattern == WeaponPattern::BOUNCE) {
                VisualFX::DrawPixelProjectile(cx + 8, cy + 2, 1.0f, 0.0f, 32, 4, accent, outline);
                setlinecolor(accent);
                setlinestyle(PS_SOLID, 3);
                line(cx - 14, cy - 13, cx - 14, cy + 13);
                setlinestyle(PS_SOLID, 1);
            } else if (spec.pattern == WeaponPattern::CODEX) {
                VisualFX::DrawPixelRect(cx - 14, cy - 18, cx + 16, cy + 18, RGB(76, 32, 96), outline, 4);
                VisualFX::DrawPixelDiamond(cx + 2, cy, 8, accent, outline);
            } else if (spec.pattern == WeaponPattern::LASER) {
                VisualFX::DrawPixelRect(cx - 23, cy - 4, cx + 17, cy + 6, RGB(63, 66, 76), outline, 3);
                VisualFX::DrawPixelRect(cx + 12, cy - 2, cx + 29, cy + 2, accent, outline, 2);
            } else {
                VisualFX::DrawPixelRect(cx - 22, cy - 6, cx + 15, cy + 7, RGB(61, 70, 76), outline, 3);
                VisualFX::DrawPixelRect(cx + 12, cy - 3, cx + 28, cy + 3, accent, outline, 2);
                VisualFX::DrawPixelRect(cx - 4, cy + 7, cx + 6, cy + 20, RGB(72, 61, 50), outline, 2);
            }
            break;
    }
}

void DrawWeaponStatPill(int x, int y, int w, const char* text, COLORREF fill, COLORREF edge) {
    VisualFX::DrawPixelRect(x, y, x + w, y + 22, fill, edge, 3);
    VisualFX::DrawTextShadow(text, x + 8, y + 2, RGB(236, 242, 244), 15);
}

} // namespace

UIManager::UIManager()
    : m_hWnd(nullptr), m_screenWidth(WINDOW_WIDTH), m_screenHeight(WINDOW_HEIGHT) {}

void UIManager::Init(HWND hWnd) {
    m_hWnd = hWnd;
    // 主菜单按钮
    UIButton btn;
    btn.bounds = AABB(WINDOW_WIDTH / 2.0f, 410.0f, 150.0f, 34.0f);
    btn.text = "开始游戏"; btn.id = 0;
    m_menuButtons.push_back(btn);
    btn.bounds = AABB(WINDOW_WIDTH / 2.0f, 490.0f, 150.0f, 34.0f);
    btn.text = "退出"; btn.id = 1;
    m_menuButtons.push_back(btn);

    // 角色选择按钮
    btn.bounds = AABB(WINDOW_WIDTH / 2.0f - 320.0f, 400.0f, 155.0f, 170.0f);
    btn.text = "骑士"; btn.id = 0;
    m_characterButtons.push_back(btn);
    btn.bounds = AABB(WINDOW_WIDTH / 2.0f, 400.0f, 155.0f, 170.0f);
    btn.text = "游侠"; btn.id = 1;
    m_characterButtons.push_back(btn);
    btn.bounds = AABB(WINDOW_WIDTH / 2.0f + 320.0f, 400.0f, 155.0f, 170.0f);
    btn.text = "法师"; btn.id = 2;
    m_characterButtons.push_back(btn);

    // 暂停菜单按钮
    btn.bounds = AABB(WINDOW_WIDTH / 2.0f, 376.0f, 140.0f, 34.0f);
    btn.text = "继续游戏"; btn.id = 0;
    m_pauseButtons.push_back(btn);
    btn.bounds = AABB(WINDOW_WIDTH / 2.0f, 446.0f, 140.0f, 34.0f);
    btn.text = "返回主菜单"; btn.id = 1;
    m_pauseButtons.push_back(btn);

    printf("[UIManager] Initialized.\n");
}

void UIManager::GetMousePos(int& outX, int& outY) const {
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(m_hWnd, &pt);
    outX = pt.x;
    outY = pt.y;
}

bool UIManager::IsMouseInRect(int mouseX, int mouseY, const AABB& rect) {
    return mouseX >= rect.Left() && mouseX <= rect.Right() &&
           mouseY >= rect.Top()  && mouseY <= rect.Bottom();
}

// ============================================================
// 主菜单
// ============================================================
void UIManager::RenderMainMenu() {
    cleardevice();
    DrawMenuBackdrop();

    DrawTextCenteredUI("Knight's Journey", WINDOW_WIDTH / 2, 142, RGB(255, 221, 116), 54);
    DrawTextCenteredUI("骑士远征", WINDOW_WIDTH / 2, 214, RGB(208, 232, 230), 28);

    AABB brief(WINDOW_WIDTH / 2.0f, 300.0f, 280.0f, 42.0f);
    VisualFX::DrawSoftPanel(brief, RGB(18, 34, 41), RGB(86, 142, 142), 18, true);
    DrawTextCenteredUI("三层地牢 · 多职业 · 武器与天赋构筑", WINDOW_WIDTH / 2, 285,
                       RGB(219, 229, 221), 20);

    for (auto& btn : m_menuButtons) DrawButton(btn);

    DrawTextCenteredUI("WASD/方向键移动  鼠标瞄准射击  空格技能  ESC暂停",
                       WINDOW_WIDTH / 2, 610, RGB(155, 179, 184), 18);
}

int UIManager::HandleMainMenuInput(int mouseX, int mouseY, bool mouseClick) {
    for (auto& btn : m_menuButtons) {
        btn.isHovered = IsMouseInRect(mouseX, mouseY, btn.bounds);
        if (btn.isHovered && mouseClick && btn.isEnabled) return btn.id;
    }
    return -1;
}

// ============================================================
// 角色选择
// ============================================================
void UIManager::RenderCharacterSelect() {
    cleardevice();
    DrawMenuBackdrop();
    DrawTextCenteredUI("选择你的英雄", WINDOW_WIDTH / 2, 88, RGB(255, 221, 116), 42);
    DrawTextCenteredUI("每个职业都有独立技能与节奏", WINDOW_WIDTH / 2, 142,
                       RGB(187, 209, 214), 20);

    const char* desc1[] = {
        "高生命与护盾",
        "火力全开提升射速",
        "适合正面推进"
    };
    const char* desc2[] = {
        "高速移动",
        "战术翻滚规避伤害",
        "翻滚后首击强化"
    };
    const char* desc3[] = {
        "高能量上限",
        "连锁闪电控制敌人",
        "适合远程清场"
    };

    for (auto& btn : m_characterButtons) {
        Profession prof = btn.id == 0 ? Profession::KNIGHT :
                          btn.id == 1 ? Profession::ROGUE : Profession::MAGE;
        COLORREF border = btn.isHovered ? RGB(255, 230, 128) : RGB(83, 126, 135);
        COLORREF fill = btn.id == 0 ? RGB(22, 42, 70) :
                        btn.id == 1 ? RGB(21, 58, 45) : RGB(47, 30, 70);
        VisualFX::DrawSoftPanel(btn.bounds, fill, border, 18, true);
        DrawClassEmblem((int)btn.bounds.x, (int)btn.bounds.y - 70, prof, btn.isHovered);
        DrawTextCenteredUI(btn.text.c_str(), (int)btn.bounds.x, (int)btn.bounds.y - 18,
                           RGB(255, 248, 215), 28);

        const char** lines = btn.id == 0 ? desc1 : (btn.id == 1 ? desc2 : desc3);
        for (int i = 0; i < 3; ++i) {
            DrawTextCenteredUI(lines[i], (int)btn.bounds.x,
                               (int)btn.bounds.y + 22 + i * 27,
                               RGB(213, 226, 226), 17);
        }
    }
}

int UIManager::HandleCharacterSelectInput(int mouseX, int mouseY, bool mouseClick) {
    for (auto& btn : m_characterButtons) {
        btn.isHovered = IsMouseInRect(mouseX, mouseY, btn.bounds);
        if (btn.isHovered && mouseClick && btn.isEnabled) return btn.id;
    }
    return -1;
}

// ============================================================
// HUD
// ============================================================
void UIManager::RenderHUD(Player* player, const BuffManager& buffMgr,
                          int currentLevel, BiomeType biome, int gold,
                          const RoomData* currentRoom, int activeEnemies) {
    char buf[128];
    const int sideLeft = ROOM_WIDTH + 32;
    const int sideCenter = ROOM_WIDTH + (WINDOW_WIDTH - ROOM_WIDTH) / 2;

    // 右侧信息栏
    VisualFX::DrawSoftPanel(AABB((float)sideCenter, WINDOW_HEIGHT / 2.0f, 178.0f, 430.0f),
                            RGB(15, 23, 34), RGB(62, 91, 110), 18, true);

    // 右侧战斗状态，避免遮挡房间左上角刷怪/弹幕区域
    sprintf(buf, "第 %d 图-%d", (currentLevel - 1) / 3 + 1, (currentLevel - 1) % 3 + 1);
    DrawTextUI(buf, sideLeft, 374, RGB(255, 232, 145), 22);
    DrawTextUI(VisualFX::GetBiomeName(biome), sideLeft + 92, 378, RGB(205, 226, 227), 18);

    if (currentRoom) {
        sprintf(buf, "%s%s", VisualFX::GetRoomTypeName(currentRoom->type),
                currentRoom->isCleared ? "  已清理" : "");
        DrawTextUI(buf, sideLeft, 410, currentRoom->isCleared ? RGB(144, 244, 150) : RGB(210, 222, 230), 18);

        if (currentRoom->totalWaveCount > 0) {
            sprintf(buf, "波次 %d/%d", currentRoom->currentWave, currentRoom->totalWaveCount);
            DrawTextUI(buf, sideLeft, 442, RGB(183, 207, 216), 17);
            sprintf(buf, "剩余敌人 %d", activeEnemies);
            DrawTextUI(buf, sideLeft, 470, activeEnemies == 0 ? RGB(143, 237, 156) : RGB(235, 188, 118), 17);
        } else if (currentRoom->type == RoomType::SHOP) {
            DrawTextUI("补给与新装备", sideLeft, 442, RGB(183, 207, 216), 17);
        } else if (currentRoom->type == RoomType::REWARD) {
            DrawTextUI("寻找宝箱奖励", sideLeft, 442, RGB(235, 211, 128), 17);
        }
    }

    // 金币
    VisualFX::DrawSoftPanel(AABB((float)sideLeft + 112.0f, 512.0f, 104.0f, 24.0f),
                            RGB(29, 24, 15), RGB(148, 115, 44), 12, false);
    VisualFX::DrawDiamond(sideLeft + 34, 512, 11, RGB(255, 213, 66), RGB(255, 247, 181));
    sprintf(buf, "%d G", gold);
    DrawTextUI(buf, sideLeft + 56, 499, RGB(255, 220, 96), 20);

    if (currentRoom &&
        (currentRoom->type == RoomType::BOSS || currentRoom->type == RoomType::EXIT) &&
        currentRoom->isCleared) {
        VisualFX::DrawSoftPanel(AABB(ROOM_WIDTH / 2.0f, ROOM_HEIGHT + 48.0f, 230.0f, 24.0f),
                                RGB(34, 30, 12), RGB(238, 198, 75), 14, true);
        DrawTextCenteredUI("按 Enter 进入下一关", ROOM_WIDTH / 2, ROOM_HEIGHT + 34,
                           RGB(255, 238, 145), 20);
    }

    if (!player || player->IsDead()) return;

    // HP / 护盾 / MP 条
    VisualFX::DrawSoftPanel(AABB(157.0f, ROOM_HEIGHT + 50.0f, 147.0f, 36.0f),
                            RGB(12, 20, 30), RGB(58, 90, 110), 16, true);
    RenderHPBar(22, ROOM_HEIGHT + 18, player->GetHP(), player->GetMaxHP(), RGB(237, 78, 82), RGB(57, 24, 30));
    RenderShieldBar(22, ROOM_HEIGHT + 45, player->GetShield(), player->GetMaxShield());
    RenderMPBar(22, ROOM_HEIGHT + 68, player->GetMP(), player->GetMaxMP());

    // 武器槽（最多 2 个）
    DrawTextUI("武器", sideLeft, 650, RGB(224, 232, 235), 21);
    for (int i = 0; i < 2; ++i) {
        Weapon* w = player->GetWeapon(i);
        bool active = (w == player->GetCurrentWeapon());
        RenderWeaponSlot((float)sideLeft + i * 176.0f, 680.0f, w, active, i);
    }

    // 技能图标
    float cdRem = player->GetSkillCooldownRemaining();
    float cdMax = player->GetSkillCooldown();
    DrawTextUI("技能", sideLeft, 792, RGB(224, 232, 235), 21);
    RenderSkillIcon((float)sideLeft, 820.0f, cdRem, cdMax);

    // 已激活天赋
    const auto& buffs = buffMgr.GetActiveBuffs();
    DrawTextUI("天赋", sideLeft, 548, RGB(198, 216, 225), 19);
    if (buffs.empty()) {
        DrawTextUI("暂无", sideLeft, 580, RGB(116, 132, 144), 17);
    } else {
        int shown = 0;
        for (const auto& b : buffs) {
            if (shown >= 4) break;
            int x = sideLeft + 4 + (shown % 2) * 160;
            int y = 580 + (shown / 2) * 32;
            VisualFX::DrawSoftPanel(AABB((float)x + 66.0f, (float)y + 12.0f, 64.0f, 15.0f),
                                    RGB(24, 36, 49), RGB(78, 112, 125), 10, false);
            DrawTextUI(b.name.c_str(), x + 8, y + 1, RGB(226, 233, 218), 15);
            shown++;
        }
    }
}

void UIManager::RenderBossBar(const Boss* boss) {
    if (!boss || boss->IsDead()) return;

    const int barW = 680;
    const int barH = 24;
    const int x = ROOM_WIDTH / 2 - barW / 2;
    const int y = 22;
    float hpRatio = boss->GetMaxHP() > 0 ? (float)boss->GetHP() / boss->GetMaxHP() : 0.0f;

    VisualFX::DrawSoftPanel(AABB(ROOM_WIDTH / 2.0f, 42.0f, 360.0f, 34.0f),
                            RGB(22, 15, 19), RGB(142, 65, 74), 18, true);
    DrawTextCenteredUI(boss->GetBossName(), ROOM_WIDTH / 2, 9, RGB(255, 224, 160), 20);
    VisualFX::DrawProgressBar(x, y + 28, barW, barH, hpRatio,
                              hpRatio > 0.35f ? RGB(221, 66, 78) : RGB(255, 148, 62),
                              RGB(54, 25, 31), RGB(165, 86, 84));

    char buf[64];
    sprintf(buf, "%d / %d", boss->GetHP(), boss->GetMaxHP());
    DrawTextCenteredUI(buf, ROOM_WIDTH / 2, y + 26, RGB(255, 242, 226), 15);
}

void UIManager::RenderHPBar(float x, float y, int current, int max, COLORREF fill, COLORREF bg) {
    int w = 260, h = 20;
    float ratio = max > 0 ? (float)current / max : 0.0f;
    VisualFX::DrawProgressBar((int)x, (int)y, w, h, ratio, fill, bg, RGB(126, 72, 78));
    char buf[32];
    sprintf(buf, "生命 %d/%d", current, max);
    DrawTextUI(buf, (int)x + 12, (int)y - 1, RGB(255, 247, 240), 15);
}

void UIManager::RenderShieldBar(float x, float y, int current, int max) {
    if (max <= 0) return;
    int w = 260, h = 15;
    float ratio = max > 0 ? (float)current / max : 0.0f;
    VisualFX::DrawProgressBar((int)x, (int)y, w, h, ratio,
                              RGB(80, 193, 230), RGB(20, 38, 55), RGB(66, 116, 142));
    char buf[32];
    sprintf(buf, "护盾 %d/%d", current, max);
    DrawTextUI(buf, (int)x + 12, (int)y - 3, RGB(230, 248, 255), 13);
}

void UIManager::RenderMPBar(float x, float y, int current, int max) {
    int w = 260, h = 16;
    float ratio = max > 0 ? (float)current / max : 0.0f;
    VisualFX::DrawProgressBar((int)x, (int)y, w, h, ratio,
                              RGB(91, 129, 255), RGB(23, 31, 69), RGB(74, 91, 162));
    char buf[32];
    sprintf(buf, "能量 %d/%d", current, max);
    DrawTextUI(buf, (int)x + 12, (int)y - 2, RGB(236, 240, 255), 14);
}

void UIManager::RenderSkillIcon(float x, float y, float cdRem, float cdMax) {
    int size = 66;
    bool ready = cdRem <= 0.0f;
    COLORREF border = ready ? RGB(255, 228, 118) : RGB(82, 102, 126);
    COLORREF fill = ready ? RGB(43, 45, 68) : RGB(29, 34, 48);
    VisualFX::DrawPixelPanel((int)x, (int)y, (int)x + size, (int)y + size, fill, border, true);
    if (ready) {
        setfillcolor(RGB(255, 226, 82));
        solidrectangle((int)x + size - 13, (int)y + 9, (int)x + size - 5, (int)y + 17);
    }

    // 技能符号
    setlinecolor(ready ? RGB(255, 238, 140) : RGB(120, 134, 152));
    setlinestyle(PS_SOLID, 3);
    line((int)x + 18, (int)y + 39, (int)x + 30, (int)y + 16);
    line((int)x + 30, (int)y + 16, (int)x + 40, (int)y + 30);
    line((int)x + 40, (int)y + 30, (int)x + 28, (int)y + 43);
    setlinestyle(PS_SOLID, 1);

    if (!ready) {
        float f = cdRem / cdMax;
        if (f < 0.0f) f = 0.0f;
        if (f > 1.0f) f = 1.0f;
        setfillcolor(RGB(8, 11, 18));
        solidrectangle((int)x + 3, (int)y + 3, (int)x + size - 3, (int)y + 3 + (int)((size - 6) * f));
        char buf[8];
        sprintf(buf, "%.1f", cdRem);
        DrawTextCenteredUI(buf, (int)x + size / 2, (int)y + 24, RGB(220, 228, 238), 18);
    } else {
        DrawTextCenteredUI("Ready", (int)x + size / 2, (int)y + 74, RGB(255, 230, 124), 14);
    }
}

void UIManager::RenderWeaponSlot(float x, float y, Weapon* weapon, bool active, int slot) {
    int w = 166, h = 104;
    COLORREF accent = weapon ? WeaponAccent(weapon->GetType()) : RGB(110, 120, 130);
    COLORREF bg = active ? RGB(39, 45, 62) : RGB(21, 28, 38);
    COLORREF border = active ? accent : RGB(70, 91, 112);
    VisualFX::DrawPixelPanel((int)x, (int)y, (int)x + w, (int)y + h, bg, border, true);

    if (active) {
        setfillcolor(accent);
        solidrectangle((int)x + 8, (int)y + 8, (int)x + w - 8, (int)y + 14);
        DrawTextUI("当前", (int)x + w - 52, (int)y + 18, RGB(255, 242, 158), 12);
    } else {
        DrawTextUI("备用", (int)x + w - 52, (int)y + 18, RGB(150, 166, 176), 12);
    }

    if (weapon) {
        DrawWeaponGlyph(weapon->GetType(), (int)x + w / 2, (int)y + 44, accent);
        DrawTextCenteredUI(WeaponShortName(weapon->GetType()), (int)x + w / 2,
                           (int)y + 66, RGB(246, 248, 250), 15);

        char stat[24];
        sprintf(stat, "伤 %d", weapon->GetBaseDamage());
        DrawWeaponStatPill((int)x + 10, (int)y + 82, 46, stat,
                           RGB(44, 38, 36), VisualFX::ScaleColor(accent, 0.78f));
        sprintf(stat, "蓝 %d", weapon->GetMPCost());
        DrawWeaponStatPill((int)x + 60, (int)y + 82, 46, stat,
                           RGB(31, 39, 54), RGB(76, 104, 172));
        sprintf(stat, "%.1f/s", weapon->GetFireRate());
        DrawWeaponStatPill((int)x + 110, (int)y + 82, 46, stat,
                           RGB(34, 44, 39), RGB(82, 138, 91));

        float cd = weapon->GetCooldownRemaining();
        if (cd > 0.0f) {
            float ratio = cd * weapon->GetFireRate();
            if (ratio > 1.0f) ratio = 1.0f;
            VisualFX::DrawProgressBar((int)x + 10, (int)y + 18, w - 20, 9, 1.0f - ratio,
                                      accent, RGB(26, 31, 38), RGB(78, 88, 100));
        } else {
            setfillcolor(VisualFX::ScaleColor(accent, active ? 1.08f : 0.72f));
            solidrectangle((int)x + 12, (int)y + 19, (int)x + w - 12, (int)y + 25);
        }
    } else {
        VisualFX::DrawPixelPanel((int)x + 36, (int)y + 28, (int)x + w - 36, (int)y + 70,
                                 RGB(31, 36, 42), RGB(65, 75, 84), false);
        DrawTextCenteredUI("空槽", (int)x + w / 2, (int)y + 76, RGB(128, 139, 148), 15);
    }

    char slotLabel[4];
    sprintf(slotLabel, "%d", slot + 1);
    VisualFX::DrawPixelRect((int)x + 10, (int)y + 18, (int)x + 34, (int)y + 42,
                            active ? RGB(49, 55, 72) : RGB(34, 40, 50),
                            active ? accent : RGB(76, 88, 100), 3);
    DrawTextCenteredUI(slotLabel, (int)x + 22, (int)y + 19,
                       active ? RGB(255, 238, 136) : RGB(176, 186, 196), 13);
}

void UIManager::RenderMiniMap(const MapGenerator* map) {
    if (!map) return;

    const int sideLeft = ROOM_WIDTH + 32;
    const int sideCenter = ROOM_WIDTH + (WINDOW_WIDTH - ROOM_WIDTH) / 2;
    const int originX = sideLeft;
    const int originY = 76;
    const int cell = 32;
    const int gap = 6;
    const RoomCoord current = map->GetCurrentRoomCoord();

    VisualFX::DrawSoftPanel(AABB((float)sideCenter, 210.0f, 175.0f, 165.0f),
                            RGB(14, 24, 35), RGB(63, 92, 112), 18, true);
    DrawTextUI("地图", originX, 45, RGB(205, 222, 228), 20);

    const auto& rooms = map->GetAllRooms();
    for (const RoomData& room : rooms) {
        int x = originX + room.coord.col * (cell + gap);
        int y = originY + room.coord.row * (cell + gap);
        bool isCurrent = (room.coord == current);

        COLORREF color = RoomColor(room.type);
        if (!room.isDiscovered && !isCurrent) {
            color = RGB(45, 55, 66);
        }
        COLORREF fill = room.isCleared ? VisualFX::ScaleColor(color, 0.55f) : VisualFX::ScaleColor(color, 0.38f);
        COLORREF border = isCurrent ? RGB(255, 234, 126) : (room.isDiscovered ? color : RGB(70, 78, 88));
        VisualFX::DrawPixelPanel(x, y, x + cell, y + cell, fill, border, false);

        if (room.type == RoomType::BOSS || room.type == RoomType::SHOP ||
            room.type == RoomType::REWARD || room.type == RoomType::EXIT) {
            const char* mark = room.type == RoomType::BOSS ? "B" :
                               (room.type == RoomType::SHOP ? "$" :
                               (room.type == RoomType::EXIT ? "X" : "*"));
            DrawTextCenteredUI(mark, x + cell / 2, y + 6, RGB(255, 246, 197), 16);
        }
        if (isCurrent) {
            setlinecolor(RGB(255, 234, 126));
            setlinestyle(PS_SOLID, 3);
            rectangle(x - 4, y - 4, x + cell + 4, y + cell + 4);
            setlinestyle(PS_SOLID, 1);
        }
    }

    DrawSmallIcon(originX + 10, 346, RoomColor(RoomType::START), "起点");
    DrawSmallIcon(originX + 135, 346, RoomColor(RoomType::BOSS), "首领");
}

// ============================================================
// 暂停菜单
// ============================================================
void UIManager::RenderPauseMenu() {
    DrawPanel(AABB(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f, 245.0f, 170.0f),
              RGB(14, 22, 34), RGB(115, 150, 170));

    DrawTextCenteredUI("游戏暂停", WINDOW_WIDTH / 2, 260, RGB(255, 232, 150), 38);
    DrawTextCenteredUI("调整节奏，下一波再冲", WINDOW_WIDTH / 2, 314,
                       RGB(183, 203, 210), 16);
    for (auto& btn : m_pauseButtons) DrawButton(btn);
}

int UIManager::HandlePauseMenuInput(int mouseX, int mouseY, bool mouseClick) {
    for (auto& btn : m_pauseButtons) {
        btn.isHovered = IsMouseInRect(mouseX, mouseY, btn.bounds);
        if (btn.isHovered && mouseClick && btn.isEnabled) return btn.id;
    }
    return -1;
}

// ============================================================
// 天赋选择
// ============================================================
void UIManager::RenderTalentSelection(const std::vector<BuffData*>& talents,
                                       const BuffManager& buffMgr) {
    (void)buffMgr;
    cleardevice();
    DrawMenuBackdrop();
    DrawTextCenteredUI("选择一项天赋", WINDOW_WIDTH / 2, 78, RGB(255, 221, 116), 40);
    DrawTextCenteredUI("强化你的构筑，准备进入下一层", WINDOW_WIDTH / 2, 130,
                       RGB(188, 211, 214), 18);

    for (int i = 0; i < (int)talents.size() && i < 3; ++i) {
        BuffData* b = talents[i];
        if (!b) continue;

        AABB box;
        if (i < (int)m_talentOptions.size()) {
            box = m_talentOptions[i].bounds;
        } else {
            float panelW = 260.0f;
            float cx = WINDOW_WIDTH / 2.0f - 280.0f + i * 280.0f;
            box = AABB(cx, WINDOW_HEIGHT / 2.0f + 26.0f, panelW / 2.0f, 150.0f);
        }

        int mx, my;
        GetMousePos(mx, my);
        bool hovered = IsMouseInRect(mx, my, box);
        COLORREF border = hovered ? RGB(255, 229, 112) : RGB(91, 124, 140);
        VisualFX::DrawSoftPanel(box, RGB(18, 30, 43), border, 18, true);
        VisualFX::DrawGlowCircle((int)box.x, (int)box.y - 78, 24,
                                 RoomColor(RoomType::REWARD), hovered ? 5 : 3);
        VisualFX::DrawDiamond((int)box.x, (int)box.y - 78, 32,
                              RGB(224, 177, 72), RGB(255, 243, 184));
        DrawTextCenteredUI(b->name.c_str(), (int)box.x, (int)box.y - 30,
                           RGB(255, 241, 172), 24);
        DrawTextCenteredUI(b->description.c_str(), (int)box.x, (int)box.y + 20,
                           RGB(216, 226, 226), 16);
        DrawTextCenteredUI("点击选择", (int)box.x, (int)box.y + 96,
                           hovered ? RGB(255, 226, 116) : RGB(150, 170, 178), 15);
    }
}

int UIManager::HandleTalentSelectionInput(int mouseX, int mouseY, bool mouseClick) {
    if (!mouseClick) return -1;
    if (!m_talentOptions.empty()) {
        for (int i = 0; i < (int)m_talentOptions.size(); ++i) {
            if (IsMouseInRect(mouseX, mouseY, m_talentOptions[i].bounds)) return i;
        }
    } else {
        for (int i = 0; i < 3; ++i) {
            float panelW = 260.0f;
            float cx = WINDOW_WIDTH / 2.0f - 280.0f + i * 280.0f;
            AABB box(cx, WINDOW_HEIGHT / 2.0f + 26.0f, panelW / 2.0f, 150.0f);
            if (IsMouseInRect(mouseX, mouseY, box)) return i;
        }
    }
    return -1;
}

// ============================================================
// 商店
// ============================================================
void UIManager::RenderShop(const std::vector<ShopItemData>& items, int playerGold, float discount) {
    cleardevice();
    DrawMenuBackdrop();

    DrawTextCenteredUI("旅途商店", WINDOW_WIDTH / 2, 50, RGB(255, 221, 116), 38);
    DrawTextCenteredUI("选择补给或新装备", WINDOW_WIDTH / 2, 102, RGB(196, 211, 204), 17);

    char buf[64];
    VisualFX::DrawSoftPanel(AABB(112.0f, 86.0f, 88.0f, 28.0f),
                            RGB(33, 27, 16), RGB(148, 115, 44), 12, true);
    VisualFX::DrawDiamond(40, 86, 12, RGB(255, 213, 66), RGB(255, 247, 181));
    sprintf(buf, "%d G", playerGold);
    DrawTextUI(buf, 64, 72, RGB(255, 220, 96), 20);

    int mx, my;
    GetMousePos(mx, my);

    int displayIdx = 0;
    for (int i = 0; i < (int)items.size(); ++i) {
        if (items[i].isSold) continue;  // 已售物品不显示
        float cy = 224.0f + displayIdx * 128.0f;
        AABB box(WINDOW_WIDTH / 2.0f, cy, 290.0f, 48.0f);
        bool hovered = IsMouseInRect(mx, my, box);
        bool canBuy = (int)(items[i].price * discount) <= playerGold;
        COLORREF border = hovered ? RGB(255, 226, 112) : RGB(98, 111, 119);
        COLORREF fill = canBuy ? RGB(24, 34, 43) : RGB(38, 30, 31);
        VisualFX::DrawSoftPanel(box, fill, border, 16, true);

        COLORREF iconColor = items[i].itemType == ShopItemType::WEAPON ?
                             RGB(184, 124, 255) : RGB(255, 116, 116);
        VisualFX::DrawDiamond((int)box.Left() + 42, (int)cy, 22, iconColor, RGB(255, 245, 210));

        DrawTextUI(items[i].name.c_str(), (int)box.Left() + 84, (int)cy - 22,
                   canBuy ? RGB(245, 248, 248) : RGB(164, 145, 145), 21);
        sprintf(buf, "%d G", (int)(items[i].price * discount));
        DrawTextUI(buf, (int)box.Left() + 84, (int)cy + 7,
                   canBuy ? RGB(126, 245, 145) : RGB(255, 117, 117), 17);
        displayIdx++;
    }
    if (displayIdx == 0) {
        DrawTextCenteredUI("商品已售罄", WINDOW_WIDTH / 2, 300, RGB(164, 174, 176), 24);
    }

    DrawTextCenteredUI("按 B 或 ESC 离开商店", WINDOW_WIDTH / 2, 628, RGB(160, 173, 176), 16);
}

int UIManager::HandleShopInput(int mouseX, int mouseY, bool mouseClick) {
    if (!mouseClick) return -1;
    int displayIdx = 0;
    for (int i = 0; i < (int)m_shopItems.size(); ++i) {
        if (m_shopItems[i].isSold) continue;  // 跳过已售
        float cy = 224.0f + displayIdx * 128.0f;
        AABB box(WINDOW_WIDTH / 2.0f, cy, 290.0f, 48.0f);
        m_shopItems[i].isHovered = IsMouseInRect(mouseX, mouseY, box);
        if (m_shopItems[i].isHovered) return i;
        displayIdx++;
    }
    return -1;
}

// ============================================================
// 武器选择（武器槽满时替换）
// ============================================================
void UIManager::RenderWeaponSelection(Player* player) {
    if (!player) return;

    Weapon* pending = player->GetPendingWeapon();
    if (!pending) return;

    DrawTextCenteredUI("武器槽已满", WINDOW_WIDTH / 2, 84, RGB(255, 221, 116), 38);
    DrawTextCenteredUI("选择一个槽位替换，或放弃新武器", WINDOW_WIDTH / 2, 136,
                       RGB(199, 215, 219), 17);

    // 三个面板：Slot 1, Slot 2, 新武器
    float panelW = 200.0f, panelH = 160.0f;
    float startX = WINDOW_WIDTH / 2.0f - panelW * 1.5f - 20.0f;
    float panelY = WINDOW_HEIGHT / 2.0f - panelH / 2.0f;

    for (int i = 0; i < 3; ++i) {
        float cx = startX + (panelW + 20.0f) * i + panelW / 2.0f;
        AABB box(cx, panelY + panelH / 2.0f, panelW / 2.0f, panelH / 2.0f);

        const char* label;
        const char* weaponName;
        COLORREF bgColor;
        COLORREF borderColor = COLOR_BORDER;

        if (i == 0 || i == 1) {
            // 已有武器槽
            Weapon* w = player->GetWeapon(i);
            label = i == 0 ? "槽位 1 (按 1)" : "槽位 2 (按 2)";
            weaponName = w ? w->GetName() : "空";
            bgColor = RGB(20, 30, 50);
            if (w == player->GetCurrentWeapon()) {
                borderColor = RGB(255, 255, 100);
            }
        } else {
            // 新武器
            label = "新武器 (放弃)";
            weaponName = pending->GetName();
            bgColor = RGB(48, 28, 42);
            borderColor = RGB(220, 120, 150);
        }

        VisualFX::DrawSoftPanel(box, bgColor, borderColor, 18, true);
        VisualFX::DrawDiamond((int)cx, (int)(panelY + 50),
                              i == 2 ? 26 : 22,
                              i == 2 ? RGB(200, 90, 130) : RGB(110, 142, 216),
                              RGB(255, 242, 196));
        DrawTextCenteredUI(label, (int)cx, (int)(panelY + 10), RGB(200, 200, 200), 16);
        DrawTextCenteredUI(weaponName, (int)cx, (int)(panelY + 84), RGB(255, 255, 255), 18);

        if (i == 2) {
            DrawTextCenteredUI("点击放弃拾取", (int)cx, (int)(panelY + 120), RGB(230, 180, 190), 14);
        } else {
            DrawTextCenteredUI("点击替换此槽位", (int)cx, (int)(panelY + 120), RGB(200, 220, 230), 14);
        }
    }

    DrawTextCenteredUI("ESC - 放弃拾取", WINDOW_WIDTH / 2, WINDOW_HEIGHT - 50, RGB(170, 184, 190), 16);
}

int UIManager::HandleWeaponSelectInput(int mouseX, int mouseY, Player* player, bool mouseClick) {
    if (!player || !mouseClick) return -2;  // -2 = no action

    float panelW = 200.0f, panelH = 160.0f;
    float startX = WINDOW_WIDTH / 2.0f - panelW * 1.5f - 20.0f;
    float panelY = WINDOW_HEIGHT / 2.0f - panelH / 2.0f;

    for (int i = 0; i < 2; ++i) {
        float cx = startX + (panelW + 20.0f) * i + panelW / 2.0f;
        AABB box(cx, panelY + panelH / 2.0f, panelW / 2.0f, panelH / 2.0f);
        if (IsMouseInRect(mouseX, mouseY, box)) return i;  // 替换槽位 i
    }

    // 第三个面板：拾取新武器（如果槽位有空其实不该到这里，但处理一下）
    {
        float cx = startX + (panelW + 20.0f) * 2 + panelW / 2.0f;
        AABB box(cx, panelY + panelH / 2.0f, panelW / 2.0f, panelH / 2.0f);
        if (IsMouseInRect(mouseX, mouseY, box)) return -1;  // 放弃
    }

    return -2;
}
void UIManager::RenderRewardRoom() {
    cleardevice();
    DrawMenuBackdrop();
    VisualFX::DrawSoftPanel(AABB(WINDOW_WIDTH / 2.0f, 350.0f, 250.0f, 120.0f),
                            RGB(42, 37, 30), RGB(209, 170, 80), 20, true);
    VisualFX::DrawDiamond(WINDOW_WIDTH / 2, 302, 46, RGB(222, 162, 64), RGB(255, 243, 180));
    DrawTextCenteredUI("奖励房间", WINDOW_WIDTH / 2, 380, RGB(255, 221, 116), 36);
    DrawTextCenteredUI("打开宝箱获取奖励！", WINDOW_WIDTH / 2, 438, RGB(230, 236, 232), 22);
}

void UIManager::RenderGameOver(bool victory, int finalLevel, int totalGold) {
    cleardevice();
    DrawMenuBackdrop();
    const char* title = victory ? "胜利！" : "游戏结束";
    COLORREF tc = victory ? RGB(255, 221, 116) : RGB(255, 94, 104);
    VisualFX::DrawSoftPanel(AABB(WINDOW_WIDTH / 2.0f, 360.0f, 240.0f, 150.0f),
                            RGB(16, 23, 32), victory ? RGB(188, 160, 82) : RGB(152, 70, 84), 20, true);
    DrawTextCenteredUI(title, WINDOW_WIDTH / 2, 226, tc, 52);
    char buf[64];
    sprintf(buf, "到达关卡: %d", finalLevel);
    DrawTextCenteredUI(buf, WINDOW_WIDTH / 2, 328, RGB(239, 244, 244), 24);
    sprintf(buf, "获得金币: %d", totalGold);
    DrawTextCenteredUI(buf, WINDOW_WIDTH / 2, 374, RGB(255, 221, 116), 24);
    DrawTextCenteredUI("点击 / Enter / 空格 返回主菜单", WINDOW_WIDTH / 2, 462, RGB(182, 198, 204), 18);
}

// ============================================================
// 通用绘制
// ============================================================
void UIManager::DrawTextUI(const char* text, int x, int y, COLORREF color, int fontSize) {
    int scaled = fontSize + (fontSize <= 18 ? 5 : (fontSize <= 24 ? 4 : 3));
    VisualFX::DrawTextShadow(text, x, y, color, scaled);
}

void UIManager::DrawTextCenteredUI(const char* text, int centerX, int y, COLORREF color, int fontSize) {
    setbkmode(TRANSPARENT);
    int scaled = fontSize + (fontSize <= 18 ? 5 : (fontSize <= 24 ? 4 : 3));
    settextstyle(scaled, 0, _T("Microsoft YaHei UI"));
    int w = textwidth(text);
    VisualFX::DrawTextShadow(text, centerX - w / 2, y, color, scaled);
}

void UIManager::DrawRectUI(const AABB& rect, COLORREF fillColor, COLORREF borderColor, int borderWidth) {
    VisualFX::DrawRoundFrame((int)rect.Left(), (int)rect.Top(),
                             (int)rect.Right(), (int)rect.Bottom(),
                             14, fillColor, borderColor, borderWidth);
}

void UIManager::DrawButton(const UIButton& btn) {
    COLORREF bg = !btn.isEnabled ? COLOR_BUTTON_DISABLED :
                  btn.isHovered ? COLOR_BUTTON_HOVER : COLOR_BUTTON_NORMAL;
    VisualFX::DrawButtonSurface(btn.bounds, bg, COLOR_BORDER, btn.isHovered, btn.isEnabled, 18);
    DrawTextCenteredUI(btn.text.c_str(), (int)btn.bounds.x, (int)btn.bounds.y - 8,
                     btn.isEnabled ? RGB(255, 248, 224) : RGB(130, 136, 142), 22);
}

void UIManager::DrawPanel(const AABB& rect, COLORREF bgColor, COLORREF borderColor, int alpha) {
    (void)alpha;
    VisualFX::DrawSoftPanel(rect, bgColor, borderColor, 18, true);
}

void UIManager::RenderTransitionFade(float progress) {
    if (progress > 0.2f) {
        VisualFX::DrawDimOverlay(RGB(0, 0, 0), progress > 0.65f ? 2 : 4);
    }
}

void UIManager::RenderScreenShake(int intensity) { (void)intensity; }

void UIManager::ShowMessageBox(const char* title, const char* message) {
    MessageBox(m_hWnd, message, title, MB_OK);
}

void UIManager::SetDefaultFont(int height, const char* fontName) {
    settextstyle(height, 0, fontName);
}

void UIManager::DrawString_Safe(int x, int y, const char* str, COLORREF color) {
    VisualFX::DrawTextShadow(str, x, y, color, 18);
}
