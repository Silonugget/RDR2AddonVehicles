#include "CVehAudio.hpp"
#include <fstream>
#include <vector>
#include <iostream>
#include <xaudio2.h>
bool playSounds = true;

SoundEngine* g_SoundEngine = new SoundEngine();

SoundEngine::SoundEngine()
    : pXAudio2(nullptr), pMasterVoice(nullptr), pSourceVoice(nullptr),
      maxSpeed(30.0f), speedToVolumeRatio(0.1f), speedToPitchRatio(7.0f), minVolume(0.15f) {
    ZeroMemory(&buffer, sizeof(buffer)); // Initialize buffer
}

SoundEngine::~SoundEngine() {
    UnInit();
}

HRESULT SoundEngine::Init() {
    HRESULT hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr)) return hr;

    hr = pXAudio2->CreateMasteringVoice(&pMasterVoice);
    return hr;
}

void SoundEngine::UnInit() {
    if (pSourceVoice) {
        pSourceVoice->DestroyVoice();
        pSourceVoice = nullptr;
    }
    if (pMasterVoice) {
        pMasterVoice->DestroyVoice();
        pMasterVoice = nullptr;
    }
    if (pXAudio2) {
        pXAudio2->Release();
        pXAudio2 = nullptr;
    }
}

HRESULT SoundEngine::FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition) {
    HRESULT hr = S_OK;
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());

    DWORD dwChunkType;
    DWORD dwChunkDataSize;
    DWORD dwRIFFDataSize = 0;
    DWORD dwFileType;
    DWORD bytesRead = 0;
    DWORD dwOffset = 0;

    while (hr == S_OK) {
        DWORD dwRead;
        if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());

        if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());

        switch (dwChunkType) {
        case fourccRIFF:
            dwRIFFDataSize = dwChunkDataSize;
            dwChunkDataSize = 4;
            if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
                hr = HRESULT_FROM_WIN32(GetLastError());
            break;

        default:
            if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
                return HRESULT_FROM_WIN32(GetLastError());
        }

        dwOffset += sizeof(DWORD) * 2;

        if (dwChunkType == fourcc) {
            dwChunkSize = dwChunkDataSize;
            dwChunkDataPosition = dwOffset;
            return S_OK;
        }

        dwOffset += dwChunkDataSize;

        if (bytesRead >= dwRIFFDataSize) return S_FALSE;
    }

    return S_OK;
}

HRESULT SoundEngine::ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset) {
    HRESULT hr = S_OK;
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());
    DWORD dwRead;
    if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
        hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}

HRESULT SoundEngine::LoadSoundFile(const std::string& filename, XAUDIO2_BUFFER& buffer, WAVEFORMATEX& wfx) {
    HRESULT hr = S_OK;
    HANDLE hFile = CreateFile(
        filename.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (INVALID_HANDLE_VALUE == hFile) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    DWORD dwChunkSize;
    DWORD dwChunkPosition;
    DWORD filetype;

    // Check the file type, should be fourccWAVE or 'XWMA'
    if (FAILED(hr = FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition))) {
        CloseHandle(hFile);
        return hr;
    }

    if (FAILED(hr = ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition))) {
        CloseHandle(hFile);
        return hr;
    }

    if (filetype != fourccWAVE) {
        CloseHandle(hFile);
        return S_FALSE;
    }

    // Read the format chunk
    if (FAILED(hr = FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition))) {
        CloseHandle(hFile);
        return hr;
    }

    if (FAILED(hr = ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition))) {
        CloseHandle(hFile);
        return hr;
    }

    // Read the data chunk
    if (FAILED(hr = FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition))) {
        CloseHandle(hFile);
        return hr;
    }

    BYTE* pDataBuffer = new BYTE[dwChunkSize];
    if (FAILED(hr = ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition))) {
        delete[] pDataBuffer;
        CloseHandle(hFile);
        return hr;
    }

    // Set the buffer
    buffer.AudioBytes = dwChunkSize;
    buffer.pAudioData = pDataBuffer;
    buffer.Flags = XAUDIO2_END_OF_STREAM;

    CloseHandle(hFile);
    return S_OK;
}

