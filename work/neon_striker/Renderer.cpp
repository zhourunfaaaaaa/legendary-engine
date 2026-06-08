#include "Renderer.h"
#include "Player.h"
#include "Enemy.h"
#include "Boss.h"
#include "Bullet.h"
#include "Particle.h"
#include "GameManager.h"
#include "Input.h"
#include <graphics.h>
#include <cmath>
#include <cstdio>

// ============================================================
// Renderer 工具函数
// ============================================================
COLORREF Renderer::AlphaColor(COLORREF base, float alpha) {
    if (alpha >= 1.0f) return base;
    if (alpha <= 0.0f) return RGB(0, 0, 0);
    int r = GetRValue(base), g = GetGValue(base), b = GetBValue(base);
    // EasyX 无 alpha，用暗色混合模拟
    int bgR = 5, bgG = 5, bgB = 15;
    r = (int)(r * alpha + bgR * (1.0f - alpha));
    g = (int)(g * alpha + bgG * (1.0f - alpha));
    b = (int)(b * alpha + bgB * (1.0f - alpha));
    return RGB(r, g, b);
}

COLORREF Renderer::LerpColor(COLORREF a, COLORREF b, float t) {
    int r = (int)(GetRValue(a) + (GetRValue(b) - GetRValue(a)) * t);
    int g = (int)(GetGValue(a) + (GetGValue(b) - GetGValue(a)) * t);
    int bl = (int)(GetBValue(a) + (GetBValue(b) - GetBValue(a)) * t);
    return RGB(r, g, bl);
}

// ============================================================
// Renderer 初始化
// ============================================================
void Renderer::Init() {
    InitBgParticles();

    // 预设字体
    settextstyle(12, 0, "System");
}

void Renderer::Shutdown() {
    m_bgParticles.clear();
}

void Renderer::InitBgParticles() {
    m_bgParticles.clear();
    for (int i = 0; i < 80; i++) {
        BgParticle p;
        p.x = RandomFloat(0.0f, (float)WINDOW_WIDTH);
        p.y = RandomFloat(0.0f, (float)WINDOW_HEIGHT);
        p.vy = 0.3f + RandomFloat(0.0f, 0.8f);
        p.size = 0.5f + RandomFloat(0.0f, 1.5f);
        p.alpha = 0.2f + RandomFloat(0.0f, 0.4f);
        m_bgParticles.push_back(p);
    }
}

// ============================================================
// 主渲染入口 — 仅用于 INTRO 和 PLAYING 状态
// MENU/GAMEOVER/VICTORY/DEFEAT 由 ProcessFrame 直接调用
// ============================================================
void Renderer::Render() {
    cleardevice();

    GameState state = GameManager::GetInstance().GetState();

    if (state == GameState::INTRO) {
        DrawIntroAnimation();
        FlushBatchDraw();
        return;
    }

    if (state != GameState::PLAYING && state != GameState::PAUSED) { FlushBatchDraw(); return; }

    // ── PLAYING 渲染 ──
    GameManager& gm = GameManager::GetInstance();
    Player* player = gm.GetPlayer();
    if (!player) { FlushBatchDraw(); return; }

    // 阶段切换白屏
    DrawTransition();

    // 背景
    DrawBackground(gm.GetDeltaTime());

    // 粒子
    DrawParticles(gm.GetParticles());

    // 玩家子弹
    for (auto* b : gm.GetPlayerBullets()) {
        if (b->IsAlive()) DrawPlayerBullet(b);
    }

    // 敌人子弹
    for (auto* b : gm.GetEnemyBullets()) {
        if (b->IsAlive()) DrawEnemyBullet(b);
    }

    // 敌人
    for (auto* e : gm.GetEnemies()) {
        if (e->IsAlive()) DrawEnemy(e);
    }

    // BOSS
    if (gm.GetBoss() && gm.GetBoss()->IsAlive()) {
        DrawBoss(gm.GetBoss());
    }
    if (gm.GetInfiniteBoss() && gm.GetInfiniteBoss()->IsAlive()) {
        DrawBoss(gm.GetInfiniteBoss());
    }

    // 激光
    if (gm.IsLaserActive() && player) {
        DrawLaser(player);
    }

    // 玩家
    DrawPlayer(player);

    // 后处理
    DrawPostFilters();

    // 闪光
    DrawScreenFlash();
    DrawHealFlash();
    DrawKillFlash();

    // HUD
    DrawHUD();

    // BOSS血条
    if ((gm.IsBossMode() && gm.GetBoss() && gm.GetBoss()->IsAlive()) ||
        (gm.GetInfiniteBoss() && gm.GetInfiniteBoss()->IsAlive())) {
        DrawBossHPBar();
    }

    // ★ 暂停覆层
    if (state == GameState::PAUSED) {
        DrawPauseOverlay();
    }

    FlushBatchDraw();
}

void Renderer::DrawPauseOverlay() {
    // 条纹半透明效果
    for (int y = 0; y < WINDOW_HEIGHT; y += 3) {
        setlinecolor(RGB(5, 10, 30));
        line(0, y, WINDOW_WIDTH, y);
    }
    // PAUSED 文字
    settextstyle(64, 0, "System");
    settextcolor(RGB(0, 255, 255));
    outtextxy((WINDOW_WIDTH - textwidth("PAUSED")) / 2, WINDOW_HEIGHT / 2 - 50, "PAUSED");
    settextstyle(18, 0, "System");
    settextcolor(RGB(180, 200, 220));
    outtextxy((WINDOW_WIDTH - textwidth("Press ESC to resume")) / 2, WINDOW_HEIGHT / 2 + 20, "Press ESC to resume");
}

// ============================================================
// 背景
// ============================================================
void Renderer::UpdateBackground(float dt) {
    for (auto& p : m_bgParticles) {
        p.y -= p.vy;
        if (p.y < -10.0f) {
            p.y = (float)WINDOW_HEIGHT + 10.0f;
            p.x = RandomFloat(0.0f, (float)WINDOW_WIDTH);
        }
    }
}

