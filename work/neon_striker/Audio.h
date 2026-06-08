#pragma once

#include "Common.h"

// ============================================================
// Audio - 音效系统 (Windows Beep)
// ============================================================
class Audio {
public:
    static Audio& GetInstance() {
        static Audio instance;
        return instance;
    }

    void PlayLaser();
    void PlayPulseLaser();
    void PlayScatterLaser();
    void PlayExplosion(float size = 1.0f);
    void PlayHit();
    void PlayUltimate();
    void PlayDamage();
    void PlayCombo(int level);
    void PlayUIClick();
    void PlayTimeWarpOn();
    void PlayTimeWarpOff();
    void PlayInfiniteBossAppear();
    void PlayInfiniteBossDeath();

    // Beep线程
    static DWORD WINAPI BeepThread(LPVOID lpParam);

private:
    Audio() = default;
};
