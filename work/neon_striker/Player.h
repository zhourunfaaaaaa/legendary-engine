#pragma once

#include "Common.h"

// ============================================================
// Player - 玩家实体
// ============================================================
class Player {
public:
    Player(float x, float y);
    ~Player() = default;

    void Update(float dt);  // dt不受子弹时间影响
    bool CanShoot() const;
    void Shoot();
    bool TakeDamage(int amount);
    void AddEnergy(int amount);
    bool UseUltimate();

    // 受击判定半径（阶段3缩小）
    float GetHitRadius() const;

    // 位置/大小
    float GetX() const { return m_x; }
    float GetY() const { return m_y; }
    void  SetX(float x) { m_x = x; }
    void  SetY(float y) { m_y = y; }
    float GetSize() const { return m_size; }

    // 武器
    WeaponType GetWeaponType() const { return m_weaponType; }
    void SetWeaponType(WeaponType w) { m_weaponType = w; }

    // 后座力
    float GetRecoilX() const { return m_recoilX; }
    float GetRecoilY() const { return m_recoilY; }
    void  SetRecoilX(float v) { m_recoilX = v; }
    void  SetRecoilY(float v) { m_recoilY = v; }

    // HP/能量
    int GetHP() const { return m_hp; }
    int GetMaxHP() const { return m_maxHp; }
    void SetHP(int hp) { m_hp = hp; }
    float GetEnergy() const { return m_energy; }
    float GetMaxEnergy() const { return m_maxEnergy; }
    void SetEnergy(float e) { m_energy = e; }
    float GetInvincible() const { return m_invincible; }
    void SetInvincible(float v) { m_invincible = v; }

    // 射击计时
    float GetShootTimer() const { return m_shootTimer; }
    void  SetShootTimer(float t) { m_shootTimer = t; }
    float GetShootInterval() const { return m_shootInterval; }

    // 动画
    float GetEngineFlame() const { return m_engineFlame; }
    float GetGlowTimer() const { return m_glowTimer; }
    void  SetGlowTimer(float t) { m_glowTimer = t; }
    void  SetEngineFlame(float f) { m_engineFlame = f; }

    // 速度
    float GetSpeed() const { return m_speed; }

private:
    float m_x, m_y;
    float m_size;
    float m_speed;

    int   m_hp;
    int   m_maxHp;
    float m_energy;
    float m_maxEnergy;
    float m_invincible;

    WeaponType m_weaponType;
    float m_shootTimer;
    float m_shootInterval;
    float m_recoilX, m_recoilY;

    float m_engineFlame;
    float m_glowTimer;
};