void Renderer::DrawBackground(float dt) {
    // 网格线
    setlinecolor(RGB(0, 100, 150));
    for (int x = 0; x < WINDOW_WIDTH; x += 60) {
        line(x, 0, x, WINDOW_HEIGHT);
    }
    for (int y = 0; y < WINDOW_HEIGHT; y += 60) {
        line(0, y, WINDOW_WIDTH, y);
    }

    // 浮动粒子
    for (const auto& p : m_bgParticles) {
        COLORREF c = AlphaColor(RGB(0, 255, 255), p.alpha);
        setfillcolor(c);
        solidcircle((int)p.x, (int)p.y, (int)(p.size * 2));
    }

    // 波次/得分元素 (波浪线装饰)
    GameManager& gm = GameManager::GetInstance();
    if (gm.GetState() == GameState::PLAYING) {
        float gt = gm.GetGameTime();
        setlinecolor(RGB(0, 80, 120));
        for (int i = 0; i < 8; i++) {
            int baseY = WINDOW_HEIGHT / 2 + (i - 4) * 80;
            baseY += (int)(std::sin(gt * 2.0f + i) * 30.0f);
            line(0, baseY, WINDOW_WIDTH, baseY);
        }
    }
}

// ============================================================
// 玩家绘制
// ============================================================
void Renderer::DrawPlayer(Player* player) {
    GameManager& gm = GameManager::GetInstance();

    // 无敌闪烁
    if (player->GetInvincible() > 0.0f && ((int)(player->GetInvincible() * 20) % 2 == 0)) return;

    float rx = player->GetRecoilX();
    float ry = player->GetRecoilY();
    float cx = player->GetX() + rx;
    float cy = player->GetY() + ry;
    float size = player->GetSize();

    // 阶段3光环
    if (gm.GetCurrentStage() >= 2) {
        float auraAlpha = 0.15f + std::sin(gm.GetGameTime() * 4.0f) * 0.05f;
        for (int i = 3; i >= 0; i--) {
            float r = 24.0f + i * 6.0f;
            float a = auraAlpha * (1.0f - i * 0.25f);
            setlinecolor(AlphaColor(RGB(0, 255, 255), a));
            circle((int)cx, (int)cy, (int)r);
        }
    }

    // 引擎尾焰
    float flameLen = 15.0f + std::sin(player->GetEngineFlame() * 3.0f) * 8.0f;
    COLORREF flameColors[] = { RGB(255,255,255), RGB(0,255,255), RGB(255,0,255) };
    for (int i = 2; i >= 0; i--) {
        float len = flameLen * (1.0f - i * 0.3f);
        float alpha = 1.0f - i * 0.3f;
        setfillcolor(AlphaColor(flameColors[i], alpha));
        int pts[6] = {
            (int)(cx - 8), (int)(cy + 10),
            (int)(cx + 8), (int)(cy + 10),
            (int)(cx), (int)(cy + 10 + len)
        };
        solidpolygon((POINT*)pts, 3);
    }

    // 小尾焰
    float sFlameLen = 8.0f + std::sin(player->GetEngineFlame() * 5.0f + 1.0f) * 5.0f;
    setfillcolor(AlphaColor(RGB(0, 255, 255), 0.7f));
    {
        int pts[6] = {
            (int)(cx - 4), (int)(cy + 6),
            (int)(cx + 4), (int)(cy + 6),
            (int)(cx), (int)(cy + 6 + sFlameLen)
        };
        solidpolygon((POINT*)pts, 3);
    }

    // ── 机身: 赛博朋克三角 (降级几何绘制) ──
    // 阴影
    setfillcolor(RGB(0, 60, 100));
    int shadowPts[8] = {
        (int)(cx - size), (int)(cy + size * 0.5f),
        (int)(cx + size), (int)(cy + size * 0.5f),
        (int)(cx + size * 0.4f), (int)(cy - size * 0.3f),
        (int)(cx - size * 0.4f), (int)(cy - size * 0.3f)
    };
    solidpolygon((POINT*)shadowPts, 4);

    // 主体
    setfillcolor(RGB(0, 80, 160));
    setlinecolor(RGB(0, 255, 255));
    int bodyPts[8] = {
        (int)(cx - size * 0.9f), (int)(cy + size * 0.3f),
        (int)(cx + size * 0.9f), (int)(cy + size * 0.3f),
        (int)(cx + size * 0.3f), (int)(cy - size * 0.4f),
        (int)(cx - size * 0.3f), (int)(cy - size * 0.4f)
    };
    solidpolygon((POINT*)bodyPts, 4);
    polygon((POINT*)bodyPts, 4);

    // 发光核心 (青色三角，较小)
    setfillcolor(RGB(0, 255, 255));
    int corePts[6] = {
        (int)(cx - size * 0.4f), (int)(cy),
        (int)(cx + size * 0.4f), (int)(cy),
        (int)(cx), (int)(cy - size * 0.6f)
    };
    solidpolygon((POINT*)corePts, 3);

    // 白色中心点
    setfillcolor(RGB(255, 255, 255));
    solidcircle((int)cx, (int)(cy - size * 0.1f), (int)(size * 0.2f));

    // 阶段发光
    if (gm.GetCurrentStage() >= 1) {
        float glow = player->GetGlowTimer();
        if (glow > 0.0f || gm.GetTransTimer() > 0.0f) {
            glow = (std::max)(glow, gm.GetTransTimer() / 1.2f);
            COLORREF gc = (gm.GetCurrentStage() >= 2) ? RGB(255, 0, 255) : RGB(0, 255, 255);
            setlinecolor(AlphaColor(gc, glow * 0.5f));
            circle((int)cx, (int)cy, (int)(size * 1.5f + glow * 20.0f));
        }
    }
}

