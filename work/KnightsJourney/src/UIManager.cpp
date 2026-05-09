// ============================================================
// UIManager.cpp - UI 管理器（HUD / 菜单 / 商店 / 天赋选择）
// ============================================================

#include "../include/UI/UIManager.h"
#include "../include/Entity/Player.h"
#include "../include/Weapon/Weapon.h"
#include "../include/System/MapGenerator.h"
#include "../include/System/BuffManager.h"
#include <graphics.h>
#include <windows.h>
#include <cstdio>
#include <cstring>

// Windows.h (via graphics.h) defines DrawText as DrawTextA/W macro.
// Undefine it so our UIManager method names don't get corrupted.
#ifdef DrawText
#undef DrawText
#endif

UIManager::UIManager()
    : m_hWnd(nullptr), m_screenWidth(WINDOW_WIDTH), m_screenHeight(WINDOW_HEIGHT) {}

void UIManager::Init(HWND hWnd) {
    m_hWnd = hWnd;
    // 主菜单按钮
    UIButton btn;
    btn.bounds = AABB(WINDOW_WIDTH / 2.0f, 360.0f, 120.0f, 30.0f);
    btn.text = "开始游戏"; btn.id = 0;
    m_menuButtons.push_back(btn);
    btn.bounds = AABB(WINDOW_WIDTH / 2.0f, 420.0f, 120.0f, 30.0f);
    btn.text = "退出"; btn.id = 1;
    m_menuButtons.push_back(btn);

    // 角色选择按钮
    btn.bounds = AABB(WINDOW_WIDTH / 2.0f, 280.0f, 100.0f, 40.0f);
    btn.text = "骑士"; btn.id = 0;
    m_characterButtons.push_back(btn);
    btn.bounds = AABB(WINDOW_WIDTH / 2.0f, 340.0f, 100.0f, 40.0f);
    btn.text = "游侠"; btn.id = 1;
    m_characterButtons.push_back(btn);
    btn.bounds = AABB(WINDOW_WIDTH / 2.0f, 400.0f, 100.0f, 40.0f);
    btn.text = "法师"; btn.id = 2;
    m_characterButtons.push_back(btn);

    // 暂停菜单按钮
    btn.bounds = AABB(WINDOW_WIDTH / 2.0f, 360.0f, 140.0f, 35.0f);
    btn.text = "继续游戏"; btn.id = 0;
    m_pauseButtons.push_back(btn);
    btn.bounds = AABB(WINDOW_WIDTH / 2.0f, 420.0f, 140.0f, 35.0f);
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
    DrawTextCenteredUI("Knight's Journey", WINDOW_WIDTH / 2, 150, RGB(255, 215, 0), 48);
    DrawTextCenteredUI("传奇引擎", WINDOW_WIDTH / 2, 210, RGB(200, 200, 200), 24);
    for (auto& btn : m_menuButtons) DrawButton(btn);
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
    DrawTextCenteredUI("选择你的英雄", WINDOW_WIDTH / 2, 150, RGB(255, 215, 0), 40);
    for (auto& btn : m_characterButtons) DrawButton(btn);

    DrawTextCenteredUI("骑士 - 火力全开，双倍射速", WINDOW_WIDTH / 2, 480, RGB(100, 180, 255), 20);
    DrawTextCenteredUI("游侠 - 战术翻滚，额外伤害", WINDOW_WIDTH / 2, 510, RGB(100, 255, 100), 20);
    DrawTextCenteredUI("法师 - 奥术闪电，远程控制", WINDOW_WIDTH / 2, 540, RGB(200, 120, 255), 20);
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
                          const RoomData* currentRoom) {
    char buf[128];

    // 左上角：关卡和 Biome
    const char* biomeStr = "森林";
    if (biome == BiomeType::ICE_DUNGEON) biomeStr = "冰原";
    else if (biome == BiomeType::VOLCANO) biomeStr = "火山";
    sprintf(buf, "关卡 %d | %s", currentLevel, biomeStr);
    DrawTextUI(buf, 10, 10, RGB(255, 255, 255), 18);

    // 金币
    sprintf(buf, "Gold: %d", gold);
    DrawTextUI(buf, 10, 35, RGB(255, 215, 0), 18);

    // Boss 房间清理后提示进入下一关
    if (currentRoom && currentRoom->type == RoomType::BOSS && currentRoom->isCleared) {
        DrawTextUI("Press Enter to proceed to next floor",
                   WINDOW_WIDTH / 2 - 140, WINDOW_HEIGHT - 30, RGB(255, 255, 100), 16);
    }

    if (!player || player->IsDead()) return;

    // HP / 护盾 / MP 条
    RenderHPBar(10, 58, player->GetHP(), player->GetMaxHP(), RGB(255, 40, 40), RGB(80, 20, 20));
    RenderShieldBar(10, 76, player->GetShield(), player->GetMaxShield());
    RenderMPBar(10, 90, player->GetMP(), player->GetMaxMP());

    // 武器槽（最多 2 个）
    for (int i = 0; i < 2; ++i) {
        Weapon* w = player->GetWeapon(i);
        bool active = (w == player->GetCurrentWeapon());
        RenderWeaponSlot(WINDOW_WIDTH - 140.0f + i * 60.0f, WINDOW_HEIGHT - 60.0f, w, active, i);
    }

    // 技能图标
    float cdRem = player->GetSkillCooldownRemaining();
    float cdMax = player->GetSkillCooldown();
    RenderSkillIcon(10, WINDOW_HEIGHT - 70.0f, cdRem, cdMax);
}