HRESULT SoundEngine::StartIdlePlayback(const std::string& filename, float speed) {
    WAVEFORMATEX wfx = { 0 };
    HRESULT hr = LoadSoundFile(filename, buffer, wfx);
    if (FAILED(hr)) return hr;

    buffer.LoopCount = XAUDIO2_LOOP_INFINITE; // Set the loop count here

    hr = pXAudio2->CreateSourceVoice(&pSourceVoice, &wfx);
    if (FAILED(hr)) return hr;

    // Simulate sound to adjust volume based on speed
    SimulateSound(speed);

    hr = pSourceVoice->SubmitSourceBuffer(&buffer);
    if (FAILED(hr)) return hr;

    hr = pSourceVoice->Start(0);
    return hr;
}
HRESULT SoundEngine::StartIdlePlayback2(const std::string& filename) {
    WAVEFORMATEX wfx = { 0 };
    HRESULT hr = LoadSoundFile(filename, buffer, wfx);
    if (FAILED(hr)) return hr;

    buffer.LoopCount = 0;
    hr = pXAudio2->CreateSourceVoice(&pSourceVoice, &wfx);
    if (FAILED(hr)) return hr;

    // Set initial volume for the sound (adjust as needed)
    float initialVolume = 0.25f; // Example volume (range is 0.0f to 1.0f)
    hr = pSourceVoice->SetVolume(initialVolume);
    if (FAILED(hr)) return hr;

    hr = pSourceVoice->SubmitSourceBuffer(&buffer);
    if (FAILED(hr)) return hr;

    hr = pSourceVoice->Start(0);
    return hr;
}

void SoundEngine::StopPlayback() {
    if (pSourceVoice) {
        pSourceVoice->Stop(0);
        pSourceVoice->DestroyVoice();
        pSourceVoice = nullptr;
    }
}
void SoundEngine::CheckIfInVehicle() {
    if (!playSounds) {
        if (engineIdlePlaying) {
            std::cout << "playSounds is false. Stopping engine idle sound." << std::endl;
            StopPlayback(); // Stop the sound
            engineIdlePlaying = false; // Reset the flag
        }
        return; // Exit early if playSounds is false
    }

    GetSoundFiles(idleSound, startSound, stopSound);

    if (CSpawnSubmenu::vehicleData.engine) {
        // Disable controls
        PAD::DISABLE_CONTROL_ACTION(INPUT_GROUP_KEYBOARD, INPUT_FRONTEND_PAUSE, true);
        PAD::DISABLE_CONTROL_ACTION(INPUT_GROUP_KEYBOARD, INPUT_FRONTEND_PAUSE_ALTERNATE, true);
        PAD::DISABLE_CONTROL_ACTION(INPUT_GROUP_KEYBOARD, INPUT_MAP, true);

        // Check if any of the disabled controls are pressed
        bool controlPressed = false;
        if (PAD::IS_DISABLED_CONTROL_PRESSED(INPUT_GROUP_KEYBOARD, INPUT_FRONTEND_PAUSE) ||
            PAD::IS_DISABLED_CONTROL_PRESSED(INPUT_GROUP_KEYBOARD, INPUT_FRONTEND_PAUSE_ALTERNATE) ||
            PAD::IS_DISABLED_CONTROL_PRESSED(INPUT_GROUP_KEYBOARD, INPUT_MAP)) {
            controlPressed = true;
        }

        if (controlPressed) {
            // Set volume to 0.0 and skip updates
            if (pSourceVoice) {
                pSourceVoice->SetVolume(0.0f);
            }
            std::cout << "Disabled key pressed. Volume set to 0.0, skipping updates." << std::endl;

            // Re-enable controls
            PAD::ENABLE_CONTROL_ACTION(INPUT_GROUP_KEYBOARD, INPUT_FRONTEND_PAUSE, true);
            PAD::ENABLE_CONTROL_ACTION(INPUT_GROUP_KEYBOARD, INPUT_FRONTEND_PAUSE_ALTERNATE, true);
            PAD::ENABLE_CONTROL_ACTION(INPUT_GROUP_KEYBOARD, INPUT_MAP, true);

            WAIT(500); // Wait only if a disabled key was pressed
            return; 
        }

        // If no disabled key was pressed, proceed with normal logic
        if (!engineIdlePlaying) {
            std::cout << "Vehicle detected. Starting engine idle sound." << std::endl;
            float speed = CSpawnSubmenu::vehicleData.currspeed;

            StartIdlePlayback2(startSound); // Start the start sound
            WAIT(600);
            StartIdlePlayback(idleSound, speed); // Start the idle sound

            engineIdlePlaying = true; // Set the flag
        }
        UpdateSpeed(); // Update speed
    } else if (engineIdlePlaying) {
        std::cout << "Vehicle no longer detected. Stopping engine idle sound." << std::endl;
        StopPlayback(); // Stop the sound
        StartIdlePlayback2(stopSound); // Start the stop sound
        engineIdlePlaying = false; // Reset the flag
    }
}





