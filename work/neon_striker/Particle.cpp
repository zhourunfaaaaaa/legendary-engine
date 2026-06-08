#include "Particle.h"

// ============================================================
// Particle
// ============================================================
Particle::Particle(float x, float y, float vx, float vy,
                   float life, COLORREF color, float size)
    : m_x(x), m_y(y), m_vx(vx), m_vy(vy)
    , m_life(life), m_maxLife(life)
    , m_color(color), m_size(size)
    , m_alive(true)
{
}

void Particle::Update(float dt) {
    m_x += m_vx * dt;
    m_y += m_vy * dt;
    m_life -= dt;
    if (m_life <= 0.0f) m_alive = false;
}

// ============================================================
// ParticleSystem
// ============================================================
ParticleSystem::ParticleSystem() {
}

ParticleSystem::~ParticleSystem() {
    Clear();
}

void ParticleSystem::Emit(float x, float y, int count, COLORREF color,
                          float life, float speed, float size, float spread) {
    for (int i = 0; i < count; i++) {
        float angle = RandomFloat(0.0f, 1.0f) * spread - spread / 2.0f;
        float spd = speed * (0.5f + RandomFloat(0.0f, 1.0f));
        m_particles.push_back(new Particle(
            x, y,
            std::cos(angle) * spd,
            std::sin(angle) * spd,
            life * (0.5f + RandomFloat(0.0f, 1.0f) * 0.5f),
            color,
            size * (0.5f + RandomFloat(0.0f, 1.0f))
        ));
    }
}

void ParticleSystem::EmitRing(float x, float y, int count, COLORREF color,
                              float life, float speed, float size) {
    for (int i = 0; i < count; i++) {
        float angle = (PI * 2.0f / count) * i + RandomFloat(0.0f, 1.0f) * 0.3f;
        float spd = speed * (0.8f + RandomFloat(0.0f, 1.0f) * 0.4f);
        m_particles.push_back(new Particle(
            x, y,
            std::cos(angle) * spd,
            std::sin(angle) * spd,
            life, color, size
        ));
    }
}

void ParticleSystem::EmitDirected(float x, float y, int count, COLORREF color,
                                  float angle, float life, float speed,
                                  float size, float spread) {
    for (int i = 0; i < count; i++) {
        float a = angle + (RandomFloat(0.0f, 1.0f) - 0.5f) * spread;
        float spd = speed * (0.5f + RandomFloat(0.0f, 1.0f));
        m_particles.push_back(new Particle(
            x, y,
            std::cos(a) * spd,
            std::sin(a) * spd,
            life * (0.5f + RandomFloat(0.0f, 1.0f) * 0.5f),
            color,
            size * (0.5f + RandomFloat(0.0f, 1.0f))
        ));
    }
}

void ParticleSystem::Update(float dt) {
    for (int i = (int)m_particles.size() - 1; i >= 0; i--) {
        m_particles[i]->Update(dt);
        if (!m_particles[i]->IsAlive()) {
            delete m_particles[i];
            m_particles[i] = m_particles.back();
            m_particles.pop_back();
        }
    }
}

void ParticleSystem::Clear() {
    for (auto* p : m_particles) delete p;
    m_particles.clear();
}