void UIManager::RenderHPBar(float x, float y, int current, int max, COLORREF fill, COLORREF bg) {
    int w = 200, h = 16;
    setfillcolor(bg);
    solidrectangle((int)x, (int)y, (int)x + w, (int)y + h);
    float ratio = (float)current / max;
    if (ratio < 0) ratio = 0;
    setfillcolor(fill);
    solidrectangle((int)x, (int)y, (int)x + (int)(w * ratio), (int)y + h);
    setlinecolor(COLOR_BORDER);
    rectangle((int)x, (int)y, (int)x + w, (int)y + h);
    char buf[32];
    sprintf(buf, "HP %d/%d", current, max);
    DrawTextUI(buf, (int)x + 4, (int)y + 1, RGB(255, 255, 255), 12);
}

void UIManager::RenderShieldBar(float x, float y, int current, int max) {
    if (max <= 0) return;
    int w = 200, h = 10;
    setfillcolor(RGB(20, 40, 60));
    solidrectangle((int)x, (int)y, (int)x + w, (int)y + h);
    float ratio = (float)current / max;
    if (ratio < 0) ratio = 0;
    setfillcolor(RGB(60, 180, 220));
    solidrectangle((int)x, (int)y, (int)x + (int)(w * ratio), (int)y + h);
    setlinecolor(COLOR_BORDER);
    rectangle((int)x, (int)y, (int)x + w, (int)y + h);
    char buf[32];
    sprintf(buf, "Shield %d/%d", current, max);
    DrawTextUI(buf, (int)x + 4, (int)y + 1, RGB(255, 255, 255), 10);
}

void UIManager::RenderMPBar(float x, float y, int current, int max) {
    int w = 200, h = 12;
    setfillcolor(RGB(20, 30, 80));
    solidrectangle((int)x, (int)y, (int)x + w, (int)y + h);
    float ratio = (float)current / max;
    if (ratio < 0) ratio = 0;
    setfillcolor(COLOR_MP_BAR);
    solidrectangle((int)x, (int)y, (int)x + (int)(w * ratio), (int)y + h);
    setlinecolor(COLOR_BORDER);
    rectangle((int)x, (int)y, (int)x + w, (int)y + h);
    char buf[32];
    sprintf(buf, "MP %d/%d", current, max);
    DrawTextUI(buf, (int)x + 4, (int)y + 1, RGB(255, 255, 255), 11);
}

void UIManager::RenderSkillIcon(float x, float y, float cdRem, float cdMax) {
    int size = 56;
    setfillcolor(RGB(40, 40, 60));
    solidrectangle((int)x, (int)y, (int)x + size, (int)y + size);
    setfillcolor(COLOR_BORDER);
    solidrectangle((int)x, (int)y, (int)x + size, (int)y + size);

    if (cdRem <= 0.0f) {
        DrawTextUI("SKILL", (int)x + 8, (int)y + 20, RGB(255, 255, 0), 14);
    } else {
        float f = cdRem / cdMax;
        setfillcolor(RGB(0, 0, 0));
        solidrectangle((int)x, (int)y, (int)x + size, (int)y + (int)(size * f));
        char buf[8];
        sprintf(buf, "%.1f", cdRem);
        DrawTextUI(buf, (int)x + 14, (int)y + 20, RGB(200, 200, 200), 14);
    }
}

void UIManager::RenderWeaponSlot(float x, float y, Weapon* weapon, bool active, int slot) {
    int size = 50;
    COLORREF bg = active ? RGB(60, 60, 100) : RGB(30, 30, 40);
    setfillcolor(bg);
    solidrectangle((int)x, (int)y, (int)x + size, (int)y + size);
    COLORREF border = active ? RGB(255, 255, 100) : COLOR_BORDER;
    setfillcolor(border);
    solidrectangle((int)x, (int)y, (int)x + size, (int)y + size);

    if (weapon) {
        DrawString_Safe((int)x + 4, (int)y + 10, weapon->GetName(), RGB(255, 255, 255));
    } else {
        DrawString_Safe((int)x + 12, (int)y + 16, "--", RGB(100, 100, 100));
    }

    char slotLabel[4];
    sprintf(slotLabel, "%d", slot + 1);
    DrawTextUI(slotLabel, (int)x + size - 16, (int)y + 2, RGB(180, 180, 180), 12);
}

