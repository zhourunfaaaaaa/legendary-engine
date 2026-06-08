#include "Audio.h"
#include <windows.h>
#include <process.h>
#include <thread>

// 音效参数结构体
struct BeepParam {
    DWORD freq;
    DWORD duration;
    int   repeat;   // 重复次数 (用于散射/爆炸多音)
    DWORD gap;      // 重复间隔ms
};

DWORD WINAPI Audio::BeepThread(LPVOID lpParam) {
    BeepParam* bp = (BeepParam*)lpParam;
    for (int i = 0; i < bp->repeat; i++) {
        Beep(bp->freq, bp->duration);
        if (i < bp->repeat - 1) Sleep(bp->gap);
    }
    delete bp;
    return 0;
}

void Audio::PlayLaser() {
    BeepParam* bp = new BeepParam{ 1200, 30, 1, 30 };
    CreateThread(nullptr, 0, BeepThread, bp, 0, nullptr);
}

void Audio::PlayPulseLaser() {
    BeepParam* bp = new BeepParam{ 300, 60, 1, 30 };
    CreateThread(nullptr, 0, BeepThread, bp, 0, nullptr);
}

void Audio::PlayScatterLaser() {
    BeepParam* bp = new BeepParam{ 800, 25, 3, 20 };
    CreateThread(nullptr, 0, BeepThread, bp, 0, nullptr);
}

void Audio::PlayExplosion(float size) {
    DWORD dur = (DWORD)(150 * size);
    BeepParam* bp = new BeepParam{ 80, dur, 1, 30 };
    CreateThread(nullptr, 0, BeepThread, bp, 0, nullptr);
}

void Audio::PlayHit() {
    BeepParam* bp = new BeepParam{ 1500, 20, 1, 30 };
    CreateThread(nullptr, 0, BeepThread, bp, 0, nullptr);
}

void Audio::PlayUltimate() {
    BeepParam* bp = new BeepParam{ 200, 40, 8, 50 };
    CreateThread(nullptr, 0, BeepThread, bp, 0, nullptr);
}

void Audio::PlayDamage() {
    BeepParam* bp = new BeepParam{ 150, 120, 1, 30 };
    CreateThread(nullptr, 0, BeepThread, bp, 0, nullptr);
}

void Audio::PlayCombo(int level) {
    DWORD freq = 400 + level * 200;
    BeepParam* bp = new BeepParam{ freq, 80, 1, 30 };
    CreateThread(nullptr, 0, BeepThread, bp, 0, nullptr);
}

void Audio::PlayUIClick() {
    BeepParam* bp = new BeepParam{ 1000, 30, 1, 30 };
    CreateThread(nullptr, 0, BeepThread, bp, 0, nullptr);
}

void Audio::PlayTimeWarpOn() {
    BeepParam* bp = new BeepParam{ 500, 200, 1, 30 };
    CreateThread(nullptr, 0, BeepThread, bp, 0, nullptr);
}

void Audio::PlayTimeWarpOff() {
    BeepParam* bp = new BeepParam{ 500, 100, 1, 30 };
    CreateThread(nullptr, 0, BeepThread, bp, 0, nullptr);
}

void Audio::PlayInfiniteBossAppear() {
    BeepParam* bp = new BeepParam{ 80, 300, 2, 100 };
    CreateThread(nullptr, 0, BeepThread, bp, 0, nullptr);
}

void Audio::PlayInfiniteBossDeath() {
    BeepParam* bp = new BeepParam{ 200, 150, 3, 80 };
    CreateThread(nullptr, 0, BeepThread, bp, 0, nullptr);
}