void SoundEngine::UpdateSpeed() {
    // Get the player's coordinates
    Vector3 playerPos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true);

    // Get the vehicle's coordinates
    Vector3 vehiclePos = ENTITY::GET_ENTITY_COORDS(CSpawnSubmenu::vehicleData.underVehicle, NULL, true);

    // Calculate the distance between the player and the vehicle
    float distance = CalculateDistance(playerPos, vehiclePos);

    // Determine the type of vehicle
    std::string vehicleType = CSpawnSubmenu::vehicleData.vehicleType;

    // Calculate the minimum volume based on the distance and vehicle type
    float minVolume;
    if (vehicleType == "car" || vehicleType == "bike" || vehicleType == "boat" || vehicleType == "tank") {
        minVolume = 0.15f - (distance * 0.006f);
    } else {
        minVolume = 0.15f - (distance * 0.003f);
    }

    if (minVolume < 0.0f) {
        minVolume = 0.0f;
    }

    // If the player is too far from the vehicle, set the volume to the minimum and skip speed update
    if (distance > 15.0f) { // Define an appropriate threshold value
        if (pSourceVoice) {
            pSourceVoice->SetVolume(minVolume);
        }
        return;
    }

    // Get the speed from CSpawnSubmenu::vehicleData
    float speed = CSpawnSubmenu::vehicleData.currspeed;

    // Implement gear boost logic
    float targetGearBoost = 0.0f;
    if ((PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_UP_ONLY")) || PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_SPRINT"))) && CSpawnSubmenu::vehicleData.playerInVehicle) {
        if (vehicleType == "car" || vehicleType == "bike") {
            if (speed < 20.0f) {
                targetGearBoost = 12.0f; // Gear 1 boost
            }
        } else {
            targetGearBoost = 8.0f;
        }
    }

    // Smoothly interpolate the gear boost
    static float currentGearBoost = 0.0f;
    float interpolationSpeed = 0.1f; // Adjust this value to control the smoothness
    currentGearBoost = currentGearBoost + interpolationSpeed * (targetGearBoost - currentGearBoost);

    // Apply gear boost to speed
    speed += currentGearBoost;

    SimulateSound(speed);
    float pitchMultiplier = CalculatePitch(speed);
    AdjustPlaybackRate(pitchMultiplier);
}




void SoundEngine::AdjustPlaybackRate(float pitchMultiplier) {
    if (pSourceVoice) {
        pSourceVoice->SetFrequencyRatio(pitchMultiplier);
    }
}

void SoundEngine::SimulateSound(float speed) {
    float volume = CalculateVolume(speed);
    if (pSourceVoice) {
        pSourceVoice->SetVolume(volume);
    }
}
const float MAX_VOLUME = 0.2f; // Define the maximum volume

