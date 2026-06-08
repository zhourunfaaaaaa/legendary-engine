#pragma once

#include "Common.h"

// ============================================================
// Boss - BOSS实体 (挑战模式 + 无限Raid)
// ============================================================
class Boss {
public:
    Boss(float x, float y, int hp, bool isInfinite = false);
    ~Boss() = default;

    void Update(float dt);

    Vector2 GetShootPos() const;

    float GetX() const { return m_x; }
    float GetY() const { return m_y; }
    void  SetX(float x) { m_x = x; }
    void  SetY(float y) { m_y = y; }
    float GetSize() const { return m_size; }

    int  GetHP() const { return m_hp; }
    int  GetMaxHP() const { return m_maxHp; }
    void SetHP(int hp) { m_hp = hp; }
    bool IsAlive() const { return m_alive; }
    void Kill() { m_alive = false; }

    bool IsInfinite() const { return m_isInfinite; }

    // 阶段
    int GetBossPhase() const { return m_bossPhase; }

    // 闪烁
    float GetFlashTimer() const { return m_flashTimer; }
    void  SetFlashTimer(float t) { m_flashTimer = t; }

    // 无敌
    float GetInvulnTimer() const { return m_invulnTimer; }
    void  SetInvulnTimer(float t) { m_invulnTimer = t; }

    // 攻击冷却
    float GetRingBurstTimer() const { return m_ringBurstTimer; }
    void  SetRingBurstTimer(float t) { m_ringBurstTimer = t; }
    float GetAimedFanTimer() const { return m_aimedFanTimer; }
    void  SetAimedFanTimer(float t) { m_aimedFanTimer = t; }
    float GetWarnBeamTimer() const { return m_warnBeamTimer; }
    void  SetWarnBeamTimer(float t) { m_warnBeamTimer = t; }
    float GetLaserOrbTimer() const { return m_laserOrbTimer; }
    void  SetLaserOrbTimer(float t) { m_laserOrbTimer = t; }
    float GetBounceTimer() const { return m_bounceTimer; }
    void  SetBounceTimer(float t) { m_bounceTimer = t; }

    // 攻击解锁
    bool HasRingBurst() const { return m_hasRingBurst; }
    bool HasAimedFan() const { return m_hasAimedFan; }
    bool HasWarnBeam() const { return m_hasWarnBeam; }
    bool HasBounce() const { return m_hasBounce; }
    bool HasLaserOrb() const { return m_hasLaserOrb; }
    void UnlockWarnBeam() { m_hasWarnBeam = true; }
    void UnlockBounce() { m_hasBounce = true; }
    void UnlockLaserOrb() { m_hasLaserOrb = true; }

    // 预警激光状态
    bool  IsWarnBeamActive() const { return m_warnBeamActive; }
    void  SetWarnBeamActive(bool a) { m_warnBeamActive = a; }
    int   GetWarnBeamPhase() const { return m_warnBeamPhase; }
    void  SetWarnBeamPhase(int p) { m_warnBeamPhase = p; }
    float GetWarnBeamTimer2() const { return m_warnBeamTimer2; }
    void  SetWarnBeamTimer2(float t) { m_warnBeamTimer2 = t; }
    float GetWarnBeamAngle() const { return m_warnBeamAngle; }
    void  SetWarnBeamAngle(float a) { m_warnBeamAngle = a; }

    // Phase3加速
    float GetPhase3SpeedMult() const { return m_phase3SpeedMult; }

    // 移动
    int GetMoveDir() const { return m_moveDir; }

    // 螺旋角
    float GetSpiralAngle() const { return m_spiralAngle; }
    void  SetSpiralAngle(float a) { m_spiralAngle = a; }

private:
    float m_x, m_y;
    float m_size;
    int   m_hp, m_maxHp;
    bool  m_alive;
    bool  m_isInfinite;

    int   m_bossPhase;
    float m_flashTimer;
    float m_invulnTimer;
    int   m_moveDir;

    // 攻击冷却
    float m_ringBurstTimer,  m_ringBurstCD;
    float m_aimedFanTimer,   m_aimedFanCD;
    float m_warnBeamTimer,   m_warnBeamCD;
    float m_laserOrbTimer,   m_laserOrbCD;
    float m_bounceTimer,     m_bounceCD;

    bool m_hasRingBurst, m_hasAimedFan, m_hasWarnBeam, m_hasBounce, m_hasLaserOrb;

    // 预警激光
    bool  m_warnBeamActive;
    int   m_warnBeamPhase;
    float m_warnBeamTimer2;
    float m_warnBeamAngle;

    float m_phase3SpeedMult;
    float m_spiralAngle;
};