// ============================================================
// 暂停菜单
// ============================================================
void UIManager::RenderPauseMenu() {
    DrawPanel(AABB(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f, WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f),
              RGB(10, 10, 30), COLOR_BORDER);

    DrawTextCenteredUI("游戏暂停", WINDOW_WIDTH / 2, 280, RGB(255, 255, 255), 36);
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
    cleardevice();
    DrawTextCenteredUI("选择一项天赋", WINDOW_WIDTH / 2, 80, RGB(255, 215, 0), 36);

    for (int i = 0; i < (int)talents.size() && i < 3; ++i) {
        BuffData* b = talents[i];
        if (!b) continue;
        float cy = 200.0f + i * 140.0f;
        AABB box(WINDOW_WIDTH / 2.0f, cy, 250.0f, 50.0f);
        DrawPanel(box, RGB(20, 30, 50), COLOR_BORDER);
        DrawTextCenteredUI(b->name.c_str(), WINDOW_WIDTH / 2, (int)cy - 10, RGB(255, 255, 100), 22);
        DrawTextCenteredUI(b->description.c_str(), WINDOW_WIDTH / 2, (int)cy + 18, RGB(200, 200, 200), 16);
    }
}

int UIManager::HandleTalentSelectionInput(int mouseX, int mouseY, bool mouseClick) {
    if (!mouseClick) return -1;
    for (int i = 0; i < 3; ++i) {
        float cy = 200.0f + i * 140.0f;
        AABB box(WINDOW_WIDTH / 2.0f, cy, 250.0f, 50.0f);
        if (IsMouseInRect(mouseX, mouseY, box)) return i;
    }
    return -1;
}

// ============================================================
// 商店
// ============================================================
void UIManager::RenderShop(const std::vector<ShopItemData>& items, int playerGold, float discount) {
    cleardevice();
    DrawTextCenteredUI("商店", WINDOW_WIDTH / 2, 60, RGB(255, 215, 0), 36);
    char buf[64];
    sprintf(buf, "Gold: %d", playerGold);
    DrawTextUI(buf, 20, 100, RGB(255, 215, 0), 20);

    int displayIdx = 0;
    for (int i = 0; i < (int)items.size(); ++i) {
        if (items[i].isSold) continue;  // 已售物品不显示
        float cy = 200.0f + displayIdx * 120.0f;
        AABB box(WINDOW_WIDTH / 2.0f, cy, 250.0f, 45.0f);
        DrawPanel(box, RGB(20, 30, 50), items[i].isHovered ? RGB(255, 255, 100) : COLOR_BORDER);
        DrawTextCenteredUI(items[i].name.c_str(), WINDOW_WIDTH / 2, (int)cy - 10, RGB(255, 255, 255), 20);
        sprintf(buf, "%d G", (int)(items[i].price * discount));
        DrawTextCenteredUI(buf, WINDOW_WIDTH / 2, (int)cy + 14, items[i].price <= playerGold ? RGB(100, 255, 100) : RGB(255, 100, 100), 16);
        displayIdx++;
    }
    if (displayIdx == 0) {
        DrawTextCenteredUI("商品已售罄", WINDOW_WIDTH / 2, 300, RGB(150, 150, 150), 24);
    }

    DrawTextCenteredUI("按 ESC 离开商店", WINDOW_WIDTH / 2, 600, RGB(150, 150, 150), 16);
}

int UIManager::HandleShopInput(int mouseX, int mouseY, bool mouseClick) {
    if (!mouseClick) return -1;
    int displayIdx = 0;
    for (int i = 0; i < (int)m_shopItems.size(); ++i) {
        if (m_shopItems[i].isSold) continue;  // 跳过已售
        float cy = 200.0f + displayIdx * 120.0f;
        AABB box(WINDOW_WIDTH / 2.0f, cy, 250.0f, 45.0f);
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

    DrawTextCenteredUI("武器槽已满，请选择", WINDOW_WIDTH / 2, 80, RGB(255, 215, 0), 36);

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
            label = "新武器";
            weaponName = pending->GetName();
            bgColor = RGB(30, 20, 50);
            borderColor = RGB(220, 160, 255);
        }

        DrawPanel(box, bgColor, borderColor);
        DrawTextCenteredUI(label, (int)cx, (int)(panelY + 20), RGB(200, 200, 200), 16);
        DrawTextCenteredUI(weaponName, (int)cx, (int)(panelY + 55), RGB(255, 255, 255), 18);

        if (i == 2) {
            DrawTextCenteredUI("点击此处拾取", (int)cx, (int)(panelY + 90), RGB(200, 200, 200), 14);
        } else {
            DrawTextCenteredUI("点击替换此槽位", (int)cx, (int)(panelY + 90), RGB(200, 200, 200), 14);
        }
    }

    DrawTextCenteredUI("ESC - 放弃拾取", WINDOW_WIDTH / 2, WINDOW_HEIGHT - 50, RGB(150, 150, 150), 16);
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
    DrawTextCenteredUI("奖励房间", WINDOW_WIDTH / 2, 280, RGB(255, 215, 0), 36);
    DrawTextCenteredUI("打开宝箱获取奖励！", WINDOW_WIDTH / 2, 340, RGB(255, 255, 255), 22);
}