// Function to calculate the distance between two points
float SoundEngine::CalculateDistance(const Vector3& pos1, const Vector3& pos2) {
    return sqrt(pow(pos2.x - pos1.x, 2) + pow(pos2.y - pos1.y, 2) + pow(pos2.z - pos1.z, 2));
}
float SoundEngine::CalculateVolume(float speed) {
    // Get the player's coordinates
    Vector3 playerPos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true);

    // Get the vehicle's coordinates
    Vector3 vehiclePos = ENTITY::GET_ENTITY_COORDS(CSpawnSubmenu::vehicleData.underVehicle, NULL, true);

    // Calculate the distance between the player and the vehicle
    float distance = CalculateDistance(playerPos, vehiclePos);

    // Determine the type of vehicle
    std::string vehicleType = CSpawnSubmenu::vehicleData.vehicleType;

    // Calculate the minimum volume based on the distance and vehicle type
    float minVolume;
    if (vehicleType == "car" || vehicleType == "bike" || vehicleType == "boat" || vehicleType == "tank") {
        minVolume = 0.15f - (distance * 0.006f);
    } else {
        minVolume = 0.15f - (distance * 0.003f);
    }

    if (minVolume < 0.0f) {
        minVolume = 0.0f;
    }

    // Calculate the volume based on speed
    float volume = speed / maxSpeed * speedToVolumeRatio;

    // Ensure volume doesn't go below the calculated minVolume and doesn't exceed MAX_VOLUME
    if (volume < minVolume) {
        volume = minVolume;
    } else if (volume > MAX_VOLUME) {
        volume = MAX_VOLUME;
    }

    return volume;
}


