// Licensed under the MIT License.

#pragma once
#include "SubmenuInclude.h"
#include "../UI/Drawing.h"

// Define VehicleData struct
struct VehicleData {
        std::string vehicleKey; // Vehicle key
        std::string vehicleType; // I REALLY NEED TO INITIALIZE THIS 
        std::string soundType; // I REALLY NEED TO INITIALIZE THIS 
        std::string weaponType; // I REALLY NEED TO INITIALIZE THIS 

    Object objectModel; // Single object model
    Vehicle underVehicle; // Single under vehicle
    std::vector<Object> frontWheels; // Multiple front wheels
    std::vector<Object> rearWheels; // Multiple rear wheels
    std::vector<Vector3> frontWheelOffsets; // Offsets for front wheels
    std::vector<Vector3> rearWheelOffsets; // Offsets for rear wheels
    std::vector<Object> exhausts; // Multiple exhausts
    std::vector<Vector3> exhaustOffsets; // Offsets for rear wheels
    std::vector<Object> headlights; // Multiple headlights
    std::vector<Object> brakelights; // Multiple brakelights
    Vector3 cloneOffsets; // Offsets for the clone
    Vector3 cloneRotation; // Rotation for the clone
    Ped clonePed; // Add this line
    bool playerInVehicle = false; // Add this line
    std::vector<Object> propellers; //  propeller object
    std::vector<Vector3> propellerOffsets; // Changed to vector for multiple offsets
    std::vector<Vector3> propellerRotations; // Change this line
    std::vector<Object> topblades; // Multiple headlights
    std::vector<Vector3> topbladeOffsets; // Multiple brakelights
    std::vector<Vector3> topbladeRotations; // Multiple brakelights
    std::vector<Object> rearblades; // Multiple headlights
    std::vector<Vector3> rearbladeOffsets; // Multiple brakelights
        std::vector<Vector3> rearbladeRotations; // Multiple brakelights

    Object cargobobDoor; // Single door object
    Object cargobobfrontBlade; // Single front blade object
    Object cargobobrearBlade; // Single rear blade object
            Vector3 cargobobfrontbladeOffsets; // Offsets for the clone
            Vector3 cargobobrearbladeOffsets; // Offsets for the clone
                        Vector3 cargobobdoorOffsets; // Offsets for the clone
                        Vector3 cargobobdoorRotation; // Offsets for the clone
                        Object ospreyDoor; // Single door object
                        Vector3 ospreyDoorOffsets; // Offsets for the clone
                        Vector3 ospreyDoorRotation; // Offsets for the clone
                        Object leftThruster; // Single door object
                        Vector3 leftThrusterOffsets; // Offsets for the clone
                        Object rightThruster; // Single door object
                        Vector3 rightThrusterOffsets; // Offsets for the clone

                        Object leftPropeller; // Single door object
                        Vector3 leftbladeOffsets; // Offsets for the clone
                        Object rightPropeller; // Single door object
                        Vector3 rightbladeOffsets; // Offsets for the clone
                        Object tankTop; // Single door object
                        Vector3 tankTopOffsets; // Offsets for the clone
                        Vector3 tankTopRotations; // Offsets for the clone
                        bool engine = false;
                        std::vector<Vector3> updatedFrontWheelOffsets;
std::vector<Vector3> updatedRearWheelOffsets;
float currspeed;
    bool horseAttached = false; // New flag to track attachment state

};

class CSpawnSubmenu {
public:
    ~CSpawnSubmenu();
    void SpawnVehicle(const std::string& vehicleKey);
        void DeleteVehicle();
            void UpdateWheels(); // New update loop declaration
                 void TrackVehicleEntryExit(); // Declaration of the new function
                 void IncreaseVehicleSpeed();
                 void PlaneUpdate();
                  void HeliUpdate();
                 void OspreyUpdate();
                 void TankUpdate();
                 void ToggleEngine();
				 void DisplayVehicleStatus();
                 void FirstPersonCamAndRadius();
                 void VehWeaponsUpdate();
                 void HorseFinder();
                 void HoverCar();
                 void AdjustAircraftPitch(float pitchDirection); // Declaration of the new function
    void AdjustAircraftOrientation(float rollDirection, float yawDirection);
       static VehicleData vehicleData; // Single instance to store VehicleData

    
private:


    float currentBaseSpeed = 0.0f; // Initialize currentBaseSpeed
    float maxBaseSpeed = 3.2f;
    	float maxSpeedMPH = 40.0f;
    float maxSpeedMetersPerSecond = maxSpeedMPH / 2.23694f;

	float speedIncrement = 0.005f;
    float currentBladeRotationAngle = 0.0f;
    float currentForce = 0.0f;
    float decayRate = 0.02f;
    bool doorOpen = false;
    const float maxCargobobDoorRotationAngle = 50.0f; // Maximum angle to open the OSPREY door
    const float doorRotationSpeed = 1.0f; // Speed of rotation
    float currentDoorRotationAngle = 0.0f;
    float doorturnSmoothing = 20.0f;
    const float maxOspreyDoorRotationAngle = 34.0f;
    bool flightMode = false;
    bool thrusterRotated = false;
    float thrusterTargetRotationAngle = 0.0f;
    float thrusterCurrentRotationAngle = 0.0f;
    float thrusterRotationSpeed = 1.0f;
    std::string newPtfxDictionary = "core";
    std::string newPtfxName = "ent_amb_smoke_cabin";
    bool isParticleEffectActive = false;
    std::vector<int> currentPtfxHandles;
        bool firstEntry = true;
        bool animationPlayed = false;
		int currentStateIndex = 1;
        std::vector<float> distanceStates; // Change to vector<float>
    float camDistance = !distanceStates.empty() ? distanceStates[currentStateIndex] : 1.0f;
};

class COptionsSubmenu {
public:
    static bool bPauseTime;
    void Init();
};

// Declarations
extern CSpawnSubmenu* g_SpawnSubmenu;
extern COptionsSubmenu* g_OptionsSubmenu;
extern nlohmann::json g_Config;