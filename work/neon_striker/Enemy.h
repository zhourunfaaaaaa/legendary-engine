#pragma once

#include "Common.h"

// ============================================================
// Enemy - 敌人实体（6种类型）
// ============================================================
class Enemy {
public:
    Enemy(float x, float y, EnemyType type);
    ~Enemy() = default;

    void Update(float dt);

    Vector2 GetShootPos() const;

    // 属性
    float GetX() const { return m_x; }
    float GetY() const { return m_y; }
    void  SetX(float x) { m_x = x; }
    void  SetY(float y) { m_y = y; }
    float GetSize() const { return m_size; }
    float GetSpeed() const { return m_speed; }

    EnemyType GetType() const { return m_type; }

    int  GetHP() const { return m_hp; }
    int  GetMaxHP() const { return m_maxHp; }
    void SetHP(int hp) { m_hp = hp; }

    bool IsAlive() const { return m_alive; }
    void Kill() { m_alive = false; }

    ShootPattern GetPattern() const { return m_pattern; }

    // 射击计时
    float GetShootTimer() const { return m_shootTimer; }
    void  SetShootTimer(float t) { m_shootTimer = t; }
    float GetShootInterval() const { return m_shootInterval; }

    // 特殊属性
    float GetSpiralAngle() const { return m_spiralAngle; }
    void  SetSpiralAngle(float a) { m_spiralAngle = a; }
    float GetPhase() const { return m_phase; }
    void  SetPhase(float p) { m_phase = p; }

    // 护盾
    bool  IsShieldActive() const { return m_shieldActive; }
    void  SetShieldActive(bool a) { m_shieldActive = a; }
    float GetShieldMax() const { return m_shieldMax; }
    float GetShieldDR() const { return m_shieldDR; }

    // 分裂
    int  GetSplitCount() const { return m_splitCount; }
    bool HasSplit() const { return m_hasSplit; }
    void SetHasSplit(bool s) { m_hasSplit = s; }

    // 狙击
    bool  HasWarningLine() const { return m_warningLine; }
    void  SetWarningLine(bool w) { m_warningLine = w; }
    float GetWarnTimer() const { return m_warnTimer; }
    void  SetWarnTimer(float t) { m_warnTimer = t; }
    float GetSniperAngle() const { return m_sniperAngle; }
    void  SetSniperAngle(float a) { m_sniperAngle = a; }

    // 闪烁
    float GetFlashTimer() const { return m_flashTimer; }
    void  SetFlashTimer(float t) { m_flashTimer = t; }

    // 分数
    int  GetScoreValue() const { return m_scoreValue; }

    // 颜色
    COLORREF GetColor() const { return m_color; }

    // 摇摆
    float GetWobble() const { return m_wobble; }
    void  SetWobble(float w) { m_wobble = w; }
    float GetWobbleSpeed() const { return m_wobbleSpeed; }
    float GetWobbleAmp() const { return m_wobbleAmp; }

private:
    float m_x, m_y;
    float m_size;
    float m_speed;
    bool  m_alive;

    EnemyType m_type;
    int  m_hp, m_maxHp;
    int  m_scoreValue;
    COLORREF m_color;

    ShootPattern m_pattern;
    float m_shootTimer;
    float m_shootInterval;
    float m_spiralAngle;
    float m_phase;

    // 护盾
    bool  m_shieldActive;
    float m_shieldMax;
    float m_shieldDR;

    // 分裂
    int  m_splitCount;
    bool m_hasSplit;

    // 狙击
    bool  m_warningLine;
    float m_warnTimer;
    float m_sniperAngle;

    float m_flashTimer;

    // 移动摇摆
    float m_wobble;
    float m_wobbleSpeed;
    float m_wobbleAmp;
};