float SoundEngine::CalculatePitch(float speed) {
    float pitchSemitones = speed / maxSpeed * speedToPitchRatio;
    return pow(2, pitchSemitones / 12);
}
void SoundEngine::GetSoundFiles(std::string& idleSound, std::string& startSound, std::string& stopSound) {
    // Determine the sound files based on the vehicle's sound type
    if (CSpawnSubmenu::vehicleData.soundType == "truck") {
        idleSound = "RDR2AddonVehicleSounds\\truckidle.wav";
        startSound = "RDR2AddonVehicleSounds\\truckstart.wav";
        stopSound = "RDR2AddonVehicleSounds\\truckstop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "bigtruck") {
        idleSound = "RDR2AddonVehicleSounds\\bigtruckidle.wav";
        startSound = "RDR2AddonVehicleSounds\\bigtruckstart.wav";
        stopSound = "RDR2AddonVehicleSounds\\bigtruckstop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "bat") {
        idleSound = "RDR2AddonVehicleSounds\\batidle.wav";
        startSound = "RDR2AddonVehicleSounds\\batstart.wav";
        stopSound = "RDR2AddonVehicleSounds\\batstop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "camaro") {
        idleSound = "RDR2AddonVehicleSounds\\camaroidle.wav";
        startSound = "RDR2AddonVehicleSounds\\camarostart.wav";
        stopSound = "RDR2AddonVehicleSounds\\camarostop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "electric") {
        idleSound = "RDR2AddonVehicleSounds\\electricidle.wav";
        startSound = "RDR2AddonVehicleSounds\\electricstart.wav";
        stopSound = "RDR2AddonVehicleSounds\\electricstop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "hellcat") {
        idleSound = "RDR2AddonVehicleSounds\\hellcatidle.wav";
        startSound = "RDR2AddonVehicleSounds\\hellcatstart.wav";
        stopSound = "RDR2AddonVehicleSounds\\hellcatstop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "heli") {
        idleSound = "RDR2AddonVehicleSounds\\heliidle.wav";
        startSound = "RDR2AddonVehicleSounds\\helistart.wav";
        stopSound = "RDR2AddonVehicleSounds\\helistop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "chopperbike") {
        idleSound = "RDR2AddonVehicleSounds\\chopperbikeidle.wav";
        startSound = "RDR2AddonVehicleSounds\\chopperbikestart.wav";
        stopSound = "RDR2AddonVehicleSounds\\chopperbikestop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "moped") {
        idleSound = "RDR2AddonVehicleSounds\\mopedidle.wav";
        startSound = "RDR2AddonVehicleSounds\\mopedstart.wav";
        stopSound = "RDR2AddonVehicleSounds\\mopedstop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "mustang") {
        idleSound = "RDR2AddonVehicleSounds\\mustangidle.wav";
        startSound = "RDR2AddonVehicleSounds\\mustangstart.wav";
        stopSound = "RDR2AddonVehicleSounds\\mustangstop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "oldtruck") {
        idleSound = "RDR2AddonVehicleSounds\\oldtruckidle.wav";
        startSound = "RDR2AddonVehicleSounds\\oldtruckstart.wav";
        stopSound = "RDR2AddonVehicleSounds\\oldtruckstop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "lambo") {
        idleSound = "RDR2AddonVehicleSounds\\lamboidle.wav";
        startSound = "RDR2AddonVehicleSounds\\lambostart.wav";
        stopSound = "RDR2AddonVehicleSounds\\lambostop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "lancer") {
        idleSound = "RDR2AddonVehicleSounds\\lanceridle.wav";
        startSound = "RDR2AddonVehicleSounds\\lancerstart.wav";
        stopSound = "RDR2AddonVehicleSounds\\lancerstop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "plane") {
        idleSound = "RDR2AddonVehicleSounds\\planeidle.wav";
        startSound = "RDR2AddonVehicleSounds\\planestart.wav";
        stopSound = "RDR2AddonVehicleSounds\\planestop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "jet") {
        idleSound = "RDR2AddonVehicleSounds\\jetidle.wav";
        startSound = "RDR2AddonVehicleSounds\\jetstart.wav";
        stopSound = "RDR2AddonVehicleSounds\\jetstop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "speedboat") {
        idleSound = "RDR2AddonVehicleSounds\\speedboatidle.wav";
        startSound = "RDR2AddonVehicleSounds\\speedboatstart.wav";
        stopSound = "RDR2AddonVehicleSounds\\speedboatstop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "sport") {
        idleSound = "RDR2AddonVehicleSounds\\sportcaridle.wav";
        startSound = "RDR2AddonVehicleSounds\\sportcarstart.wav";
        stopSound = "RDR2AddonVehicleSounds\\sportcarstop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "vintage") {
        idleSound = "RDR2AddonVehicleSounds\\vintageidle.wav";
        startSound = "RDR2AddonVehicleSounds\\vintagestart.wav";
        stopSound = "RDR2AddonVehicleSounds\\vintagestop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "xwing") {
        idleSound = "RDR2AddonVehicleSounds\\xwingidle.wav";
        startSound = "RDR2AddonVehicleSounds\\xwingstart.wav";
        stopSound = "RDR2AddonVehicleSounds\\xwingstop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "jetski") {
        idleSound = "RDR2AddonVehicleSounds\\jetskiidle.wav";
        startSound = "RDR2AddonVehicleSounds\\jetskistart.wav";
        stopSound = "RDR2AddonVehicleSounds\\jetskistop.wav";
    } else if (CSpawnSubmenu::vehicleData.soundType == "dirtbike") {
        idleSound = "RDR2AddonVehicleSounds\\dirtbikeidle.wav";
        startSound = "RDR2AddonVehicleSounds\\dirtbikestart.wav";
        stopSound = "RDR2AddonVehicleSounds\\dirtbikestop.wav";
    } else {
        idleSound = "RDR2AddonVehicleSounds\\batidle.wav";
        startSound = "RDR2AddonVehicleSounds\\f15078start.wav";
        stopSound = "RDR2AddonVehicleSounds\\f15078stop.wav";
    }
}