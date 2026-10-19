#include "AudioSystem.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

AudioSystem::AudioSystem() = default;

AudioSystem::~AudioSystem() {
    Close();
}

Sound AudioSystem::GenerateBeep(float freq, float duration, float decay) {
    int sampleRate = 44100;
    int frameCount = static_cast<int>(sampleRate * duration);
    short* buffer = (short*)malloc(frameCount * sizeof(short));

    for (int i = 0; i < frameCount; ++i) {
        float t = (float)i / (float)sampleRate;
        float env = std::exp(-t * decay);
        float sample = std::sin(2.0f * PI * freq * t) * env;
        buffer[i] = static_cast<short>(std::clamp(sample, -1.0f, 1.0f) * 32000.0f);
    }

    Wave wave = {
        .frameCount = static_cast<unsigned int>(frameCount),
        .sampleRate = static_cast<unsigned int>(sampleRate),
        .sampleSize = 16,
        .channels = 1,
        .data = buffer
    };

    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}

Sound AudioSystem::GenerateNoise(float duration, float decay, float lowpass) {
    int sampleRate = 44100;
    int frameCount = static_cast<int>(sampleRate * duration);
    short* buffer = (short*)malloc(frameCount * sizeof(short));

    float filter = 0.0f;
    for (int i = 0; i < frameCount; ++i) {
        float t = (float)i / (float)sampleRate;
        float env = std::exp(-t * decay);
        float white = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        filter += (white - filter) * lowpass;
        float sample = filter * env;
        buffer[i] = static_cast<short>(std::clamp(sample, -1.0f, 1.0f) * 32000.0f);
    }

    Wave wave = {
        .frameCount = static_cast<unsigned int>(frameCount),
        .sampleRate = static_cast<unsigned int>(sampleRate),
        .sampleSize = 16,
        .channels = 1,
        .data = buffer
    };

    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}

Sound AudioSystem::GenerateWindLoop() {
    int sampleRate = 44100;
    float duration = 1.0f; // 1 second loop
    int frameCount = static_cast<int>(sampleRate * duration);
    short* buffer = (short*)malloc(frameCount * sizeof(short));

    float filter = 0.0f;
    for (int i = 0; i < frameCount; ++i) {
        float white = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        filter += (white - filter) * 0.04f; // Soft filtered pinkish noise
        buffer[i] = static_cast<short>(std::clamp(filter * 1.5f, -1.0f, 1.0f) * 24000.0f);
    }

    Wave wave = {
        .frameCount = static_cast<unsigned int>(frameCount),
        .sampleRate = static_cast<unsigned int>(sampleRate),
        .sampleSize = 16,
        .channels = 1,
        .data = buffer
    };

    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}

void AudioSystem::Init() {
    if (initialized) return;

    InitAudioDevice();

    // Procedural sound generation
    sndFootstep[0] = GenerateNoise(0.08f, 35.0f, 0.25f);
    sndFootstep[1] = GenerateNoise(0.09f, 32.0f, 0.22f);
    sndFootstep[2] = GenerateNoise(0.08f, 38.0f, 0.28f);

    sndJump = GenerateNoise(0.18f, 15.0f, 0.15f);
    SetSoundPitch(sndJump, 1.4f);

    sndLandSoft = GenerateNoise(0.20f, 18.0f, 0.35f);
    sndLandHard = GenerateNoise(0.35f, 10.0f, 0.50f);

    sndSlide = GenerateNoise(0.30f, 6.0f, 0.40f);
    sndWallrun = GenerateNoise(0.25f, 8.0f, 0.30f);
    sndRoll = GenerateNoise(0.40f, 7.0f, 0.18f);

    sndBonus = GenerateBeep(880.0f, 0.25f, 12.0f);
    sndSpringboard = GenerateBeep(520.0f, 0.35f, 8.0f);
    sndGameOver = GenerateBeep(220.0f, 0.8f, 4.0f);

    sndWindLoop = GenerateWindLoop();

    initialized = true;
}

void AudioSystem::Close() {
    if (!initialized) return;

    for (int i = 0; i < 3; ++i) UnloadSound(sndFootstep[i]);
    UnloadSound(sndJump);
    UnloadSound(sndLandSoft);
    UnloadSound(sndLandHard);
    UnloadSound(sndSlide);
    UnloadSound(sndWallrun);
    UnloadSound(sndRoll);
    UnloadSound(sndBonus);
    UnloadSound(sndSpringboard);
    UnloadSound(sndGameOver);
    UnloadSound(sndWindLoop);

    CloseAudioDevice();
    initialized = false;
}

void AudioSystem::PlayFootstep() {
    if (!initialized) return;
    SetSoundVolume(sndFootstep[currentFootstep], 0.35f);
    SetSoundPitch(sndFootstep[currentFootstep], 0.9f + (float)(rand() % 20) / 100.0f);
    PlaySound(sndFootstep[currentFootstep]);
    currentFootstep = (currentFootstep + 1) % 3;
}

void AudioSystem::PlayJump() {
    if (!initialized) return;
    SetSoundVolume(sndJump, 0.5f);
    PlaySound(sndJump);
}

void AudioSystem::PlayLand(bool hardFall) {
    if (!initialized) return;
    if (hardFall) {
        SetSoundVolume(sndLandHard, 0.7f);
        PlaySound(sndLandHard);
    } else {
        SetSoundVolume(sndLandSoft, 0.45f);
        PlaySound(sndLandSoft);
    }
}

void AudioSystem::PlaySlide() {
    if (!initialized) return;
    SetSoundVolume(sndSlide, 0.6f);
    PlaySound(sndSlide);
}

void AudioSystem::PlayWallrunStart() {
    if (!initialized) return;
    SetSoundVolume(sndWallrun, 0.55f);
    PlaySound(sndWallrun);
}

void AudioSystem::PlayRoll() {
    if (!initialized) return;
    SetSoundVolume(sndRoll, 0.65f);
    PlaySound(sndRoll);
}

void AudioSystem::PlayStuntBonus() {
    if (!initialized) return;
    SetSoundVolume(sndBonus, 0.5f);
    PlaySound(sndBonus);
}

void AudioSystem::PlaySpringboard() {
    if (!initialized) return;
    SetSoundVolume(sndSpringboard, 0.7f);
    PlaySound(sndSpringboard);
}

void AudioSystem::PlayGameOver() {
    if (!initialized) return;
    SetSoundVolume(sndGameOver, 0.8f);
    PlaySound(sndGameOver);
}

void AudioSystem::Update(float speedRatio, bool isWallrunning, bool isSliding) {
    if (!initialized) return;

    // Loop wind rush at high speeds
    if (speedRatio > 0.3f) {
        if (!windLoopPlaying) {
            PlaySound(sndWindLoop);
            windLoopPlaying = true;
        } else if (!IsSoundPlaying(sndWindLoop)) {
            PlaySound(sndWindLoop);
        }
        float vol = std::clamp((speedRatio - 0.3f) * 0.9f, 0.0f, 0.8f);
        SetSoundVolume(sndWindLoop, vol);
        SetSoundPitch(sndWindLoop, 0.8f + speedRatio * 0.5f);
    } else {
        if (windLoopPlaying && IsSoundPlaying(sndWindLoop)) {
            StopSound(sndWindLoop);
            windLoopPlaying = false;
        }
    }

    if (isWallrunning && !IsSoundPlaying(sndWallrun)) {
        PlaySound(sndWallrun);
    }
    if (isSliding && !IsSoundPlaying(sndSlide)) {
        PlaySound(sndSlide);
    }
}
