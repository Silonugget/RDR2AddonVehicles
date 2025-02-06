#pragma once
#include "../global.h"
#include "Examples.hpp"
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

class SoundEngine {
public:
    SoundEngine();
    ~SoundEngine();

    HRESULT Init();
    void UnInit();
    HRESULT LoadSoundFile(const std::string& filename, XAUDIO2_BUFFER& buffer, WAVEFORMATEX& wfx);
    HRESULT StartIdlePlayback(const std::string& filename, float speed);
        HRESULT StartIdlePlayback2(const std::string& filename);

    void StopPlayback();
    void CheckIfInVehicle();

    void UpdateSpeed();
	float minVolume = 0.15f;

private:
    IXAudio2* pXAudio2;
    IXAudio2MasteringVoice* pMasterVoice;
    IXAudio2SourceVoice* pSourceVoice;
    XAUDIO2_BUFFER buffer;
    float maxSpeed;
    float speedToVolumeRatio;
    float speedToPitchRatio;

    bool engineIdlePlaying = false;
        std::string idleSound;
    std::string startSound;
    std::string stopSound;
    HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition);
    HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset);
    void AdjustPlaybackRate(float pitchMultiplier);
    void SimulateSound(float speed);
    void GetSoundFiles(std::string& idleSound, std::string& startSound, std::string& stopSound);
    float CalculateVolume(float speed);
    float CalculateDistance(const Vector3& pos1, const Vector3& pos2);
    float CalculatePitch(float speed);
};

extern SoundEngine* g_SoundEngine;