// ============================================================
// 敌人绘制
// ============================================================
void Renderer::DrawEnemy(Enemy* enemy) {
    float x = enemy->GetX(), y = enemy->GetY();
    float size = enemy->GetSize();
    COLORREF color = enemy->GetColor();

    // 闪烁
    bool flash = enemy->GetFlashTimer() > 0.0f && std::sin(enemy->GetFlashTimer() * 40.0f) > 0.0f;
    COLORREF drawColor = flash ? RGB(255, 255, 255) : color;

    GameManager& gm = GameManager::GetInstance();

    switch (enemy->GetType()) {
        case EnemyType::SMALL:
            setfillcolor(drawColor);
            setlinecolor(RGB(255, 255, 255));
            // 小菱形
            {
                int pts[8] = { (int)x, (int)(y-size), (int)(x+size*0.6f), (int)y, (int)x, (int)(y+size*0.8f), (int)(x-size*0.6f), (int)y };
                solidpolygon((POINT*)pts, 4);
                polygon((POINT*)pts, 4);
            }
            setfillcolor(RGB(255, 255, 255));
            solidcircle((int)x, (int)y, (int)(size * 0.2f));
            break;

        case EnemyType::NORMAL: {
            setfillcolor(RGB(180, 80, 0));
            setlinecolor(RGB(255, 200, 0));
            circle((int)x, (int)y, (int)size);
            setfillcolor(drawColor);
            circle((int)x, (int)y, (int)(size * 0.55f));
            setfillcolor(RGB(255, 255, 255));
            solidcircle((int)x, (int)y, (int)(size * 0.2f));
            break;
        }

        case EnemyType::LARGE: {
            COLORREF dark = RGB(200, 20, 20);
            setfillcolor(dark);
            setlinecolor(RGB(255, 100, 100));
            int pts[12];
            for (int i = 0; i < 6; i++) {
                float a = PI * 2.0f / 6.0f * i - PI / 2.0f;
                pts[i*2] = (int)(x + std::cos(a) * size);
                pts[i*2+1] = (int)(y + std::sin(a) * size);
            }
            solidpolygon((POINT*)pts, 6);
            polygon((POINT*)pts, 6);
            setfillcolor(RGB(255, 80, 80));
            solidcircle((int)x, (int)y, (int)(size * 0.4f));
            setfillcolor(RGB(255, 255, 255));
            solidcircle((int)x, (int)y, (int)(size * 0.15f));
            break;
        }

        case EnemyType::FAST:
            setfillcolor(drawColor);
            setlinecolor(RGB(255, 255, 255));
            // 小三角
            {
                int pts[6] = { (int)x, (int)(y-size), (int)(x+size*0.5f), (int)(y+size*0.4f), (int)(x-size*0.5f), (int)(y+size*0.4f) };
                solidpolygon((POINT*)pts, 3);
            }
            break;

        case EnemyType::SHIELD: {
            if (enemy->IsShieldActive()) {
                float pulse = 0.5f + std::sin(gm.GetGameTime() * 3.0f) * 0.3f;
                setlinecolor(RGB(60, 180, 255));
                circle((int)x, (int)y, (int)(size * 1.2f));
                circle((int)x, (int)y, (int)(size * 1.15f));
            }
            setfillcolor(RGB(30, 80, 150));
            setlinecolor(RGB(80, 180, 255));
            circle((int)x, (int)y, (int)size);
            setfillcolor(RGB(100, 200, 255));
            solidcircle((int)x, (int)y, (int)(size * 0.5f));
            setfillcolor(RGB(255, 255, 255));
            solidcircle((int)x, (int)y, (int)(size * 0.2f));
            break;
        }

        case EnemyType::SPLITTER:
            setfillcolor(RGB(200, 130, 30));
            setlinecolor(RGB(255, 200, 80));
            circle((int)x, (int)y, (int)size);
            setfillcolor(RGB(255, 180, 60));
            solidcircle((int)x, (int)y, (int)(size * 0.5f));
            setlinecolor(RGB(255, 255, 0));
            circle((int)x, (int)y, (int)(size * 0.35f));
            break;

        case EnemyType::SNIPER:
            setfillcolor(drawColor);
            setlinecolor(RGB(255, 255, 255));
            // 菱形
            {
                int pts[8] = { (int)x, (int)(y-size*0.8f), (int)(x+size*0.5f), (int)y, (int)x, (int)(y+size*0.8f), (int)(x-size*0.5f), (int)y };
                solidpolygon((POINT*)pts, 4);
                polygon((POINT*)pts, 4);
            }
            // 预警线
            if (enemy->HasWarningLine()) {
                float a = enemy->GetSniperAngle();
                float alpha = enemy->GetWarnTimer() / 0.5f;
                setlinecolor(AlphaColor(RGB(255, 0, 136), alpha));
                float endX = x + std::cos(a) * 300.0f;
                float endY = y + std::sin(a) * 300.0f;
                line((int)x, (int)y, (int)endX, (int)endY);
            }
            break;
    }
}

// ============================================================
// BOSS绘制
// ============================================================
void Renderer::DrawBoss(Boss* boss) {
    float x = boss->GetX(), y = boss->GetY();
    float size = boss->GetSize();
    bool flash = boss->GetFlashTimer() > 0.0f && std::sin(boss->GetFlashTimer() * 40.0f) > 0.0f;

    GameManager& gm = GameManager::GetInstance();

    // 红色能量护盾
    setfillcolor(RGB(60, 5, 5));
    solidcircle((int)x, (int)y, (int)(size * 1.3f));

    // 红色发光轮廓
    setlinecolor(flash ? RGB(255, 255, 255) : RGB(255, 80, 80));
    circle((int)x, (int)(y - 10), (int)size);
    circle((int)x, (int)(y - 10), (int)(size + 2));

    // BOSS模型 - 十角星
    COLORREF fillC = flash ? RGB(220, 30, 30) : RGB(140, 15, 15);
    COLORREF lineC = flash ? RGB(255, 255, 255) : RGB(255, 130, 130);
    setfillcolor(fillC);
    setlinecolor(lineC);

    int pts[20];
    for (int i = 0; i < 10; i++) {
        float a = PI * 2.0f / 10.0f * i - PI / 2.0f;
        float r = (i % 2 == 0) ? size : size * 0.5f;
        pts[i * 2] = (int)(x + std::cos(a) * r);
        pts[i * 2 + 1] = (int)(y + std::sin(a) * r);
    }
    solidpolygon((POINT*)pts, 10);
    polygon((POINT*)pts, 10);

    // 中心红眼
    setfillcolor(RGB(255, 0, 0));
    solidcircle((int)x, (int)(y - 5), (int)(size * 0.25f));
    setfillcolor(RGB(255, 255, 255));
    solidcircle((int)x, (int)(y - 5), (int)(size * 0.1f));

    // 阶段3额外标记
    if (boss->GetBossPhase() >= 2) {
        setlinecolor(AlphaColor(RGB(255, 255, 0), 0.6f));
        circle((int)x, (int)y, (int)(size * 1.5f));
    }
}

