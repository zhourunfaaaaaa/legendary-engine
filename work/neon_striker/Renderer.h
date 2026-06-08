#pragma once

#include <vector>
#include "Common.h"

class Player;
class Enemy;
class Boss;
class PlayerBullet;
class EnemyBullet;
class ParticleSystem;

// ============================================================
// Renderer - 所有绘制逻辑（单例）
// ============================================================
class Renderer {
public:
    static Renderer& GetInstance() {
        static Renderer instance;
        return instance;
    }

    void Init();
    void Shutdown();

    // 主渲染入口
    void Render();

    // 背景
    void UpdateBackground(float dt);
    void DrawBackground(float dt);

    // 各实体绘制
    void DrawPlayer(Player* player);
    void DrawEnemy(Enemy* enemy);
    void DrawBoss(Boss* boss);
    void DrawPlayerBullet(PlayerBullet* bullet);
    void DrawEnemyBullet(EnemyBullet* bullet);
    void DrawLaser(Player* player);
    void DrawParticles(ParticleSystem* ps);

    // 后处理
    void DrawPostFilters();

    // HUD
    void DrawHUD();

    // BOSS血条
    void DrawBossHPBar();
    void ShowBossHPBar(bool show);

    // 菜单
    void DrawMainMenu();
    void DrawGameOverScreen();
    void DrawVictoryScreen();
    void DrawDefeatScreen();

    // 入场动画
    void UpdateIntroAnimation(float dt);
    void DrawIntroAnimation();

    // 暂停覆层
    void DrawPauseOverlay();

    // 阶段切换动画
    void DrawTransition();

    // 屏幕特效
    void DrawScreenFlash();
    void DrawHealFlash();
    void DrawKillFlash();

    // 通用
    void DrawRect(float x1, float y1, float x2, float y2, COLORREF color, bool filled = true);
    void DrawCircle(float x, float y, float r, COLORREF color, bool filled = true, int thickness = 1);
    void DrawLine(float x1, float y1, float x2, float y2, COLORREF color, int thickness = 1);

    // EasyX 无 alpha 通道，用预乘 alpha 颜色替代
    static COLORREF AlphaColor(COLORREF base, float alpha);
    static COLORREF LerpColor(COLORREF a, COLORREF b, float t);

    // 按钮
    void DrawMenuButton(const char* text, int x, int y, int w, int h);

    // 背景网格粒子
    struct BgParticle {
        float x, y;
        float vy;
        float size;
        float alpha;
    };

private:
    Renderer() = default;

    std::vector<BgParticle> m_bgParticles;
    bool m_bossHpVisible = false;

    // 字体
    LOGFONT m_fontSmall;
    LOGFONT m_fontMedium;
    LOGFONT m_fontLarge;
    LOGFONT m_fontHuge;

    void InitBgParticles();
};