void UIManager::RenderGameOver(bool victory, int finalLevel, int totalGold) {
    cleardevice();
    const char* title = victory ? "胜利！" : "游戏结束";
    COLORREF tc = victory ? RGB(255, 215, 0) : RGB(255, 60, 60);
    DrawTextCenteredUI(title, WINDOW_WIDTH / 2, 250, tc, 48);
    char buf[64];
    sprintf(buf, "到达关卡: %d", finalLevel);
    DrawTextCenteredUI(buf, WINDOW_WIDTH / 2, 320, RGB(255, 255, 255), 24);
    sprintf(buf, "获得金币: %d", totalGold);
    DrawTextCenteredUI(buf, WINDOW_WIDTH / 2, 360, RGB(255, 215, 0), 24);
    DrawTextCenteredUI("点击返回主菜单", WINDOW_WIDTH / 2, 440, RGB(180, 180, 180), 18);
}

// ============================================================
// 通用绘制
// ============================================================
void UIManager::DrawTextUI(const char* text, int x, int y, COLORREF color, int fontSize) {
    settextcolor(color);
    settextstyle(fontSize, 0, _T("Consolas"));
    outtextxy(x, y, text);
}

void UIManager::DrawTextCenteredUI(const char* text, int centerX, int y, COLORREF color, int fontSize) {
    settextcolor(color);
    settextstyle(fontSize, 0, _T("Consolas"));
    int w = textwidth(text);
    outtextxy(centerX - w / 2, y, text);
}

void UIManager::DrawRectUI(const AABB& rect, COLORREF fillColor, COLORREF borderColor, int borderWidth) {
    setfillcolor(fillColor);
    solidrectangle((int)rect.Left(), (int)rect.Top(), (int)rect.Right(), (int)rect.Bottom());
    if (borderWidth > 0) {
        setfillcolor(borderColor);
        solidrectangle((int)rect.Left(), (int)rect.Top(), (int)rect.Right(), (int)rect.Bottom());
        // Just draw filling: fill inside again (EasyX doesn't have stroke rect easily, simplify)
        setfillcolor(fillColor);
        solidrectangle((int)(rect.Left() + borderWidth), (int)(rect.Top() + borderWidth),
                       (int)(rect.Right() - borderWidth), (int)(rect.Bottom() - borderWidth));
    }
}

void UIManager::DrawButton(const UIButton& btn) {
    COLORREF bg = !btn.isEnabled ? COLOR_BUTTON_DISABLED :
                  btn.isHovered ? COLOR_BUTTON_HOVER : COLOR_BUTTON_NORMAL;
    DrawRectUI(btn.bounds, bg, COLOR_BORDER, 2);
    DrawTextCenteredUI(btn.text.c_str(), (int)btn.bounds.x, (int)btn.bounds.y - 8,
                     btn.isEnabled ? RGB(255, 255, 255) : RGB(120, 120, 120), 18);
}

void UIManager::DrawPanel(const AABB& rect, COLORREF bgColor, COLORREF borderColor, int alpha) {
    (void)alpha;
    setfillcolor(bgColor);
    solidrectangle((int)rect.Left(), (int)rect.Top(), (int)rect.Right(), (int)rect.Bottom());
    setfillcolor(borderColor);
    solidrectangle((int)rect.Left(), (int)rect.Top(), (int)rect.Right(), (int)rect.Bottom());
    setfillcolor(bgColor);
    solidrectangle((int)rect.Left() + 2, (int)rect.Top() + 2, (int)rect.Right() - 2, (int)rect.Bottom() - 2);
}

void UIManager::RenderTransitionFade(float progress) {
    int alpha = (int)(progress * 255);
    setfillcolor(RGB(0, 0, 0));
    // Simple full-screen fill (EasyX doesn't support alpha easily)
    if (alpha > 200) {
        solidrectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
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
    settextcolor(color);
    settextstyle(12, 0, _T("Consolas"));
    outtextxy(x, y, str);
}