// ============================================================
// 子弹绘制
// ============================================================
void Renderer::DrawPlayerBullet(PlayerBullet* bullet) {
    float x = bullet->GetX(), y = bullet->GetY();
    float size = bullet->GetSize();
    COLORREF color = bullet->GetColor();

    // 拖尾
    const auto& trail = bullet->GetTrail();
    for (size_t i = 0; i < trail.size(); i++) {
        float alpha = (float)i / trail.size() * 0.35f;
        float s = size * (float)i / trail.size() * 0.5f;
        setfillcolor(AlphaColor(color, alpha));
        solidcircle((int)trail[i].x, (int)trail[i].y, (int)(std::max)(1.0f, s));
    }

    // 主体菱形
    setfillcolor(color);
    setlinecolor(RGB(255, 255, 255));
    int pts[8] = {
        (int)x, (int)(y - size * 2),
        (int)(x + size * 0.6f), (int)y,
        (int)x, (int)(y + size * 0.8f),
        (int)(x - size * 0.6f), (int)y
    };
    solidpolygon((POINT*)pts, 4);
    polygon((POINT*)pts, 4);

    // 白色核心
    setfillcolor(RGB(255, 255, 255));
    solidcircle((int)x, (int)y, (int)(size * 0.35f));
}

void Renderer::DrawEnemyBullet(EnemyBullet* bullet) {
    float x = bullet->GetX(), y = bullet->GetY();
    float size = bullet->GetSize();
    COLORREF color = bullet->GetColor();
    float t = bullet->GetTime();
    float glowPulse = 0.6f + std::sin(t * 15.0f) * 0.4f;

    switch (bullet->GetBehavior()) {
        case BulletBehavior::LASER_ORB: {
            setfillcolor(RGB(255, 255, 255));
            solidcircle((int)x, (int)y, (int)(size * 2));
            setfillcolor(RGB(255, 255, 60));
            solidcircle((int)x, (int)y, (int)(size * 1.4f));
            setfillcolor(RGB(255, 160, 20));
            solidcircle((int)x, (int)y, (int)(size * 0.8f));
            if (bullet->GetLifeTimer() > 1.5f && std::sin(t * 20.0f) > 0.0f) {
                setfillcolor(RGB(255, 255, 255));
                solidcircle((int)x, (int)y, (int)(size * 0.5f));
            }
            return;
        }
        case BulletBehavior::BOUNCE: {
            setfillcolor(RGB(40, 200, 40));
            setlinecolor(RGB(100, 255, 100));
            int pts[8] = { (int)x, (int)(y-size*1.4f), (int)(x+size), (int)y, (int)x, (int)(y+size*1.4f), (int)(x-size), (int)y };
            solidpolygon((POINT*)pts, 4);
            polygon((POINT*)pts, 4);
            setfillcolor(RGB(0, 255, 0));
            solidcircle((int)x, (int)y, (int)(size * 0.5f));
            return;
        }
        default:
            break;
    }

    // 普通敌弹 — 三层提亮
    setfillcolor(RGB(255, 255, 255));
    solidcircle((int)x, (int)y, (int)(size * 0.4f));
    setfillcolor(color);
    solidcircle((int)x, (int)y, (int)(size * 0.8f));
    // 光晕: 取颜色的高亮版本
    int r = std::min(255, GetRValue(color) + 80);
    int g = std::min(255, GetGValue(color) + 80);
    int b = std::min(255, GetBValue(color) + 80);
    setfillcolor(RGB(r, g, b));
    solidcircle((int)x, (int)y, (int)(size * 1.4f));
}

// ============================================================
// 激光绘制
// ============================================================
void Renderer::DrawLaser(Player* player) {
    GameManager& gm = GameManager::GetInstance();
    if (gm.GetCurrentStage() < 2) return;

    float lx = player->GetX() + player->GetRecoilX();
    float ly = player->GetY() + player->GetRecoilY() - player->GetSize();

    float wScale = LASER_WIDTH / 30.0f;
    float halfW = LASER_WIDTH / 2.0f;

    // 多层渐变光束
    // 外层光晕
    for (int i = 0; i < 5; i++) {
        float w = halfW + i * halfW * 0.8f;
        float alpha = 0.05f - i * 0.01f;
        if (alpha <= 0.0f) break;
        setfillcolor(AlphaColor(RGB(0, 255, 255), alpha));
        int x1 = (int)(lx - w), x2 = (int)(lx + w);
        solidrectangle(x1, (int)(ly - LASER_REACH), x2, (int)ly);
    }

    // 中层
    setfillcolor(AlphaColor(RGB(0, 255, 255), 0.15f));
    int mx1 = (int)(lx - halfW * 0.6f), mx2 = (int)(lx + halfW * 0.6f);
    solidrectangle(mx1, (int)(ly - LASER_REACH), mx2, (int)ly);

    // 核心
    setfillcolor(AlphaColor(RGB(200, 255, 255), 0.5f));
    int cx1 = (int)(lx - 3), cx2 = (int)(lx + 3);
    solidrectangle(cx1, (int)(ly - LASER_REACH), cx2, (int)ly);

    // 白色热线
    setfillcolor(AlphaColor(RGB(255, 255, 255), 0.8f));
    solidrectangle((int)(lx - 1), (int)(ly - LASER_REACH), (int)(lx + 1), (int)ly);

    // 发射点光球
    float muzzleScale = (std::min)(wScale, 0.55f);
    setfillcolor(RGB(255, 255, 255));
    solidcircle((int)lx, (int)ly, (int)(6.0f * muzzleScale));
    setfillcolor(RGB(0, 255, 255));
    solidcircle((int)lx, (int)ly, (int)(4.0f * muzzleScale));
}

// ============================================================
// 粒子绘制
// ============================================================
void Renderer::DrawParticles(ParticleSystem* ps) {
    for (auto* p : ps->GetParticles()) {
        float alpha = (std::max)(0.0f, p->GetLife() / p->GetMaxLife());
        float s = p->GetSize() * (0.5f + 0.5f * alpha);
        setfillcolor(AlphaColor(p->GetColor(), alpha));
        solidcircle((int)p->GetX(), (int)p->GetY(), (int)(std::max)(1.0f, s));
    }
}

