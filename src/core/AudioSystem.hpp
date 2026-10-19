#pragma once

#include "raylib.h"
#include <vector>

class AudioSystem {
public:
    AudioSystem();
    ~AudioSystem();

    void Init();
    void Close();

    void PlayFootstep();
    void PlayJump();
    void PlayLand(bool hardFall);
    void PlaySlide();
    void PlayWallrunStart();
    void PlayRoll();
    void PlayStuntBonus();
    void PlaySpringboard();
    void PlayGameOver();

    // Updates continuous sounds (wind whoosh, etc.)
    void Update(float speedRatio, bool isWallrunning, bool isSliding);

private:
    bool initialized = false;

    Sound sndFootstep[3];
    Sound sndJump;
    Sound sndLandSoft;
    Sound sndLandHard;
    Sound sndSlide;
    Sound sndWallrun;
    Sound sndRoll;
    Sound sndBonus;
    Sound sndSpringboard;
    Sound sndGameOver;

    Sound sndWindLoop;
    bool windLoopPlaying = false;

    int currentFootstep = 0;

    Sound GenerateBeep(float freq, float duration, float decay);
    Sound GenerateNoise(float duration, float decay, float lowpass);
    Sound GenerateWindLoop();
};
