#pragma once

#include <vector>
#include "Common.h"

// ============================================================
// Particle - 粒子
// ============================================================
class Particle {
public:
    Particle(float x, float y, float vx, float vy, float life, COLORREF color, float size = 2.0f);
    ~Particle() = default;

    void Update(float dt);

    float GetX() const { return m_x; }
    float GetY() const { return m_y; }
    float GetSize() const { return m_size; }
    float GetLife() const { return m_life; }
    float GetMaxLife() const { return m_maxLife; }
    COLORREF GetColor() const { return m_color; }
    bool  IsAlive() const { return m_alive; }

private:
    float m_x, m_y;
    float m_vx, m_vy;
    float m_life, m_maxLife;
    COLORREF m_color;
    float m_size;
    bool  m_alive;
};

// ============================================================
// ParticleSystem - 粒子系统
// ============================================================
class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();

    void Emit(float x, float y, int count, COLORREF color, float life = 0.5f,
              float speed = 200.0f, float size = 2.0f, float spread = PI * 2);
    void EmitRing(float x, float y, int count, COLORREF color, float life = 0.5f,
                  float speed = 200.0f, float size = 2.0f);
    void EmitDirected(float x, float y, int count, COLORREF color, float angle,
                      float life = 0.5f, float speed = 200.0f, float size = 2.0f,
                      float spread = 0.5f);

    void Update(float dt);
    void Clear();
    const std::vector<Particle*>& GetParticles() const { return m_particles; }

private:
    std::vector<Particle*> m_particles;
};
