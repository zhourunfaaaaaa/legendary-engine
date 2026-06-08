#pragma once

#include "../Core/Common.h"
#include <windows.h>
#include <vector>
#include <string>
#include <functional>

class Player;
class Boss;
class BuffManager;
struct BuffData;
struct RoomData;

// ============================================================
// UIButton - 简单的矩形按钮数据
// ============================================================
struct UIButton {
    AABB        bounds;
    std::string text;
    bool        isHovered;
    bool        isEnabled;
    int         id;

    UIButton() : isHovered(false), isEnabled(true), id(-1) {}
};

// ============================================================
// TalentOption - 天赋选项数据
// ============================================================
struct TalentOption {
    BuffType    buffType;
    std::string name;
    std::string description;
    AABB        bounds;
    bool        isHovered;
    bool        isSelected;

    TalentOption() : buffType(BuffType::HP_BOOST), isHovered(false), isSelected(false) {}
};

// ============================================================
// ShopItem - 商店商品数据
// ============================================================
struct ShopItemData {
    ShopItemType itemType;
    std::string  name;
    int          price;
    int          value;
    WeaponType   weaponType;
    AABB         bounds;
    bool         isHovered;
    bool         isSold;

    ShopItemData() : itemType(ShopItemType::HP_POTION), price(0), value(0),
                     weaponType(WeaponType::ASSAULT_RIFLE), isHovered(false), isSold(false) {}
};

// ============================================================
// UIManager - UI 管理器
// ============================================================
class UIManager {
public:
    UIManager();
    ~UIManager() = default;

    void Init(HWND hWnd);

    // === 主菜单 ===
    void RenderMainMenu();
    int  HandleMainMenuInput(int mouseX, int mouseY, bool mouseClick);

    // === 角色选择 ===
    void RenderCharacterSelect();
    int  HandleCharacterSelectInput(int mouseX, int mouseY, bool mouseClick);

    // === HUD ===
    void RenderHUD(Player* player, const BuffManager& buffMgr,
                   int currentLevel, BiomeType biome, int gold,
                   const RoomData* currentRoom, int activeEnemies);
    void RenderMiniMap(const class MapGenerator* map);
    void RenderBossBar(const Boss* boss);
    void RenderHPBar(float x, float y, int current, int max, COLORREF fillColor, COLORREF bgColor);
    void RenderShieldBar(float x, float y, int current, int max);
    void RenderMPBar(float x, float y, int current, int max);
    void RenderSkillIcon(float x, float y, float cooldownRemaining, float cooldownMax);
    void RenderWeaponSlot(float x, float y, Weapon* weapon, bool isActiveSlot, int slotIndex);

    // === 暂停菜单 ===
    void RenderPauseMenu();
    int  HandlePauseMenuInput(int mouseX, int mouseY, bool mouseClick);

    // === 天赋选择 ===
    void RenderTalentSelection(const std::vector<BuffData*>& talents,
                               const BuffManager& buffMgr);
    int  HandleTalentSelectionInput(int mouseX, int mouseY, bool mouseClick);

    // === 商店 ===
    void RenderShop(const std::vector<ShopItemData>& items, int playerGold, float discount);
    int  HandleShopInput(int mouseX, int mouseY, bool mouseClick);

    // === 奖励房间 ===
    void RenderRewardRoom();

    // === 结算 ===
    void RenderGameOver(bool victory, int finalLevel, int totalGold);

    // === 武器选择（武器槽满时替换） ===
    void RenderWeaponSelection(Player* player);
    int  HandleWeaponSelectInput(int mouseX, int mouseY, Player* player, bool mouseClick);

    // === 通用绘制函数 (named with UI suffix to avoid Windows macro conflicts) ===
    void DrawTextUI(const char* text, int x, int y, COLORREF color = 0xFFFFFF, int fontSize = 20);
    void DrawTextCenteredUI(const char* text, int centerX, int y, COLORREF color = 0xFFFFFF, int fontSize = 20);
    void DrawRectUI(const AABB& rect, COLORREF fillColor, COLORREF borderColor, int borderWidth = 1);
    void DrawButton(const UIButton& btn);
    void DrawPanel(const AABB& rect, COLORREF bgColor, COLORREF borderColor, int alpha = 255);

    // === 过渡效果 ===
    void RenderTransitionFade(float progress);
    void RenderScreenShake(int intensity);

    // === 弹窗 ===
    void ShowMessageBox(const char* title, const char* message);

    // === 工具 ===
    bool IsMouseInRect(int mouseX, int mouseY, const AABB& rect);
    void GetMousePos(int& outX, int& outY) const;
    void SetDefaultFont(int height, const char* fontName = "Microsoft YaHei UI");

    // 设置/获取界面选中的天赋选项
    void SetTalentOptions(const std::vector<TalentOption>& options) { m_talentOptions = options; }
    const std::vector<TalentOption>& GetTalentOptions() const { return m_talentOptions; }

    // 商店数据
    void SetShopItems(const std::vector<ShopItemData>& items) { m_shopItems = items; }
    std::vector<ShopItemData>& GetShopItemsMutable() { return m_shopItems; }

    // 商店持久化（每层只生成一次）
    bool IsShopGeneratedThisLevel() const { return m_shopGeneratedThisLevel; }
    void SetShopGeneratedThisLevel(bool v) { m_shopGeneratedThisLevel = v; }
    std::vector<ShopItemData>& GetCachedShopItems() { return m_cachedShopItems; }
    void SetCachedShopItems(const std::vector<ShopItemData>& items) { m_cachedShopItems = items; }

private:
    void DrawString_Safe(int x, int y, const char* str, COLORREF color);

    // 颜色常量
    static const COLORREF COLOR_HP_BAR    = 0x00FF4040;
    static const COLORREF COLOR_MP_BAR    = 0x0040A0FF;
    static const COLORREF COLOR_GOLD      = 0x00FFD700;
    static const COLORREF COLOR_BG_DARK   = 0x001A1A2E;
    static const COLORREF COLOR_BG_PANEL  = 0x0016223E;
    static const COLORREF COLOR_BORDER    = 0x00506080;
    static const COLORREF COLOR_TEXT_WHITE = 0x00FFFFFF;
    static const COLORREF COLOR_BUTTON_NORMAL = 0x00305070;
    static const COLORREF COLOR_BUTTON_HOVER  = 0x004070A0;
    static const COLORREF COLOR_BUTTON_DISABLED = 0x00303030;

    HWND        m_hWnd;
    int         m_screenWidth;
    int         m_screenHeight;

    std::vector<UIButton> m_menuButtons;
    std::vector<UIButton> m_characterButtons;
    std::vector<UIButton> m_pauseButtons;
    std::vector<TalentOption> m_talentOptions;
    std::vector<ShopItemData> m_shopItems;
    std::vector<ShopItemData> m_cachedShopItems;
    bool m_shopGeneratedThisLevel = false;
};

// Undef Windows macro that conflicts with method names (if other files include us after windows.h)
#ifdef DrawText
#undef DrawText
#endif