// ============================================================
// 后处理滤镜
// ============================================================
void Renderer::DrawPostFilters() {
    GameManager& gm = GameManager::GetInstance();
    if (!gm.IsFilterEnabled()) return;

    // 扫描线
    for (int y = 0; y < WINDOW_HEIGHT; y += 3) {
        setlinecolor(RGB(0, 30, 60));
        line(0, y, WINDOW_WIDTH, y);
    }

    // 边角暗角
    for (int y = 0; y < 10; y++) {
        int v = (10 - y) * 2;
        setfillcolor(RGB(v, v, v + 5));
        solidrectangle(0, y, WINDOW_WIDTH, y + 1);
        solidrectangle(0, WINDOW_HEIGHT - y - 1, WINDOW_WIDTH, WINDOW_HEIGHT - y);
    }
}

// ============================================================
// 屏幕特效
// ============================================================
void Renderer::DrawScreenFlash() {
    GameManager& gm = GameManager::GetInstance();
    float flash = gm.GetScreenFlash();
    if (flash <= 0.0f) return;
    setfillcolor(AlphaColor(RGB(255, 255, 255), flash));
    solidrectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

void Renderer::DrawHealFlash() {
    GameManager& gm = GameManager::GetInstance();
    float t = gm.GetHealFlashTimer();
    if (t <= 0.0f) return;
    setfillcolor(AlphaColor(RGB(0, 255, 50), t * 0.3f));
    solidrectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

void Renderer::DrawKillFlash() {
    GameManager& gm = GameManager::GetInstance();
    float t = gm.GetKillFlashTimer();
    if (t <= 0.0f) return;
    // 微小红闪 (忽略，EasyX难以做到像素级)
}

void Renderer::DrawTransition() {
    GameManager& gm = GameManager::GetInstance();
    float t = gm.GetTransTimer();
    if (t <= 0.0f) return;
    float alpha = t / 1.2f;
    setfillcolor(AlphaColor(RGB(255, 255, 255), alpha * 0.7f));
    solidrectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

// ============================================================
// HUD
// ============================================================
void Renderer::DrawHUD() {
    GameManager& gm = GameManager::GetInstance();
    Player* player = gm.GetPlayer();
    if (!player) return;

    // HP条 (左上)
    int hpX = 16, hpY = 16, hpW = 220, hpH = 14;
    setfillcolor(RGB(10, 5, 15));
    solidrectangle(hpX, hpY, hpX + hpW, hpY + hpH);
    setlinecolor(RGB(255, 80, 150));
    rectangle(hpX, hpY, hpX + hpW, hpY + hpH);

    float hpPct = (float)player->GetHP() / player->GetMaxHP();
    int fillW = (int)(hpW * hpPct);
    COLORREF hpColor;
    if (hpPct < 0.25f) hpColor = RGB(255, 0, 0);
    else if (hpPct < 0.5f) hpColor = RGB(255, 128, 0);
    else hpColor = RGB(0, 255, 255);
    setfillcolor(hpColor);
    solidrectangle(hpX + 1, hpY + 1, hpX + fillW, hpY + hpH - 1);

    // HP文字
    settextcolor(RGB(255, 100, 100));
    settextstyle(10, 0, "System");
    char hpText[32];
    sprintf(hpText, "HP");
    outtextxy(hpX + 6, hpY - 2, hpText);
    sprintf(hpText, "%d/%d", player->GetHP(), player->GetMaxHP());
    outtextxy(hpX + 160, hpY - 2, hpText);

    // 能量条 (HP下方)
    int eX = 16, eY = 36, eW = 220, eH = 8;
    setfillcolor(RGB(5, 10, 20));
    solidrectangle(eX, eY, eX + eW, eY + eH);
    setlinecolor(RGB(80, 255, 255));
    rectangle(eX, eY, eX + eW, eY + eH);

    float ePct = player->GetEnergy() / player->GetMaxEnergy();
    int eFillW = (int)(eW * ePct);
    setfillcolor(RGB(0, 255, 255));
    solidrectangle(eX + 1, eY + 1, eX + eFillW, eY + eH - 1);

    settextcolor(RGB(0, 255, 255));
    char eText[32];
    sprintf(eText, "ENERGY");
    outtextxy(eX + 6, eY - 2, eText);

    // 击杀面板 (右上)
    int kX = WINDOW_WIDTH - 180, kY = 12;
    setfillcolor(RGB(5, 10, 25));
    solidrectangle(kX, kY, WINDOW_WIDTH - 16, kY + 80);
    setlinecolor(RGB(80, 255, 255));
    rectangle(kX, kY, WINDOW_WIDTH - 16, kY + 80);

    settextstyle(10, 0, "System");
    settextcolor(RGB(0, 255, 255));
    outtextxy(kX + 10, kY + 4, "KILLS");

    settextstyle(48, 0, "System");
    char kText[32];
    sprintf(kText, "%d", gm.GetTotalKills());
    int tw = textwidth(kText);
    outtextxy(WINDOW_WIDTH - 30 - tw, kY + 16, kText);

    // 最高分
    settextstyle(10, 0, "System");
    settextcolor(RGB(0, 200, 100));
    char hsText[64];
    sprintf(hsText, "HI: %d", gm.GetHighScore());
    outtextxy(kX + 10, kY + 62, hsText);

    // HUD 底部 (分数/波次/武器)
    settextstyle(16, 0, "System");
    settextcolor(RGB(0, 255, 255));

    int hudY = WINDOW_HEIGHT - 50;
    char infoText[128];
    sprintf(infoText, "SCORE: %d    WAVE: %d    WEAPON: ", gm.GetScore(), gm.GetWave());
    outtextxy(30, hudY, infoText);

    WeaponType wt = player->GetWeaponType();
    const char* wn = "BEAM";
    if (wt == WeaponType::PULSE) wn = "PULSE";
    else if (wt == WeaponType::SCATTER) wn = "SCATTER";
    int infoW = textwidth(infoText);
    outtextxy(30 + infoW, hudY, (char*)wn);

    // 阶段指示 + 击杀进度
    int stage = gm.GetCurrentStage();
    int kills = gm.GetTotalKills();
    if (!gm.IsBossMode()) {
        if (stage < 2) {
            int nextStage = (stage < 1) ? STAGE2_KILLS : STAGE3_KILLS;
            settextcolor(RGB(180, 180, 100));
            char progText[32];
            sprintf(progText, "%d/%d > STAGE%d", kills, nextStage, (stage < 1 ? 2 : 3));
            outtextxy(WINDOW_WIDTH - 280, hudY, progText);
        } else {
            settextcolor(RGB(255, 0, 255));
            outtextxy(WINDOW_WIDTH - 200, hudY, "STAGE 3 MAX");
        }
    } else {
        // Boss mode: time-based stages
        float bossTime = gm.GetBossGameTimer();
        if (stage < 2) {
            float nextTime = (stage < 1) ? BOSS_STAGE2_TIME : BOSS_STAGE3_TIME;
            settextcolor(RGB(180, 180, 100));
            char progText[32];
            sprintf(progText, "%.0f/%.0fs > STAGE%d", bossTime, nextTime, (stage < 1 ? 2 : 3));
            outtextxy(WINDOW_WIDTH - 300, hudY, progText);
        }
    }

    // 连击
    if (gm.GetComboTimer() > 0.0f && gm.GetComboCount() > 5) {
        settextstyle(28, 0, "System");
        settextcolor(RGB(255, 255, 0));
        char cText[32];
        sprintf(cText, "COMBO x%d", gm.GetComboCount());
        int cw = textwidth(cText);
        outtextxy((WINDOW_WIDTH - cw) / 2, WINDOW_HEIGHT / 2 - 50, cText);
    }
}

// ============================================================
// BOSS血条
// ============================================================
void Renderer::DrawBossHPBar() {
    GameManager& gm = GameManager::GetInstance();
    Boss* boss = gm.IsBossMode() ? gm.GetBoss() : gm.GetInfiniteBoss();
    if (!boss || !boss->IsAlive()) {
        ShowBossHPBar(false);
        return;
    }

    ShowBossHPBar(true);

    int barW = 720, barH = 32;
    int barX = (WINDOW_WIDTH - barW) / 2;
    int barY = 8;

    // 背景
    setfillcolor(RGB(20, 0, 0));
    solidrectangle(barX, barY, barX + barW, barY + barH);
    setlinecolor(RGB(255, 50, 50));
    rectangle(barX, barY, barX + barW, barY + barH);

    float pct = (float)boss->GetHP() / boss->GetMaxHP();
    int fillW = (int)(barW * pct);
    setfillcolor(RGB(255, 50, 50));
    solidrectangle(barX, barY, barX + fillW, barY + barH);

    // BOSS名称
    settextstyle(16, 0, "System");
    settextcolor(RGB(255, 255, 255));
    char bossText[64];
    if (boss->IsInfinite()) {
        sprintf(bossText, "RAID  %d / %d", boss->GetHP(), boss->GetMaxHP());
    } else {
        sprintf(bossText, "BOSS  %d / %d", boss->GetHP(), boss->GetMaxHP());
    }
    int bw = textwidth(bossText);
    outtextxy((WINDOW_WIDTH - bw) / 2, barY + barH / 4, bossText);

    // 阶段文字
    int phase = boss->GetBossPhase();
    settextstyle(12, 0, "System");
    settextcolor(RGB(255, 255, 0));
    char phaseText[32];
    sprintf(phaseText, "PHASE %d", phase + 1);
    int pw = textwidth(phaseText);
    outtextxy((WINDOW_WIDTH - pw) / 2, barY + barH + 6, phaseText);
}

void Renderer::ShowBossHPBar(bool show) {
    m_bossHpVisible = show;
}

// ============================================================
// 主菜单
// ============================================================
void Renderer::DrawMainMenu() {
    GameManager& gm = GameManager::GetInstance();

    // 半透明深色背景
    setfillcolor(RGB(2, 2, 20));
    solidrectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // 标题
    settextstyle(72, 0, "System");
    settextcolor(RGB(0, 255, 255));
    char title[] = "NEON STRIKER";
    int tw = textwidth(title);
    outtextxy((WINDOW_WIDTH - tw) / 2, 120, title);

    // 副标题
    settextstyle(20, 0, "System");
    settextcolor(RGB(255, 100, 50));
    char subtitle[] = "CYBER BULLET HELL";
    int sw = textwidth(subtitle);
    outtextxy((WINDOW_WIDTH - sw) / 2, 200, subtitle);

    // 分隔线
    setlinecolor(RGB(0, 255, 255));
    line(WINDOW_WIDTH / 2 - 150, 230, WINDOW_WIDTH / 2 + 150, 230);

    // 模式卡片区域
    int cardW = 300, cardH = 120;
    int cardY = 270;
    int gap = 30;

    // 检查鼠标
    int mx = Input::GetInstance().GetMouseX();
    int my = Input::GetInstance().GetMouseY();
    bool clicked = Input::GetInstance().WasMousePressed();

    // 无限模式卡片
    int card1X = WINDOW_WIDTH / 2 - cardW - gap / 2;
    bool hover1 = (mx >= card1X && mx <= card1X + cardW && my >= cardY && my <= cardY + cardH);

    COLORREF borderC1 = hover1 ? RGB(0, 255, 255) : RGB(0, 150, 200);
    setfillcolor(RGB(0, 30, 60));
    setlinecolor(borderC1);
    rectangle(card1X, cardY, card1X + cardW, cardY + cardH);
    if (hover1) {
        setlinecolor(RGB(0, 255, 255));
        rectangle(card1X - 1, cardY - 1, card1X + cardW + 1, cardY + cardH + 1);
    }

    settextstyle(36, 0, "System");
    settextcolor(RGB(0, 255, 255));
    char infLabel[] = "Infinite Mode";
    int lw = textwidth(infLabel);
    outtextxy(card1X + (cardW - lw) / 2, cardY + 20, infLabel);

    settextstyle(14, 0, "System");
    settextcolor(RGB(255, 128, 80));
    char infDesc[] = "Survive - Evolve - Raid at 80";
    int dw = textwidth(infDesc);
    outtextxy(card1X + (cardW - dw) / 2, cardY + 70, infDesc);

    // 点击检测
    if (hover1 && clicked) {
        gm.StartGame();
    }

    // BOSS卡片
    int card2X = WINDOW_WIDTH / 2 + gap / 2;
    bool hover2 = (mx >= card2X && mx <= card2X + cardW && my >= cardY && my <= cardY + cardH);

    COLORREF borderC2 = hover2 ? RGB(0, 255, 255) : RGB(0, 150, 200);
    setfillcolor(RGB(0, 30, 60));
    setlinecolor(borderC2);
    rectangle(card2X, cardY, card2X + cardW, cardY + cardH);
    if (hover2) {
        setlinecolor(RGB(0, 255, 255));
        rectangle(card2X - 1, cardY - 1, card2X + cardW + 1, cardY + cardH + 1);
    }

    settextstyle(36, 0, "System");
    settextcolor(RGB(0, 255, 255));
    char bossLabel[] = "BOSS Challenge";
    lw = textwidth(bossLabel);
    outtextxy(card2X + (cardW - lw) / 2, cardY + 20, bossLabel);

    settextstyle(14, 0, "System");
    settextcolor(RGB(255, 128, 80));
    char bossDesc[] = "Pattern Fight - Phase 1-3";
    dw = textwidth(bossDesc);
    outtextxy(card2X + (cardW - dw) / 2, cardY + 70, bossDesc);

    if (hover2 && clicked) {
        gm.StartBossGame();
    }

    // 历史最高
    settextstyle(18, 0, "System");
    settextcolor(RGB(0, 200, 100));
    char hsLabel[64];
    sprintf(hsLabel, "High Score: %d", gm.GetHighScore());
    int hsl = textwidth(hsLabel);
    outtextxy((WINDOW_WIDTH - hsl) / 2, 420, hsLabel);

    // 控制说明
    settextstyle(14, 0, "System");
    settextcolor(RGB(0, 150, 180));
    char ctrlText[] = "WASD/Drag Move | Auto Fire | E Ultimate | Space Bullet Time | 1/2/3 Weapon";
    int cl = textwidth(ctrlText);
    outtextxy((WINDOW_WIDTH - cl) / 2, 480, ctrlText);

    char ctrlText2[] = "F Filter | F11 Fullscreen";
    int cl2 = textwidth(ctrlText2);
    outtextxy((WINDOW_WIDTH - cl2) / 2, 508, ctrlText2);
}

// ============================================================
// Game Over / Victory / Defeat
// ============================================================
void Renderer::DrawGameOverScreen() {
    GameManager& gm = GameManager::GetInstance();

    setfillcolor(RGB(5, 0, 10));
    solidrectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    settextstyle(56, 0, "System");
    settextcolor(RGB(255, 80, 80));
    char go[] = "GAME OVER";
    int tw = textwidth(go);
    outtextxy((WINDOW_WIDTH - tw) / 2, 200, go);

    setlinecolor(RGB(255, 50, 50));
    line(WINDOW_WIDTH / 2 - 120, 280, WINDOW_WIDTH / 2 + 120, 280);

    settextstyle(18, 0, "System");
    settextcolor(RGB(255, 255, 255));
    char info[128];
    sprintf(info, "Score: %d    Kills: %d    Wave: %d", gm.GetScore(), gm.GetTotalKills(), gm.GetWave());
    int iw = textwidth(info);
    outtextxy((WINDOW_WIDTH - iw) / 2, 320, info);

    char hsInfo[64];
    sprintf(hsInfo, "Best: %d", gm.GetHighScore());
    int hw = textwidth(hsInfo);
    outtextxy((WINDOW_WIDTH - hw) / 2, 350, hsInfo);

    // 按钮
    DrawMenuButton("Restart", WINDOW_WIDTH / 2 - 200, 420, 180, 50);
    DrawMenuButton("Main Menu", WINDOW_WIDTH / 2 + 20, 420, 180, 50);

    // 点击检测
    int mx = Input::GetInstance().GetMouseX(), my = Input::GetInstance().GetMouseY();
    bool clicked = Input::GetInstance().WasMousePressed();

    if (clicked) {
        if (mx >= WINDOW_WIDTH / 2 - 200 && mx <= WINDOW_WIDTH / 2 - 20 && my >= 420 && my <= 470) {
            gm.RestartGame();
        }
        if (mx >= WINDOW_WIDTH / 2 + 20 && mx <= WINDOW_WIDTH / 2 + 200 && my >= 420 && my <= 470) {
            gm.ShowMainMenu();
        }
    }
}

void Renderer::DrawVictoryScreen() {
    GameManager& gm = GameManager::GetInstance();

    setfillcolor(RGB(5, 5, 2));
    solidrectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    settextstyle(56, 0, "System");
    settextcolor(RGB(255, 220, 50));
    char vt[] = "VICTORY";
    int tw = textwidth(vt);
    outtextxy((WINDOW_WIDTH - tw) / 2, 180, vt);

    setlinecolor(RGB(255, 215, 0));
    line(WINDOW_WIDTH / 2 - 100, 260, WINDOW_WIDTH / 2 + 100, 260);

    settextstyle(18, 0, "System");
    settextcolor(RGB(255, 255, 255));
    char info[128];
    sprintf(info, "Time: %.1fs    Damage: %d", gm.GetBossFinishTime(), gm.GetBossDamageDealt());
    int iw = textwidth(info);
    outtextxy((WINDOW_WIDTH - iw) / 2, 300, info);

    DrawMenuButton("Retry", WINDOW_WIDTH / 2 - 200, 390, 180, 50);
    DrawMenuButton("Main Menu", WINDOW_WIDTH / 2 + 20, 390, 180, 50);

    int mx = Input::GetInstance().GetMouseX(), my = Input::GetInstance().GetMouseY();
    bool clicked = Input::GetInstance().WasMousePressed();

    if (clicked) {
        if (mx >= WINDOW_WIDTH / 2 - 200 && mx <= WINDOW_WIDTH / 2 - 20 && my >= 390 && my <= 440) {
            gm.RestartBossGame();
        }
        if (mx >= WINDOW_WIDTH / 2 + 20 && mx <= WINDOW_WIDTH / 2 + 200 && my >= 390 && my <= 440) {
            gm.ShowMainMenu();
        }
    }
}

void Renderer::DrawDefeatScreen() {
    GameManager& gm = GameManager::GetInstance();

    setfillcolor(RGB(20, 2, 2));
    solidrectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    settextstyle(56, 0, "System");
    settextcolor(RGB(255, 100, 100));
    char df[] = "DEFEATED";
    int tw = textwidth(df);
    outtextxy((WINDOW_WIDTH - tw) / 2, 200, df);

    setlinecolor(RGB(255, 68, 68));
    line(WINDOW_WIDTH / 2 - 100, 280, WINDOW_WIDTH / 2 + 100, 280);

    settextstyle(18, 0, "System");
    settextcolor(RGB(255, 255, 255));
    char info[128];
    sprintf(info, "Damage: %d    BOSS HP: %d/%d",
             gm.GetBossDamageDealt(),
             gm.GetBoss() ? gm.GetBoss()->GetHP() : 0, BOSS_HP);
    int iw = textwidth(info);
    outtextxy((WINDOW_WIDTH - iw) / 2, 320, info);

    DrawMenuButton("Retry", WINDOW_WIDTH / 2 - 200, 390, 180, 50);
    DrawMenuButton("Main Menu", WINDOW_WIDTH / 2 + 20, 390, 180, 50);

    int mx = Input::GetInstance().GetMouseX(), my = Input::GetInstance().GetMouseY();
    bool clicked = Input::GetInstance().WasMousePressed();

    if (clicked) {
        if (mx >= WINDOW_WIDTH / 2 - 200 && mx <= WINDOW_WIDTH / 2 - 20 && my >= 390 && my <= 440) {
            gm.RestartBossGame();
        }
        if (mx >= WINDOW_WIDTH / 2 + 20 && mx <= WINDOW_WIDTH / 2 + 200 && my >= 390 && my <= 440) {
            gm.ShowMainMenu();
        }
    }
}

void Renderer::DrawMenuButton(const char* text, int x, int y, int w, int h) {
    int mx = Input::GetInstance().GetMouseX(), my = Input::GetInstance().GetMouseY();
    bool hover = (mx >= x && mx <= x + w && my >= y && my <= y + h);

    COLORREF bg = hover ? RGB(0, 100, 140) : RGB(10, 20, 40);
    COLORREF border = hover ? RGB(0, 255, 255) : RGB(0, 150, 200);
    setfillcolor(bg);
    setlinecolor(border);
    rectangle(x, y, x + w, y + h);

    settextstyle(18, 0, "System");
    settextcolor(hover ? RGB(0, 255, 255) : RGB(255, 255, 255));
    int tw = textwidth((char*)text);
    outtextxy(x + (w - tw) / 2, y + (h - 18) / 2, (char*)text);
}

// ============================================================
// 入场动画
// ============================================================
void Renderer::UpdateIntroAnimation(float dt) {
    GameManager& gm = GameManager::GetInstance();
    gm.SetIntroTimer(gm.GetIntroTimer() + dt);

    int phase = gm.GetIntroPhase();
    float timer = gm.GetIntroTimer();

    // 超时强制结束
    if (timer > 2.5f) {
        gm.SetIntroPhase(0);
        gm.SetState(GameState::PLAYING);
        return;
    }

    // 点击/按键跳过 (用 WasMousePressed 防止菜单拖动残留触发)
    if (Input::GetInstance().WasPressed(VK_SPACE) ||
        Input::GetInstance().WasPressed(VK_RETURN) ||
        Input::GetInstance().WasPressed(VK_ESCAPE) ||
        Input::GetInstance().WasMousePressed()) {
        gm.SetIntroPhase(0);
        gm.SetState(GameState::PLAYING);
        return;
    }

    // 阶段切换
    switch (phase) {
        case 1: // 扫描线
            if (timer > 0.4f) { gm.SetIntroPhase(2); gm.SetIntroTimer(0.0f); }
            break;
        case 2: // SYSTEM ONLINE
            if (timer > 0.4f) { gm.SetIntroPhase(3); gm.SetIntroTimer(0.0f); }
            break;
        case 3: // READY TO FIGHT
            if (timer > 0.5f) { gm.SetIntroPhase(4); gm.SetIntroTimer(0.0f); }
            break;
        case 4: // 淡出
            if (timer > 0.5f) {
                gm.SetIntroPhase(0);
                gm.SetState(GameState::PLAYING);
            }
            break;
    }
}

void Renderer::DrawIntroAnimation() {
    GameManager& gm = GameManager::GetInstance();
    int phase = gm.GetIntroPhase();
    float timer = gm.GetIntroTimer();

    if (phase == 0) return;

    // 半透明叠加
    setfillcolor(AlphaColor(RGB(0, 0, 0), 0.1f));
    solidrectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    switch (phase) {
        case 1: {
            // 扫描线
            float scanY = (timer / 0.3f) * WINDOW_HEIGHT;
            float alpha = (std::min)(1.0f, timer * 5.0f);
            setlinecolor(AlphaColor(RGB(0, 255, 255), alpha));
            line(0, (int)scanY, WINDOW_WIDTH, (int)scanY);
            line(0, (int)scanY + 1, WINDOW_WIDTH, (int)scanY + 1);
            break;
        }
        case 2: {
            // SYSTEM ONLINE
            settextstyle(40, 0, "System");
            if (timer < 0.1f) {
                settextcolor(AlphaColor(RGB(0, 255, 255), timer / 0.1f));
                char t[] = "SYSTEM";
                int tw = textwidth(t);
                outtextxy((WINDOW_WIDTH - tw) / 2, WINDOW_HEIGHT / 2 - 20, t);
            } else {
                settextcolor(RGB(0, 255, 255));
                char t[] = "SYSTEM ONLINE";
                int tw = textwidth(t);
                outtextxy((WINDOW_WIDTH - tw) / 2, WINDOW_HEIGHT / 2 - 20, t);
            }
            break;
        }
        case 3: {
            // READY TO FIGHT
            settextstyle(24, 0, "System");
            if (timer < 0.15f) {
                settextcolor(AlphaColor(RGB(0, 255, 255), timer / 0.15f));
            } else {
                settextcolor(RGB(0, 255, 255));
            }
            char t[] = "READY TO FIGHT";
            int tw = textwidth(t);
            outtextxy((WINDOW_WIDTH - tw) / 2, WINDOW_HEIGHT / 2 + 20, t);
            break;
        }
        case 4: {
            // 淡出
            float alpha = 1.0f - timer / 0.5f;
            settextcolor(AlphaColor(RGB(0, 255, 255), alpha));
            settextstyle(40, 0, "System");
            char t[] = "SYSTEM ONLINE";
            int tw = textwidth(t);
            outtextxy((WINDOW_WIDTH - tw) / 2, WINDOW_HEIGHT / 2 - 20, t);
            break;
        }
    }
}
