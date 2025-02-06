#include "Examples.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
bool autoWarp = true;
bool seatbelt = true;
bool uiToggle = true;
bool attachHorse = false;
bool hoverCar = false;
using json = nlohmann::json;

CSpawnSubmenu* g_SpawnSubmenu = new CSpawnSubmenu();
COptionsSubmenu* g_OptionsSubmenu = new COptionsSubmenu();

CSpawnSubmenu::~CSpawnSubmenu() {
    DeleteVehicle();
}
VehicleData CSpawnSubmenu::vehicleData;
void CSpawnSubmenu::SpawnVehicle(const std::string& vehicleKey) {
    // Delete existing veh
    DeleteVehicle();
    // Fetch the objectModel from config.json 

    std::string objectModelStr = g_Config["Config"]["Vehicles"][vehicleKey]["objectModel"];
    auto attachOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["attachOffsets"];
    Vector3 attachPosition(attachOffsets[0], attachOffsets[1], attachOffsets[2]);
    Vector3 attachRotation(attachOffsets[3], attachOffsets[4], attachOffsets[5]);
    // Fetch clone offsets from config.json
    auto cloneOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["cloneOffsets"];
    Vector3 clonePosition(cloneOffsets["x"], cloneOffsets["y"], cloneOffsets["z"]);
    Vector3 cloneRotation(cloneOffsets["rx"], cloneOffsets["ry"], cloneOffsets["rz"]);

    // Store the clone offsets and rotation in vehicleData
    vehicleData.cloneOffsets = clonePosition;
    vehicleData.cloneRotation = cloneRotation;

    Hash objectModelHash = joaat(objectModelStr.c_str());

    STREAMING::REQUEST_MODEL(objectModelHash, NULL);

    while (!STREAMING::HAS_MODEL_LOADED(objectModelHash)) {
        WAIT(5);
    }

    vehicleData.objectModel = OBJECT::CREATE_OBJECT(objectModelHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true) + Vector3(2, 0, 0), true, true, true, false, false);
    vehicleData.vehicleKey = vehicleKey;
    std::string vehicleType = g_Config["Config"]["Vehicles"][vehicleKey]["type"];
    vehicleData.vehicleType = vehicleType;
    std::string soundType = g_Config["Config"]["Vehicles"][vehicleKey]["soundType"];
    vehicleData.soundType = soundType;
    if (g_Config["Config"]["Vehicles"][vehicleKey].contains("weaponType")) {
    std::string weaponType = g_Config["Config"]["Vehicles"][vehicleKey]["weaponType"];
    vehicleData.weaponType = weaponType;
} else {
    vehicleData.weaponType = "none";
};
    if (vehicleType == "car" || vehicleType == "bike") {
        Hash underVehicleHash;
        if (g_Config["Config"]["AltHandling"].get<bool>()) {
            underVehicleHash = joaat("buggy01");
        } else {
            underVehicleHash = joaat("coach4");
        }
        STREAMING::REQUEST_MODEL(underVehicleHash, NULL);

        while (!STREAMING::HAS_MODEL_LOADED(underVehicleHash)) {
            WAIT(5);
        }

        vehicleData.underVehicle = VEHICLE::CREATE_VEHICLE(underVehicleHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true) + Vector3(2, 0, 0), ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()), true, true, false, false);
        ENTITY::FREEZE_ENTITY_POSITION(vehicleData.underVehicle, false);
        ENTITY::SET_ENTITY_VISIBLE(vehicleData.underVehicle, false);
        if (g_Config["Config"]["AltHandling"].get<bool>()) {
                attachPosition.z += 0.2f;
		}
        ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.objectModel, vehicleData.underVehicle, 0, attachPosition, attachRotation, NULL, true, true, false, 0, true, NULL, NULL);

        // Fetch wheelModels from config
        std::string frontWheelModelStr = g_Config["Config"]["Vehicles"][vehicleKey]["frontWheelModel"];
        std::string rearWheelModelStr = g_Config["Config"]["Vehicles"][vehicleKey]["rearWheelModel"];

        Hash frontWheelModel = joaat(frontWheelModelStr.c_str());
        Hash rearWheelModel = joaat(rearWheelModelStr.c_str());

        STREAMING::REQUEST_MODEL(frontWheelModel, NULL);
        STREAMING::REQUEST_MODEL(rearWheelModel, NULL);

        while (!STREAMING::HAS_MODEL_LOADED(frontWheelModel) || !STREAMING::HAS_MODEL_LOADED(rearWheelModel)) {
            WAIT(5);
        }

        // Create wheels
        auto frontWheelOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["frontWheelOffsets"];
        auto rearWheelOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["rearWheelOffsets"];

                // Store the offsets in vehicleData
        for (const auto& offset : frontWheelOffsets) {
            vehicleData.frontWheelOffsets.push_back(Vector3(offset["x"], offset["y"], offset["z"]));
        }
        for (const auto& offset : rearWheelOffsets) {
            vehicleData.rearWheelOffsets.push_back(Vector3(offset["x"], offset["y"], offset["z"]));
        }

        for (const auto& offset : frontWheelOffsets) {
            Vector3 wheelPosition(offset["x"], offset["y"], offset["z"]);
            float orientationAngle = (offset["x"] > 0) ? 180.0f : 0.0f;
            Object frontWheel = OBJECT::CREATE_OBJECT(frontWheelModel, ENTITY::GET_ENTITY_COORDS(vehicleData.objectModel, NULL, true) + wheelPosition, true, true, true, false, false);
            ENTITY::ATTACH_ENTITY_TO_ENTITY(frontWheel, vehicleData.objectModel, 0, wheelPosition, Vector3(0, 0, orientationAngle), NULL, true, true, false, 0, true, NULL, NULL);
            vehicleData.frontWheels.push_back(frontWheel);
        }
        for (const auto& offset : rearWheelOffsets) {
            Vector3 wheelPosition(offset["x"], offset["y"], offset["z"]);
            float orientationAngle = (offset["x"] > 0) ? 180.0f : 0.0f;
            Object rearWheel = OBJECT::CREATE_OBJECT(rearWheelModel, ENTITY::GET_ENTITY_COORDS(vehicleData.objectModel, NULL, true) + wheelPosition, true, true, true, false, false);
            ENTITY::ATTACH_ENTITY_TO_ENTITY(rearWheel, vehicleData.objectModel, 0, wheelPosition, Vector3(0, 0, orientationAngle), NULL, true, true, false, 0, true, NULL, NULL);
            vehicleData.rearWheels.push_back(rearWheel);
        }

        Hash exhaustModel = joaat("p_pebble04x");
        STREAMING::REQUEST_MODEL(exhaustModel, NULL);

        while (!STREAMING::HAS_MODEL_LOADED(exhaustModel)) {
            WAIT(5);
        }
        auto exhaustOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["exhaustOffsets"];
                // Create exhausts- a pebble is used, change to 01x to debug offsets
if (g_Config["Config"]["Vehicles"][vehicleKey].contains("exhaustOffsets")) {
    auto exhaustOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["exhaustOffsets"];
    for (const auto& offset : exhaustOffsets) {
        Vector3 exhaustPosition(offset["x"], offset["y"], offset["z"]);
        Object exhaust = OBJECT::CREATE_OBJECT(exhaustModel, ENTITY::GET_ENTITY_COORDS(vehicleData.objectModel, NULL, true) + exhaustPosition, true, true, true, false, false);
        ENTITY::ATTACH_ENTITY_TO_ENTITY(exhaust, vehicleData.objectModel, 0, exhaustPosition, Vector3(0, 0, 0), NULL, true, true, false, 0, true, NULL, NULL);
        vehicleData.exhausts.push_back(exhaust);
                    vehicleData.exhaustOffsets.push_back(Vector3(offset["x"], offset["y"], offset["z"]));

    }   

        }

        VEHICLE::SET_VEHICLE_LIGHTS(vehicleData.underVehicle, 1);

        // headlights and brake lights
        Hash headlightModel = joaat("p_steamerlight01x");
        Hash brakelightModel = joaat("p_stageshelllight_red01x");
        STREAMING::REQUEST_MODEL(headlightModel, NULL);
        STREAMING::REQUEST_MODEL(brakelightModel, NULL);
        while (!STREAMING::HAS_MODEL_LOADED(headlightModel) || !STREAMING::HAS_MODEL_LOADED(brakelightModel)) {
    WAIT(5);
}

        auto headlightOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["headlightOffsets"];
        for (const auto& offset : headlightOffsets) {
            Vector3 headlightPosition(offset["x"], offset["y"], offset["z"]);
            Vector3 headlightRotation(offset["rx"], offset["ry"], offset["rz"]);
            Object headlight = OBJECT::CREATE_OBJECT(headlightModel, ENTITY::GET_ENTITY_COORDS(vehicleData.objectModel, NULL, true) + headlightPosition, true, true, true, false, false);
            ENTITY::ATTACH_ENTITY_TO_ENTITY(headlight, vehicleData.objectModel, 0, headlightPosition, headlightRotation, NULL, true, true, false, 0, true, NULL, NULL);
            vehicleData.headlights.push_back(headlight);
            ENTITY::SET_ENTITY_VISIBLE(headlight, false);
        }
        auto brakelightOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["brakelightOffsets"];
        for (const auto& offset : brakelightOffsets) {
        Vector3 brakelightPosition(offset["x"], offset["y"], offset["z"]);
        Vector3 brakelightRotation(offset["rx"], offset["ry"], offset["rz"]);
        Object brakelight = OBJECT::CREATE_OBJECT(brakelightModel, ENTITY::GET_ENTITY_COORDS(vehicleData.objectModel, NULL, true) + brakelightPosition, true, true, true, false, false);
        ENTITY::ATTACH_ENTITY_TO_ENTITY(brakelight, vehicleData.objectModel, 0, brakelightPosition, brakelightRotation, NULL, true, true, false, 0, true, NULL, NULL);
        vehicleData.brakelights.push_back(brakelight);
        ENTITY::SET_ENTITY_VISIBLE(brakelight, false);
        }
		if (autoWarp) {
                TASK::TASK_WARP_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), vehicleData.underVehicle, -1);
        }

	}
    if (vehicleType == "boat") {
    if (ENTITY::IS_ENTITY_IN_WATER(PLAYER::PLAYER_PED_ID())) {
        Hash underVehicleHash = joaat("boatsteam02x");
        STREAMING::REQUEST_MODEL(underVehicleHash, NULL);

        while (!STREAMING::HAS_MODEL_LOADED(underVehicleHash)) {
            WAIT(5);
        }

        vehicleData.underVehicle = VEHICLE::CREATE_VEHICLE(underVehicleHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true) + Vector3(2, 0, 0), ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()), true, true, false, false);
        ENTITY::FREEZE_ENTITY_POSITION(vehicleData.underVehicle, false);
        ENTITY::SET_ENTITY_VISIBLE(vehicleData.underVehicle, false);
        ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.objectModel, vehicleData.underVehicle, 0, attachPosition, attachRotation, NULL, true, true, false, 0, true, NULL, NULL);
        if (autoWarp) {
            TASK::TASK_WARP_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), vehicleData.underVehicle, -1);
        }
    } else {
        UIUtil::PrintSubtitle("You must be in ~COLOR_BLUE~water~s~ to spawn this");
        if (ENTITY::DOES_ENTITY_EXIST(vehicleData.objectModel)) {
        ENTITY::DELETE_ENTITY(&vehicleData.objectModel);
    }
    }
}

    if (vehicleType == "jet") {
        Hash underVehicleHash = joaat("boatsteam02x");
        STREAMING::REQUEST_MODEL(underVehicleHash, NULL);

        while (!STREAMING::HAS_MODEL_LOADED(underVehicleHash)) {
            WAIT(5);
        }

        vehicleData.underVehicle = VEHICLE::CREATE_VEHICLE(underVehicleHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true) + Vector3(2, 0, 0), ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()), true, true, false, false);
        ENTITY::FREEZE_ENTITY_POSITION(vehicleData.underVehicle, false);
        ENTITY::SET_ENTITY_VISIBLE(vehicleData.underVehicle, false);
        ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.objectModel, vehicleData.underVehicle, 0, attachPosition, attachRotation, NULL, true, true, false, 0, true, NULL, NULL);
		if (autoWarp) {
                TASK::TASK_WARP_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), vehicleData.underVehicle, -1);
        }
    }
	 // PLANE SPAWN LOGIC
    if (vehicleType == "plane") {
        Hash underVehicleHash = joaat("boatsteam02x");
        STREAMING::REQUEST_MODEL(underVehicleHash, NULL);

        while (!STREAMING::HAS_MODEL_LOADED(underVehicleHash)) {
            WAIT(5);
        }

        vehicleData.underVehicle = VEHICLE::CREATE_VEHICLE(underVehicleHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true) + Vector3(2, 0, 0), ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()), true, true, false, false);
        ENTITY::FREEZE_ENTITY_POSITION(vehicleData.underVehicle, false);
        ENTITY::SET_ENTITY_VISIBLE(vehicleData.underVehicle, false);
        ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.objectModel, vehicleData.underVehicle, 0, attachPosition, attachRotation, NULL, true, true, false, 0, true, NULL, NULL);
		if (autoWarp) {
                TASK::TASK_WARP_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), vehicleData.underVehicle, -1);
        }                std::string propellerModelStr = g_Config["Config"]["Vehicles"][vehicleKey]["propellerModel"];
                Hash propellerModelHash = joaat(propellerModelStr.c_str());

                STREAMING::REQUEST_MODEL(propellerModelHash, NULL);
        while (!STREAMING::HAS_MODEL_LOADED(propellerModelHash)) {
            WAIT(5); }
                        

        auto propellerOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["propellerOffsets"];

       Vector3 propellerPosition(propellerOffsets["x"], propellerOffsets["y"], propellerOffsets["z"]);
       Vector3 propellerRotation(propellerOffsets["rx"], propellerOffsets["ry"], propellerOffsets["rz"]);
       Object propellerModel = OBJECT::CREATE_OBJECT(propellerModelHash, ENTITY::GET_ENTITY_COORDS(vehicleData.objectModel, NULL, true), true, true, true, false, false);

       ENTITY::ATTACH_ENTITY_TO_ENTITY(propellerModel, vehicleData.underVehicle, 0, propellerPosition, propellerRotation, NULL, true, true, false, 0, true, NULL, NULL);
       vehicleData.propellers.push_back(propellerModel);
            vehicleData.propellerOffsets.push_back(Vector3(propellerOffsets["x"], propellerOffsets["y"], propellerOffsets["z"]));
                        vehicleData.propellerRotations.push_back(Vector3(propellerOffsets["rx"], propellerOffsets["ry"], propellerOffsets["rz"]));


    }
    if (vehicleType == "heli") {
        Hash underVehicleHash = joaat("boatsteam02x");
        STREAMING::REQUEST_MODEL(underVehicleHash, NULL);

        while (!STREAMING::HAS_MODEL_LOADED(underVehicleHash)) {
            WAIT(5);
        }

        vehicleData.underVehicle = VEHICLE::CREATE_VEHICLE(underVehicleHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true) + Vector3(2, 0, 0), ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()), true, true, false, false);
        ENTITY::FREEZE_ENTITY_POSITION(vehicleData.underVehicle, false);
        ENTITY::SET_ENTITY_VISIBLE(vehicleData.underVehicle, false);
        ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.objectModel, vehicleData.underVehicle, 0, attachPosition, attachRotation, NULL, true, true, false, 0, true, NULL, NULL);
		if (autoWarp) {
                TASK::TASK_WARP_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), vehicleData.underVehicle, -1);
        }    // Request and load the top propeller model
    std::string topPropellerModelStr = g_Config["Config"]["Vehicles"][vehicleKey]["topbladeModel"];
    Hash topPropellerModelHash = joaat(topPropellerModelStr.c_str());
    STREAMING::REQUEST_MODEL(topPropellerModelHash, NULL);

    while (!STREAMING::HAS_MODEL_LOADED(topPropellerModelHash)) {
        WAIT(5);
    }

    // Define the top propeller offset
    auto topPropellerOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["topbladeOffsets"];
    Vector3 topPropellerOffset(topPropellerOffsets["x"], topPropellerOffsets["y"], topPropellerOffsets["z"]);
        Vector3 topPropellerRotation(topPropellerOffsets["rx"], topPropellerOffsets["ry"], topPropellerOffsets["rz"]);

    // Create and attach the top propeller
    Object topPropeller = OBJECT::CREATE_OBJECT(topPropellerModelHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true), true, true, true, false, false);
    ENTITY::ATTACH_ENTITY_TO_ENTITY(topPropeller, vehicleData.underVehicle, 0, topPropellerOffset, topPropellerRotation, NULL, true, true, false, 0, true, NULL, NULL);
    vehicleData.topblades.push_back(topPropeller);
    vehicleData.topbladeOffsets.push_back(topPropellerOffset);
        vehicleData.propellerRotations.push_back(topPropellerRotation);


    // Request and load the rear blade model
    std::string rearBladeModelStr = g_Config["Config"]["Vehicles"][vehicleKey]["rearbladeModel"];
    Hash rearBladeModelHash = joaat(rearBladeModelStr.c_str());
    STREAMING::REQUEST_MODEL(rearBladeModelHash, NULL);

    while (!STREAMING::HAS_MODEL_LOADED(rearBladeModelHash)) {
        WAIT(5);
    }

    // Define the rear blade offset
    auto rearBladeOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["rearbladeOffsets"];
    Vector3 rearBladeOffset(rearBladeOffsets["x"], rearBladeOffsets["y"], rearBladeOffsets["z"]);
        Vector3 rearBladeRotation(rearBladeOffsets["rx"], rearBladeOffsets["ry"], rearBladeOffsets["rz"]);

    // Create and attach the rear blade
    Object rearBlade = OBJECT::CREATE_OBJECT(rearBladeModelHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true), true, true, true, false, false);
    ENTITY::ATTACH_ENTITY_TO_ENTITY(rearBlade, vehicleData.underVehicle, 0, rearBladeOffset, rearBladeRotation, NULL, true, true, false, 0, true, NULL, NULL);
    vehicleData.rearblades.push_back(rearBlade);
    vehicleData.rearbladeOffsets.push_back(rearBladeOffset);
        vehicleData.rearbladeRotations.push_back(rearBladeRotation);

}
    if (vehicleType == "cargobob") {
        Hash underVehicleHash = joaat("boatsteam02x");
        STREAMING::REQUEST_MODEL(underVehicleHash, NULL);

        while (!STREAMING::HAS_MODEL_LOADED(underVehicleHash)) {
            WAIT(5);
        }

        vehicleData.underVehicle = VEHICLE::CREATE_VEHICLE(underVehicleHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true) + Vector3(2, 0, 0), ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()), true, true, false, false);
        ENTITY::FREEZE_ENTITY_POSITION(vehicleData.underVehicle, false);
        ENTITY::SET_ENTITY_VISIBLE(vehicleData.underVehicle, false);
        ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.objectModel, vehicleData.underVehicle, 0, attachPosition, attachRotation, NULL, true, true, false, 0, true, NULL, NULL);
        // Fetch the doorModel from config.json
    std::string doorModelStr = g_Config["Config"]["Vehicles"][vehicleKey]["doorModel"];
    Hash doorModelHash = joaat(doorModelStr.c_str());

    STREAMING::REQUEST_MODEL(doorModelHash, NULL);

    while (!STREAMING::HAS_MODEL_LOADED(doorModelHash)) {
        WAIT(5);
    }

    // Create the door object
    auto doorOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["doorattachOffsets"];
vehicleData.cargobobdoorOffsets = Vector3(doorOffsets[0], doorOffsets[1], doorOffsets[2]);
    vehicleData.cargobobdoorRotation = Vector3(doorOffsets[3], doorOffsets[4], doorOffsets[5]);

    Object cargobobDoor = OBJECT::CREATE_OBJECT(doorModelHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true) + vehicleData.cargobobdoorOffsets, true, true, true, false, false);
    ENTITY::ATTACH_ENTITY_TO_ENTITY(cargobobDoor, vehicleData.underVehicle, 0, vehicleData.cargobobdoorOffsets, vehicleData.cargobobdoorRotation, NULL, true, true, false, 0, true, NULL, NULL);

    // Store the door object in vehicleData
    vehicleData.cargobobDoor = cargobobDoor;
    // Fetch the bladeModel from config.json
    std::string bladeModelStr = g_Config["Config"]["Vehicles"][vehicleKey]["bladeModel"];
    Hash bladeModelHash = joaat(bladeModelStr.c_str());

    STREAMING::REQUEST_MODEL(bladeModelHash, NULL);

    while (!STREAMING::HAS_MODEL_LOADED(bladeModelHash)) {
        WAIT(5);
    }

    // Create the front blade object
    auto frontBladeOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["frontbladeOffsets"];
    
    Vector3 frontBladeRotation(0.0f, 0.0f, 0.0f); // Adjust rotation as needed
        vehicleData.cargobobfrontbladeOffsets = Vector3(frontBladeOffsets["x"], frontBladeOffsets["y"], frontBladeOffsets["z"]);

    Object cargobobfrontBlade = OBJECT::CREATE_OBJECT(bladeModelHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true) + vehicleData.cargobobfrontbladeOffsets, true, true, true, false, false);
    ENTITY::ATTACH_ENTITY_TO_ENTITY(cargobobfrontBlade, vehicleData.underVehicle, 0, vehicleData.cargobobfrontbladeOffsets, frontBladeRotation, NULL, true, true, false, 0, true, NULL, NULL);

    // Store the front blade object in vehicleData
    vehicleData.cargobobfrontBlade = cargobobfrontBlade;

    // Create the rear blade object
    auto rearBladeOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["rearbladeOffsets"];
    vehicleData.cargobobrearbladeOffsets = Vector3(rearBladeOffsets["x"], rearBladeOffsets["y"], rearBladeOffsets["z"]);
    Vector3 rearBladeRotation(0.0f, 0.0f, 0.0f); // Adjust rotation as needed

    Object cargobobrearBlade = OBJECT::CREATE_OBJECT(bladeModelHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true) + vehicleData.cargobobrearbladeOffsets, true, true, true, false, false);
    ENTITY::ATTACH_ENTITY_TO_ENTITY(cargobobrearBlade, vehicleData.underVehicle, 0, vehicleData.cargobobrearbladeOffsets, rearBladeRotation, NULL, true, true, false, 0, true, NULL, NULL);

    // Store the rear blade object in vehicleData
    vehicleData.cargobobrearBlade = cargobobrearBlade;
    if (autoWarp) {
                TASK::TASK_WARP_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), vehicleData.underVehicle, -1);
        }
}
    if (vehicleType == "osprey") {
        Hash underVehicleHash = joaat("boatsteam02x");
        STREAMING::REQUEST_MODEL(underVehicleHash, NULL);

        while (!STREAMING::HAS_MODEL_LOADED(underVehicleHash)) {
            WAIT(5);
        }

        vehicleData.underVehicle = VEHICLE::CREATE_VEHICLE(underVehicleHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true) + Vector3(2, 0, 0), ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()), true, true, false, false);
        ENTITY::FREEZE_ENTITY_POSITION(vehicleData.underVehicle, false);
        ENTITY::SET_ENTITY_VISIBLE(vehicleData.underVehicle, false);
        ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.objectModel, vehicleData.underVehicle, 0, attachPosition, attachRotation, NULL, true, true, false, 0, true, NULL, NULL);
        		if (autoWarp) {
                TASK::TASK_WARP_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), vehicleData.underVehicle, -1);
        }
    // Load models for osprey
        std::string bladeModelStr = g_Config["Config"]["Vehicles"][vehicleKey]["bladeModel"];
        std::string doorModelStr = g_Config["Config"]["Vehicles"][vehicleKey]["doorModel"];
        std::string leftThrusterModelStr = g_Config["Config"]["Vehicles"][vehicleKey]["leftthrusterModel"];
        std::string rightThrusterModelStr = g_Config["Config"]["Vehicles"][vehicleKey]["rightthrusterModel"];

        Hash bladeModelHash = joaat(bladeModelStr.c_str());
        Hash doorModelHash = joaat(doorModelStr.c_str());
        Hash leftThrusterModelHash = joaat(leftThrusterModelStr.c_str());
        Hash rightThrusterModelHash = joaat(rightThrusterModelStr.c_str());

        STREAMING::REQUEST_MODEL(bladeModelHash, NULL);
        STREAMING::REQUEST_MODEL(doorModelHash, NULL);
        STREAMING::REQUEST_MODEL(leftThrusterModelHash, NULL);
        STREAMING::REQUEST_MODEL(rightThrusterModelHash, NULL);

        while (!STREAMING::HAS_MODEL_LOADED(bladeModelHash) || !STREAMING::HAS_MODEL_LOADED(doorModelHash) ||
               !STREAMING::HAS_MODEL_LOADED(leftThrusterModelHash) || !STREAMING::HAS_MODEL_LOADED(rightThrusterModelHash)) {
            WAIT(5);
        }

        auto leftbladeOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["leftbladeOffsets"];
        vehicleData.leftbladeOffsets = Vector3(leftbladeOffsets["x"], leftbladeOffsets["y"], leftbladeOffsets["z"]);

        auto rightbladeOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["rightbladeOffsets"];
        vehicleData.rightbladeOffsets = Vector3(rightbladeOffsets["x"], rightbladeOffsets["y"], rightbladeOffsets["z"]);

        auto doorattachOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["doorattachOffsets"];
        vehicleData.ospreyDoorOffsets = Vector3(doorattachOffsets[0], doorattachOffsets[1], doorattachOffsets[2]);

        auto leftthrusterattachOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["leftthrusterattachOffsets"];
                vehicleData.leftThrusterOffsets = Vector3(leftthrusterattachOffsets["x"], leftthrusterattachOffsets["y"], leftthrusterattachOffsets["z"]);

        auto rightthrusterattachOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["rightthrusterattachOffsets"];
                vehicleData.rightThrusterOffsets = Vector3(rightthrusterattachOffsets["x"], rightthrusterattachOffsets["y"], rightthrusterattachOffsets["z"]);

        vehicleData.leftPropeller = OBJECT::CREATE_OBJECT(bladeModelHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true) + Vector3(2, 0, 0), true, true, true, false, false);
        vehicleData.rightPropeller = OBJECT::CREATE_OBJECT(bladeModelHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true) + Vector3(2, 0, 0), true, true, true, false, false);
        vehicleData.ospreyDoor = OBJECT::CREATE_OBJECT(doorModelHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true) + Vector3(2, 0, 0), true, true, true, false, false);
        vehicleData.leftThruster = OBJECT::CREATE_OBJECT(leftThrusterModelHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true) + Vector3(2, 0, 0), true, true, true, false, false);
        vehicleData.rightThruster = OBJECT::CREATE_OBJECT(rightThrusterModelHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true) + Vector3(2, 0, 0), true, true, true, false, false);

        ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.leftPropeller, vehicleData.leftThruster, 0, Vector3(leftbladeOffsets["x"], leftbladeOffsets["y"], leftbladeOffsets["z"]), Vector3(0, 0, 0), NULL, true, true, false, 0, true, NULL, NULL);
        ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.rightPropeller, vehicleData.rightThruster, 0, Vector3(rightbladeOffsets["x"], rightbladeOffsets["y"], rightbladeOffsets["z"]), Vector3(0, 0, 0), NULL, true, true, false, 0, true, NULL, NULL);
        ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.ospreyDoor, vehicleData.underVehicle, 0, Vector3(doorattachOffsets[0], doorattachOffsets[1], doorattachOffsets[2]), Vector3(0, 0, 0), NULL, true, true, false, 0, true, NULL, NULL);
        ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.leftThruster, vehicleData.underVehicle, 0, Vector3(leftthrusterattachOffsets["x"], leftthrusterattachOffsets["y"], leftthrusterattachOffsets["z"]), Vector3(0, 0, 0), NULL, true, true, false, 0, true, NULL, NULL);
        ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.rightThruster, vehicleData.underVehicle, 0, Vector3(rightthrusterattachOffsets["x"], rightthrusterattachOffsets["y"], rightthrusterattachOffsets["z"]), Vector3(0, 0, 0), NULL, true, true, false, 0, true, NULL, NULL);
    }
    if (vehicleType == "tank") {
        Hash underVehicleHash;
        if (g_Config["Config"]["AltHandling"].get<bool>()) {
            underVehicleHash = joaat("buggy01");
        } else {
            underVehicleHash = joaat("coach4");
        }
        STREAMING::REQUEST_MODEL(underVehicleHash, NULL);

        while (!STREAMING::HAS_MODEL_LOADED(underVehicleHash)) {
            WAIT(5);
        }

        vehicleData.underVehicle = VEHICLE::CREATE_VEHICLE(underVehicleHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true) + Vector3(2, 0, 0), ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()), true, true, false, false);
        ENTITY::FREEZE_ENTITY_POSITION(vehicleData.underVehicle, false);
        ENTITY::SET_ENTITY_VISIBLE(vehicleData.underVehicle, false);

        ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.objectModel, vehicleData.underVehicle, 0, attachPosition, attachRotation, NULL, true, true, false, 0, true, NULL, NULL);
    std::string tanktopModelStr = g_Config["Config"]["Vehicles"][vehicleKey]["tanktopModel"];
    Hash tanktopModelHash = joaat(tanktopModelStr.c_str());
    STREAMING::REQUEST_MODEL(tanktopModelHash, NULL);

    while (!STREAMING::HAS_MODEL_LOADED(tanktopModelHash)) {
        WAIT(5);
    }

    vehicleData.tankTop = OBJECT::CREATE_OBJECT(tanktopModelHash, ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), NULL, true) + Vector3(2, 0, 0), true, true, true, false, false);

    auto tanktopOffsets = g_Config["Config"]["Vehicles"][vehicleKey]["tanktopOffsets"];
    vehicleData.tankTopOffsets = Vector3(tanktopOffsets["x"], tanktopOffsets["y"], tanktopOffsets["z"]);
    vehicleData.tankTopRotations = Vector3(tanktopOffsets["rx"], tanktopOffsets["ry"], tanktopOffsets["rz"]);

    ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.tankTop, vehicleData.underVehicle, 0, vehicleData.tankTopOffsets, vehicleData.tankTopRotations, NULL, true, true, false, 0, true, NULL, NULL);
    if (autoWarp) {
            TASK::TASK_WARP_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), vehicleData.underVehicle, -1);
        }
}

    g_Menu->GoToSubmenu(Submenu_EntryMenu);
}


void CSpawnSubmenu::FirstPersonCamAndRadius() {
    if (vehicleData.underVehicle) {
        if (vehicleData.playerInVehicle) {
            auto distanceStates = g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["ThirdPersonCamDistances"];

            // Initialize camDistance with the first value from distanceStates if it hasn't been set yet
            if (currentStateIndex == 1 && !distanceStates.empty()) {
                camDistance = distanceStates[0];
            }

            // Change the camera distance if V key is pressed
            if (PAD::IS_CONTROL_JUST_PRESSED(0, joaat("INPUT_NEXT_CAMERA")) && !CAM::IS_FIRST_PERSON_CAMERA_ACTIVE(0, 0, 0)) {
                currentStateIndex++;
                if (currentStateIndex > distanceStates.size()) {
                    currentStateIndex = 1;
                }

                camDistance = distanceStates[currentStateIndex - 1];
            }
            CAM::SET_THIRD_PERSON_CAM_ORBIT_DISTANCE_LIMITS_THIS_UPDATE(1.0f, camDistance);

            // Set the third-person camera distance
            if (CAM::IS_FIRST_PERSON_CAMERA_ACTIVE(0, 0, 0)) {
                // Get the head bone location of the entity "clonePed"
                int headBone = PED::GET_PED_BONE_INDEX(vehicleData.clonePed, 0x796E); // 0x796E is the bone index for "HEAD"
                Vector3 headPos = PED::GET_PED_BONE_COORDS(vehicleData.clonePed, headBone, 0.0f, 0.0f, 0.0f);
                CAM::SET_GAMEPLAY_CAM_FOLLOW_PED_THIS_UPDATE(vehicleData.clonePed);

                ENTITY::SET_ENTITY_VISIBLE(vehicleData.clonePed, false);
                TASK::CLEAR_PED_TASKS(vehicleData.clonePed, true, true);
                animationPlayed = false;  // Reset the flag when the clone is hidden
            } else {
                if (!animationPlayed) {  // Check if the animation has not been played
                    ENTITY::SET_ENTITY_VISIBLE(vehicleData.clonePed, true);
                    TASK::TASK_PLAY_ANIM(vehicleData.clonePed, "script_re@check_point@small_cart", "int_loop_driver", 8.0f, 1.0f, -1, AF_LOOPING | AF_FORCE_START, 0.1f, NULL, NULL, NULL, NULL, NULL);

                    animationPlayed = true;  // Set the flag to indicate the animation has been played
                }
            }
        }
    }
}



bool isParticleEffectActive = false;
std::vector<int> currentPtfxHandles;

void CSpawnSubmenu::ToggleEngine() {
    if (vehicleData.playerInVehicle) {
        Entity vehicleEntity = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), false);
        if (PAD::IS_CONTROL_JUST_PRESSED(0, joaat("INPUT_REVEAL_HUD")) || firstEntry) {
            if (!vehicleData.engine) {
                // Turn on the engine
                VEHICLE::SET_VEHICLE_ENGINE_ON(vehicleEntity, true, true);
                isParticleEffectActive = true;

                // Activate particle effects
                if (vehicleData.vehicleType == "car" || vehicleData.vehicleType == "bike") {
                    Hash newPtfxDictionaryHash = joaat(newPtfxDictionary.c_str());

                    if (!STREAMING::HAS_NAMED_PTFX_ASSET_LOADED(newPtfxDictionaryHash)) {
                        STREAMING::REQUEST_NAMED_PTFX_ASSET(newPtfxDictionaryHash);
                        while (!STREAMING::HAS_NAMED_PTFX_ASSET_LOADED(newPtfxDictionaryHash)) {
                            WAIT(0);
                        }
                    }

                    // Start particle effects on each exhaust
                    for (const auto& exhaust : vehicleData.exhausts) {
                        Vector3 coords = Vector3(0, -0.2f, 0);
                        Vector3 rotate = Vector3(0, 0.0, 0.0);
                        GRAPHICS::USE_PARTICLE_FX_ASSET(newPtfxDictionary.c_str());
                        int handle = GRAPHICS::START_PARTICLE_FX_LOOPED_ON_ENTITY(newPtfxName.c_str(), exhaust, coords, coords, 0.75f, false, false, false);
                        currentPtfxHandles.push_back(handle);
                    }
                }

                if (vehicleData.vehicleType == "bike" || vehicleData.vehicleType == "car") {
                    for (const auto& headlight : vehicleData.headlights) {
                        ENTITY::SET_ENTITY_VISIBLE(headlight, true);
                    }
                }

                firstEntry = false;
            } else {
                // Turn off the engine
                VEHICLE::SET_VEHICLE_ENGINE_ON(vehicleEntity, false, false);
                isParticleEffectActive = false;

                // Stop particle effects
                for (const auto& handle : currentPtfxHandles) {
                    GRAPHICS::STOP_PARTICLE_FX_LOOPED(handle, false);
                }
                currentPtfxHandles.clear();

                if (vehicleData.vehicleType == "bike" || vehicleData.vehicleType == "car") {
                    for (const auto& headlight : vehicleData.headlights) {
                        ENTITY::SET_ENTITY_VISIBLE(headlight, false);
                    }
                }
            }

            vehicleData.engine = !vehicleData.engine;
        }
    }
}

void CSpawnSubmenu::DisplayVehicleStatus() {	
    if (uiToggle) {
    if (vehicleData.playerInVehicle) {
        int color2[4];
        if (vehicleData.underVehicle && vehicleData.engine) {
            color2[0] = 0;
            color2[1] = 126;
            color2[2] = 0;
            color2[3] = 200;
        } else {
            color2[0] = 126;
            color2[1] = 0;
            color2[2] = 0;
            color2[3] = 200;
        }

        // Get the speed of the vehicle in meters per second
        float speedMPS = ENTITY::GET_ENTITY_SPEED(vehicleData.underVehicle);
        int speedMPH = static_cast<int>(std::round(speedMPS * 2.23694f));

        // Create a formatted string with the speed
        char speedText[32];
        snprintf(speedText, sizeof(speedText), "%d mph", speedMPH);

        // Draw the formatted text with the speed
        Drawing::DrawFormattedText(speedText, Font::Title, 126, 0, 0, 215, Alignment::Center, 42, 0.46f * SCREEN_WIDTH, 0.9f * SCREEN_HEIGHT, 0, 0);

        GRAPHICS::DRAW_SPRITE("hud_textures", "gang_savings", 0.53f, 0.91f, 0.04f, 0.04f, 0.1f, color2[0], color2[1], color2[2], color2[3], 0);
    }
  }
}



// Adjust the pitch of the aircraft
void CSpawnSubmenu::AdjustAircraftPitch(float pitchDirection) {
    Vector3 rotation = ENTITY::GET_ENTITY_ROTATION(vehicleData.underVehicle, 2);
    float pitch = rotation.x;
    float roll = rotation.y;
    float yaw = rotation.z;

    // Adjust pitch based on direction; use a smaller increment for smoother rotation
    float pitchSpeed = 0.4f;  // Adjust this value for smoother or faster rotation
    pitch += pitchDirection * pitchSpeed;

    // Apply the new rotation
    ENTITY::SET_ENTITY_ROTATION(vehicleData.underVehicle, pitch, roll, yaw, 2, true);
}

// Adjust the orientation of the aircraft
void CSpawnSubmenu::AdjustAircraftOrientation(float rollDirection, float yawDirection) {
    Vector3 rotation = ENTITY::GET_ENTITY_ROTATION(vehicleData.underVehicle, 2);
    float pitch = rotation.x;
    float roll = rotation.y;
    float yaw = rotation.z;

    float rotationSpeed = 0.4f;  // Adjust this value for smoother or faster rotation
    roll += rollDirection * rotationSpeed;
    yaw += yawDirection * rotationSpeed;

    ENTITY::SET_ENTITY_ROTATION(vehicleData.underVehicle, pitch, roll, yaw, 2, true);
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
Vector3 RotationToDirection(const Vector3& rotation) {
    Vector3 adjustedRotation = {
        static_cast<float>(M_PI / 180.0) * rotation.x,
        static_cast<float>(M_PI / 180.0) * rotation.y,
        static_cast<float>(M_PI / 180.0) * rotation.z
    };
    Vector3 direction = {
        -std::sin(adjustedRotation.z) * std::abs(std::cos(adjustedRotation.x)),
        std::cos(adjustedRotation.z) * std::abs(std::cos(adjustedRotation.x)),
        std::sin(adjustedRotation.x)
    };
    return direction;
}



void CSpawnSubmenu::TankUpdate() {
    if (vehicleData.playerInVehicle) {
        std::string vehicleType = g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["type"];
        if (vehicleType == "tank") {
    static float lastRotY = 0.0f;
    static float lastRotZ = 0.0f;
    const float smoothFactor = 0.088f;

  
        Vector3 camRot = CAM::GET_GAMEPLAY_CAM_ROT(2);
        Vector3 vehRot = ENTITY::GET_ENTITY_ROTATION(vehicleData.underVehicle, 2);

        float deltaRotY = camRot.y - vehRot.y;
        float deltaRotZ = camRot.z - vehRot.z;

        // Target rotations based on camera's relative rotation
        float targetRotY = deltaRotY;
        float targetRotZ = deltaRotZ;

        // Apply smoothing to the rotations
        float smoothedRotY = lerp(lastRotY, targetRotY, smoothFactor);
        float smoothedRotZ = lerp(lastRotZ, targetRotZ, smoothFactor);

        // Update the last rotations to be used in the next frame
        lastRotY = smoothedRotY;
        lastRotZ = smoothedRotZ;

Vector3 newRotation(0.0f, 0.0f, smoothedRotZ);        // Attach tank top to the vehicle with the smoothed rotations
                    ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.tankTop, vehicleData.underVehicle, 0, vehicleData.tankTopOffsets, newRotation, NULL, true, true, false, 0, true, NULL, NULL);

 static bool pressed = false;  // Track whether the fire button has been pressed to prevent repeated firing
    const int explosionTag_id = 27;  // Define the explosion tag here
    const float damageScale = 1.0f;  // Define the scale of the explosion damage here
    if (PAD::IS_CONTROL_JUST_PRESSED(0, joaat("INPUT_JUMP"))) {
         if (!pressed) {
                    pressed = true;

                    // Perform raycasting with offset to avoid self-damage
                    Vector3 cameraRotation = CAM::GET_GAMEPLAY_CAM_ROT(2);
                    Vector3 cameraCoord = CAM::GET_GAMEPLAY_CAM_COORD();
                    Vector3 direction = RotationToDirection(cameraRotation);
                    float forwardOffset = 20.0f;  // Move start point 20 units forward
                    Vector3 startCoord = {
                        cameraCoord.x + direction.x * forwardOffset,
                        cameraCoord.y + direction.y * forwardOffset,
                        cameraCoord.z + direction.z * forwardOffset
                    };
                    float distance = 1000.0f;
                    Vector3 destination = {
                        startCoord.x + direction.x * distance,
                        startCoord.y + direction.y * distance,
                        startCoord.z + direction.z * distance
                    };
                    int rayHandle = SHAPETEST::START_EXPENSIVE_SYNCHRONOUS_SHAPE_TEST_LOS_PROBE(
                        startCoord, 
                        destination, 
                        -1, 
                        0, 
                        1
                    );

                    BOOL hit;
                    Vector3 hitCoords;
                    Vector3 surfaceNormal;
                    Entity hitEntity;
                    int result = SHAPETEST::GET_SHAPE_TEST_RESULT(rayHandle, &hit, &hitCoords, &surfaceNormal, &hitEntity);

                    if (hit) {
                        FIRE::ADD_EXPLOSION(hitCoords, explosionTag_id, damageScale, true, false, 1.0f);
                    } else {

                    }
                }
            } else {
                pressed = false;
            
    }

    }
	}
}

void CSpawnSubmenu::VehWeaponsUpdate() {
    if (vehicleData.playerInVehicle) {
        if (vehicleData.weaponType == "a10" || vehicleData.weaponType == "xwing" || vehicleData.weaponType == "heli") {
            static bool pressed = false;  // Track whether the fire button has been pressed to prevent repeated firing
            const int a10ExplosionTag_id = 27;  // Define the explosion tag for a10 here
            const int xwingExplosionTag_id = 26;  // Define the explosion tag for xwing here
            const int heliExplosionTag_id = 12;  // Define the explosion tag for heli here
            const float damageScale = 1.0f;  // Define the scale of the explosion damage here

            if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_ATTACK"))) {
                if (vehicleData.weaponType == "a10" || vehicleData.weaponType == "heli" || !pressed) {
                    pressed = true;
                    

                    // Get vehicle coordinates and forward vector
                    Vector3 vehicleCoords = ENTITY::GET_ENTITY_COORDS(vehicleData.underVehicle, true, true);
                    Vector3 forwardVector = ENTITY::GET_ENTITY_FORWARD_VECTOR(vehicleData.underVehicle);

                    // Adjust the forward offset as needed
                    float forwardOffset = 10.0f;
                    Vector3 startCoord = {
                        vehicleCoords.x + forwardVector.x * forwardOffset,
                        vehicleCoords.y + forwardVector.y * forwardOffset,
                        vehicleCoords.z + forwardVector.z * forwardOffset
                    };

                    // Shooting distance
                    float distance = 1000.0f;
                    Vector3 destination = {
                        startCoord.x + forwardVector.x * distance,
                        startCoord.y + forwardVector.y * distance,
                        startCoord.z + forwardVector.z * distance
                    };

                    // Perform raycasting
                    int rayHandle = SHAPETEST::START_EXPENSIVE_SYNCHRONOUS_SHAPE_TEST_LOS_PROBE(
                        startCoord,
                        destination,
                        -1,
                        vehicleData.underVehicle,
                        1
                    );

                    BOOL hit;
                    Vector3 hitCoords;
                    Vector3 surfaceNormal;
                    Entity hitEntity;
                    int result = SHAPETEST::GET_SHAPE_TEST_RESULT(rayHandle, &hit, &hitCoords, &surfaceNormal, &hitEntity);

                    if (hit) {
                        if (vehicleData.weaponType == "xwing" || vehicleData.weaponType == "heli") {
                            // Create two explosions for xwing and heli
                            int explosionTag_id = (vehicleData.weaponType == "xwing") ? xwingExplosionTag_id : heliExplosionTag_id;
                            Vector3 explosionCoord1 = hitCoords;
                            explosionCoord1.x += 4.0f;
                            Vector3 explosionCoord2 = hitCoords;
                            explosionCoord2.x -= 4.0f;
                            FIRE::ADD_EXPLOSION(explosionCoord1, explosionTag_id, damageScale, true, false, 1.0f);
                            FIRE::ADD_EXPLOSION(explosionCoord2, explosionTag_id, damageScale, true, false, 1.0f);
                        } else {
                            // Create a single explosion for a10
                            FIRE::ADD_EXPLOSION(hitCoords, a10ExplosionTag_id, damageScale, true, false, 1.0f);
                        }
                    } else {
                    }
                }
            } else {
                pressed = false;
            }
        }
    }
}




void CSpawnSubmenu::OspreyUpdate() {
    if (vehicleData.playerInVehicle) {
        std::string vehicleType = g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["type"];
        if (vehicleType == "osprey") {

            // Calculate speed and rotation
            float vehicleSpeed = ENTITY::GET_ENTITY_SPEED(vehicleData.underVehicle);
            currentBaseSpeed += speedIncrement;
            if (currentBaseSpeed > maxBaseSpeed) {
                currentBaseSpeed = maxBaseSpeed;
            }
            if (vehicleSpeed > maxSpeedMetersPerSecond) {
                vehicleSpeed = maxSpeedMetersPerSecond;
            }
            float rotSpeedIncrement = 3.0f * (currentBaseSpeed + vehicleSpeed);
            currentBladeRotationAngle = fmod(currentBladeRotationAngle + rotSpeedIncrement, 360.0f);

            Vector3 propellerRotation(0.0f, 0.0f, currentBladeRotationAngle);

            // Attach left propeller with updated rotation and offset
            Vector3 leftPropellerOffset = vehicleData.leftbladeOffsets;
            ENTITY::DETACH_ENTITY(vehicleData.leftPropeller, true, true);
            ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.leftPropeller, vehicleData.leftThruster, 0, leftPropellerOffset, propellerRotation, NULL, true, true, false, 0, true, NULL, NULL);

            // Attach right propeller with updated rotation and offset
            Vector3 rightPropellerOffset = vehicleData.rightbladeOffsets;
            ENTITY::DETACH_ENTITY(vehicleData.rightPropeller, true, true);
            ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.rightPropeller, vehicleData.rightThruster, 0, rightPropellerOffset, propellerRotation, NULL, true, true, false, 0, true, NULL, NULL);

            float speedMetersPerSecond = ENTITY::GET_ENTITY_SPEED(vehicleData.underVehicle);
            float speedMPH = speedMetersPerSecond * 2.23694f;  // Convert m/s to mph

if (PAD::IS_CONTROL_JUST_PRESSED(0, joaat("INPUT_ATTACK"))) {
    flightMode = !flightMode;  // Toggle flight mode

        thrusterRotated = !thrusterRotated;
    thrusterTargetRotationAngle = thrusterRotated ? 90.0f : 0.0f; // 90 for flight mode, 0 for heli mode
}
Vector3 thrusterRotation(-thrusterCurrentRotationAngle, 0.0f, 0.0f);  // Rotating around the X-axis only
Vector3 propRotation(0.0f, 0.0f, currentBladeRotationAngle);  // Rotating around the X-axis only

        ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.leftThruster, vehicleData.underVehicle, 0, vehicleData.leftThrusterOffsets, thrusterRotation, NULL, true, true, false, 0, true, NULL, NULL);
ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.rightThruster, vehicleData.underVehicle, 0, vehicleData.rightThrusterOffsets, thrusterRotation, NULL, true, true, false, 0, true, NULL, NULL);

ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.leftPropeller, vehicleData.leftThruster, 0, vehicleData.leftbladeOffsets, propRotation, NULL, true, true, false, 0, true, NULL, NULL);
ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.rightPropeller, vehicleData.rightThruster, 0, vehicleData.rightbladeOffsets, propRotation, NULL, true, true, false, 0, true, NULL, NULL);

// Rotate thrusters smoothly when flight mode is toggled
if (thrusterCurrentRotationAngle != thrusterTargetRotationAngle) {
    if (thrusterCurrentRotationAngle < thrusterTargetRotationAngle) {
        thrusterCurrentRotationAngle += thrusterRotationSpeed;
        if (thrusterCurrentRotationAngle > thrusterTargetRotationAngle) {
            thrusterCurrentRotationAngle = thrusterTargetRotationAngle;
        }
    } else {
        thrusterCurrentRotationAngle -= thrusterRotationSpeed;
        if (thrusterCurrentRotationAngle < thrusterTargetRotationAngle) {
            thrusterCurrentRotationAngle = thrusterTargetRotationAngle;
        }
    }
}
if (flightMode) {
    // Plane mode
    if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_SPRINT"))) {
        if (speedMPH < 5 && ENTITY::GET_ENTITY_HEIGHT_ABOVE_GROUND(vehicleData.underVehicle) < 7.0f) {
            currentForce = 0.888f;  // Set desired lift speed
            currentForce -= decayRate;  // Decrease force by decay rate
            if (currentForce < 0.0f) {
                currentForce = 0.0f;  // Prevent currentForce from going negative
            }
            ENTITY::APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(vehicleData.underVehicle, 1, 0.0f, 0.0f, currentForce, true, true, true, true);
        } else if (speedMPH >= 5 || ENTITY::GET_ENTITY_HEIGHT_ABOVE_GROUND(vehicleData.underVehicle) >= 17.0f) {
            VEHICLE::SET_VEHICLE_FORWARD_SPEED(vehicleData.underVehicle, speedMetersPerSecond + 0.13f);
        }
                
    }
    // Adjust aircraft roll on A or D press
    if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_LEFT_ONLY")) && !PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_JUMP"))) {
        AdjustAircraftOrientation(0, 1);  // Roll Left
    } else if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_RIGHT_ONLY")) && !PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_JUMP"))) {
        AdjustAircraftOrientation(0, -1);  // Roll Right
    }
    // Adjust aircraft yaw on A or D press with Alt
    if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_JUMP")) && PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_LEFT_ONLY"))) {
        AdjustAircraftOrientation(-1, 0);  // Yaw Left
    } else if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_JUMP")) && PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_RIGHT_ONLY"))) {
        AdjustAircraftOrientation(1, 0);  // Yaw Right
    }
} else {
    // Heli mode
    if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_SPRINT"))) {
        float accelMultiplier = g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["accelMultiplier"].get<float>();
        currentForce = accelMultiplier;
    } else {
        currentForce -= decayRate;  // Decrease force by decay rate
        if (currentForce < 0.0f) {
            currentForce = 0.0f;  // Prevent currentForce from going negative
        }
    }
    ENTITY::APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(vehicleData.underVehicle, 1, 0.0f, 0.0f, currentForce, true, true, true, true);
     // Adjust yaw directly on A or D press (no roll)
    if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_LEFT_ONLY"))) {
        AdjustAircraftOrientation(-1, 0);  // Yaw Left
    } else if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_RIGHT_ONLY"))) {
        AdjustAircraftOrientation(1, 0);  // Yaw Right
    }

    // Adjust roll on A or D press with Alt (simulate heli banking)
    if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_JUMP")) && PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_LEFT_ONLY"))) {
        AdjustAircraftOrientation(0, 1);  // Bank Left (simulate roll)
    } else if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_JUMP")) && PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_RIGHT_ONLY"))) {
        AdjustAircraftOrientation(0, -1);  // Bank Right (simulate roll)
    }
}

// Adjust pitch on W or S press if speed is less than 20 MPH
if (speedMPH < 20) {
    if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_UP_ONLY"))) {
        AdjustAircraftPitch(-1);  // Pitch Down
    } else if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_DOWN_ONLY"))) {
        AdjustAircraftPitch(1);   // Pitch Up
    }
}
if (PAD::IS_CONTROL_JUST_PRESSED(0, joaat("INPUT_AIM"))) { 
        // Toggle the door state and set target rotation angle
        doorOpen = !doorOpen;
    }

    // Set the target angle based on the door's open or closed state
    float targetAngle = doorOpen ? maxOspreyDoorRotationAngle : 0.0f;
    
    // Smoothly increment or decrement towards the target angle
    if (currentDoorRotationAngle < targetAngle) {
        currentDoorRotationAngle += doorRotationSpeed;
        if (currentDoorRotationAngle > targetAngle) currentDoorRotationAngle = targetAngle;
    } else if (currentDoorRotationAngle > targetAngle) {
        currentDoorRotationAngle -= doorRotationSpeed;
        if (currentDoorRotationAngle < targetAngle) currentDoorRotationAngle = targetAngle;
    }

    // Attach the door with the updated rotation
    ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.ospreyDoor, vehicleData.underVehicle, 0, vehicleData.ospreyDoorOffsets, Vector3(currentDoorRotationAngle + vehicleData.ospreyDoorRotation.x, vehicleData.ospreyDoorRotation.y, vehicleData.ospreyDoorRotation.z), NULL, true, true, false, 0, true, NULL, NULL);
        }
    }
}


void CSpawnSubmenu::HeliUpdate() {
    if (vehicleData.engine) {
        if (vehicleData.vehicleType == "heli" || vehicleData.vehicleType == "cargobob") {

            // Calculate speed and rotation
            float vehicleSpeed = ENTITY::GET_ENTITY_SPEED(vehicleData.underVehicle);
            currentBaseSpeed += speedIncrement;
            if (currentBaseSpeed > maxBaseSpeed) {
                currentBaseSpeed = maxBaseSpeed;
            }
            if (vehicleSpeed > maxSpeedMetersPerSecond) {
                vehicleSpeed = maxSpeedMetersPerSecond;
            }
            float rotSpeedIncrement = 3.0f * (currentBaseSpeed + vehicleSpeed);
            currentBladeRotationAngle = fmod(currentBladeRotationAngle + rotSpeedIncrement, 360.0f);

            // Prepare rotation for all propellers
            Vector3 topRotation(0.0f, 0.0f, currentBladeRotationAngle);
            Vector3 rearRotation(0.0f, 0.0f, currentBladeRotationAngle);

            if (vehicleData.vehicleType == "heli") {
                // Attach each top blade with updated rotation and offset
                for (size_t i = 0; i < vehicleData.topblades.size(); ++i) {
                    auto& topBlade = vehicleData.topblades[i];
                    Vector3 topBladeOffset = vehicleData.topbladeOffsets[i];
                    Vector3 rotation = Vector3(topRotation.x, topRotation.y, currentBladeRotationAngle);

                    ENTITY::DETACH_ENTITY(topBlade, true, true);
                    ENTITY::ATTACH_ENTITY_TO_ENTITY(topBlade, vehicleData.underVehicle, 0, topBladeOffset, rotation, NULL, true, true, false, 0, true, NULL, NULL);
                }

                // Attach each rear blade with updated rotation and offset
                for (size_t i = 0; i < vehicleData.rearblades.size(); ++i) {
                    auto& rearBlade = vehicleData.rearblades[i];
                    Vector3 rearBladeOffset = vehicleData.rearbladeOffsets[i];
                    Vector3 rotation = Vector3(currentBladeRotationAngle, 0, 0);

                    ENTITY::DETACH_ENTITY(rearBlade, true, true);
                    ENTITY::ATTACH_ENTITY_TO_ENTITY(rearBlade, vehicleData.underVehicle, 0, rearBladeOffset, rotation, NULL, true, true, false, 0, true, NULL, NULL);
                }
            } else if (vehicleData.vehicleType == "cargobob") {
                // Attach front blade with updated rotation and offset
                Vector3 frontBladeOffset = vehicleData.cargobobfrontbladeOffsets;
                ENTITY::DETACH_ENTITY(vehicleData.cargobobfrontBlade, true, true);
                ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.cargobobfrontBlade, vehicleData.underVehicle, 0, vehicleData.cargobobfrontbladeOffsets, topRotation, NULL, true, true, false, 0, true, NULL, NULL);

                // Attach rear blade with updated rotation and offset
                Vector3 rearBladeOffset = vehicleData.cargobobrearbladeOffsets;
                ENTITY::DETACH_ENTITY(vehicleData.cargobobrearBlade, true, true);
                ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.cargobobrearBlade, vehicleData.underVehicle, 0, vehicleData.cargobobrearbladeOffsets, rearRotation, NULL, true, true, false, 0, true, NULL, NULL);
            }

            // Additional logic for helicopter control
            float speedMetersPerSecond = ENTITY::GET_ENTITY_SPEED(vehicleData.underVehicle);
            float speedMPH = speedMetersPerSecond * 2.23694f;  // Convert m/s to mph
            if (vehicleData.playerInVehicle) {
    // Adjust vertical velocity on Shift press
    if (PAD::IS_CONTROL_PRESSED(0, 0x8FFC75D6)) {
        float accelMultiplier = g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["accelMultiplier"].get<float>();
        currentForce = accelMultiplier;  // Set the desired lift speed using accelMultiplier from config

            } else {
    // Gradually decrease the force when Shift is not pressed
    currentForce -= decayRate;
    if (currentForce < 0.0f) {
        currentForce = 0.0f;
    }
}

            // Apply the current force to the vehicle
            ENTITY::APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(vehicleData.underVehicle, 1, 0.0f, 0.0f, currentForce, true, true, true, true);

            // Adjust aircraft orientation on A or D press
            if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_LEFT_ONLY")) && !PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_JUMP"))) {
                AdjustAircraftOrientation(-1, 0);  // Roll Left
            } else if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_RIGHT_ONLY")) && !PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_JUMP"))) {
                AdjustAircraftOrientation(1, 0);  // Roll Right
            }

            // Adjust yaw on A or D press with Alt
            if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_JUMP")) && PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_LEFT_ONLY"))) {
                AdjustAircraftOrientation(0, 1);  // Yaw Left
            } else if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_JUMP")) && PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_RIGHT_ONLY"))) {
                AdjustAircraftOrientation(0, -1);   // Yaw Right
            }

            // Adjust pitch on W or S press if speed is less than 20 MPH
            if (speedMPH < 20) {
                if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_UP_ONLY"))) {
                    AdjustAircraftPitch(-1);  // Pitch Down
                } else if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_DOWN_ONLY"))) {
                    AdjustAircraftPitch(1);   // Pitch Up
                }
            }
            if (vehicleData.vehicleType == "cargobob") {
    if (PAD::IS_CONTROL_JUST_PRESSED(0, joaat("INPUT_ATTACK"))) { 
        // Toggle the door state and set target rotation angle
        doorOpen = !doorOpen;
    }

    // Set the target angle based on the door's open or closed state
    float targetAngle = doorOpen ? maxCargobobDoorRotationAngle : 0.0f;
    
    // Smoothly increment or decrement towards the target angle
    if (currentDoorRotationAngle < targetAngle) {
        currentDoorRotationAngle += doorRotationSpeed;
        if (currentDoorRotationAngle > targetAngle) currentDoorRotationAngle = targetAngle;
    } else if (currentDoorRotationAngle > targetAngle) {
        currentDoorRotationAngle -= doorRotationSpeed;
        if (currentDoorRotationAngle < targetAngle) currentDoorRotationAngle = targetAngle;
    }

    // Attach the door with the updated rotation
    ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.cargobobDoor, vehicleData.underVehicle, 0, vehicleData.cargobobdoorOffsets, Vector3(currentDoorRotationAngle + vehicleData.cargobobdoorRotation.x, vehicleData.cargobobdoorRotation.y, vehicleData.cargobobdoorRotation.z), NULL, true, true, false, 0, true, NULL, NULL);
}
            }
        }
    }
}


void CSpawnSubmenu::PlaneUpdate() {
    if (vehicleData.engine) {
        if (vehicleData.vehicleType == "plane" || vehicleData.vehicleType == "jet") {
            Vehicle vehicle = vehicleData.underVehicle;
            if (ENTITY::DOES_ENTITY_EXIST(vehicle)) {
                float speed = ENTITY::GET_ENTITY_SPEED(vehicle);
                float speedMph = speed * 2.23694f;  // Convert m/s to mph

                // Get the vehicle's height above the ground
                float heightAboveGround = ENTITY::GET_ENTITY_HEIGHT_ABOVE_GROUND(vehicle);

                // Propeller logic
                if (vehicleData.vehicleType == "plane") {
                    static float CurrentPropellerRotationAngle = 0.0f;
                    for (size_t i = 0; i < vehicleData.propellers.size(); ++i) {
                        float baseSpeed = 15.0f;
                        float rotationIncrement = baseSpeed + speed;  // Ensure base rotation even when speed is zero
                        CurrentPropellerRotationAngle += rotationIncrement;

                        // Normalize the rotation angle to stay within 0 to 360 degrees
                        if (CurrentPropellerRotationAngle >= 360.0f) {
                            CurrentPropellerRotationAngle = 0;
                        }

                        Vector3 newRotation = vehicleData.propellerRotations[i];
                        newRotation.y = CurrentPropellerRotationAngle;

                        ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.propellers[i], vehicle, 0, vehicleData.propellerOffsets[i], newRotation, NULL, true, true, false, 0, true, NULL, NULL);
                    }
                }

                // Control logic
                if (vehicleData.playerInVehicle) {
                    if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_LEFT_ONLY")) && !PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_JUMP"))) {
                        AdjustAircraftOrientation(0, 1);  // Yaw Left
                    } else if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_RIGHT_ONLY")) && !PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_JUMP"))) {
                        AdjustAircraftOrientation(0, -1);  // Yaw Right
                    }

                    if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_JUMP")) && PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_RIGHT_ONLY"))) {
                        AdjustAircraftOrientation(1, 0);  // Roll Left
                    } else if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_JUMP")) && PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_LEFT_ONLY"))) {
                        AdjustAircraftOrientation(-1, 0);  // Roll Right
                    }

                    // Check if the Shift key is pressed
                    if (PAD::IS_CONTROL_PRESSED(0, 0x8FFC75D6)) {
                        if (speed < 5.0f && heightAboveGround < 1.75f) {
                            // Apply force to center of mass
                            ENTITY::APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(vehicle, 1, 0.0f, 0.0f, 0.9f, true, true, true, true);
                        } else if (speed >= 5.0f || heightAboveGround >= 1.5f) {
                            // Increase speed
                                    float accelMultiplier = g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["accelMultiplier"].get<float>();

                            VEHICLE::SET_VEHICLE_FORWARD_SPEED(vehicle, speed + accelMultiplier);
                        }
                    }
                }
            }
        }
    }
}



void CSpawnSubmenu::IncreaseVehicleSpeed() {
    Vehicle vehicle = vehicleData.underVehicle;

    if (ENTITY::DOES_ENTITY_EXIST(vehicle)) {
        vehicleData.currspeed = ENTITY::GET_ENTITY_SPEED(vehicle);

        if (vehicleData.underVehicle) {
            if (vehicleData.vehicleType == "car" || vehicleData.vehicleType == "bike" || vehicleData.vehicleType == "tank") {
                if (vehicleData.playerInVehicle && vehicleData.engine) {
                    float speedMph = vehicleData.currspeed * 2.23694f;  // Convert m/s to mph

                    // Get the vehicle's height above the ground
                    float vehicleHeightAboveGround = ENTITY::GET_ENTITY_HEIGHT_ABOVE_GROUND(vehicle);

                    // Check AltHandling configuration
                    if (g_Config["Config"]["AltHandling"].get<bool>()) {
                        // Only apply speed boost if height is less than or equal to 0.67
                        if (vehicleHeightAboveGround <= 0.67f) {
                            if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_UP_ONLY")) || PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_SPRINT"))) {
                                if (speedMph < g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["topSpeed"].get<float>()) {
                                    float accelMultiplier = g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["accelMultiplier"].get<float>();
                                    accelMultiplier /= 2.0f; 
                                    float newSpeed = vehicleData.currspeed + accelMultiplier;
                                    VEHICLE::SET_VEHICLE_FORWARD_SPEED(vehicle, newSpeed);
                                }
                            }
                        }
                    } else {
                        // Only apply speed boost if height is less than or equal to 1.25
                        if (vehicleHeightAboveGround <= 1.25f) {
                            if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_UP_ONLY")) || PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_SPRINT"))) {
                                if (speedMph < g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["topSpeed"].get<float>()) {
                                    float accelMultiplier = g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["accelMultiplier"].get<float>();
                                    accelMultiplier /= 3.0f; // Cut AltAccelMultiplier in half
                                    Vector3 currentVelocity = ENTITY::GET_ENTITY_VELOCITY(vehicle, 0);
                                    Vector3 forwardVector = ENTITY::GET_ENTITY_FORWARD_VECTOR(vehicle);
                                    Vector3 newVelocity = currentVelocity + forwardVector * accelMultiplier;
                                    ENTITY::SET_ENTITY_VELOCITY(vehicle, newVelocity.x, newVelocity.y, newVelocity.z);
                                }
                            }
                            // Check for reverse input
                                                       // Check for reverse input
                            if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_DOWN_ONLY"))) {                               
                            float reverseMultiplier;
                            if (vehicleData.currspeed < 6.0f) {
                             reverseMultiplier = g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["accelMultiplier"].get<float>();
                         } else {
                            reverseMultiplier = g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["accelMultiplier"].get<float>() / 3.5f;
                          }
                            Vector3 currentVelocity = ENTITY::GET_ENTITY_VELOCITY(vehicle, 0);
                            Vector3 backwardVector = ENTITY::GET_ENTITY_FORWARD_VECTOR(vehicle) * -1.0f;
                            Vector3 newVelocity = currentVelocity + backwardVector * reverseMultiplier;
                             ENTITY::SET_ENTITY_VELOCITY(vehicle, newVelocity.x, newVelocity.y, newVelocity.z);
}

                    // Set brake lights visibility based on handbrake or down only input
                    if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_DOWN_ONLY")) || PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_VEH_HANDBRAKE"))) {
                     for (const auto& brakeLight : vehicleData.brakelights) {
                     ENTITY::SET_ENTITY_VISIBLE(brakeLight, true);
                        }
                        } else {
                        for (const auto& brakeLight : vehicleData.brakelights) {
                          ENTITY::SET_ENTITY_VISIBLE(brakeLight, false);
                            }
                        }





                        }
                    }
                }
            } else if (vehicleData.vehicleType == "boat") {
                if (vehicleData.playerInVehicle) {
                float speedMph = vehicleData.currspeed * 2.23694f;  // Convert m/s to mph

                if (PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_UP_ONLY"))) {
                 // Fetch the top speed and acceleration multiplier from the configuration
                 float topSpeed = g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["topSpeed"];
                 float accelMultiplier = g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["accelMultiplier"];

                if (speedMph < topSpeed && !PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_DOWN_ONLY"))) {
                  float newSpeed = vehicleData.currspeed + accelMultiplier;
                 VEHICLE::SET_VEHICLE_FORWARD_SPEED(vehicle, newSpeed);
                    }
                }
                }
            }
        }
    }
}





void CSpawnSubmenu::TrackVehicleEntryExit() {
    if (vehicleData.underVehicle) {
    static bool trackingStarted = false; // Track if the tracking should start

    Vehicle playerVehicle = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), false);

    if (!trackingStarted && playerVehicle != 0) {
        trackingStarted = true;
    }

    if (trackingStarted) {
        if (playerVehicle == vehicleData.underVehicle && !vehicleData.playerInVehicle) {
            vehicleData.playerInVehicle = true;


            // Spawn the clone ped
            ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), false);
            vehicleData.clonePed = PED::CLONE_PED(PLAYER::PLAYER_PED_ID(), ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()), true, true);
            
            STREAMING::REQUEST_ANIM_DICT("script_re@check_point@small_cart");
            while (!STREAMING::HAS_ANIM_DICT_LOADED("script_re@check_point@small_cart")) {
                WAIT(0);
            }

	TASK::TASK_PLAY_ANIM(vehicleData.clonePed, "script_re@check_point@small_cart", "int_loop_driver", 8.0f, 1.0f, -1, AF_LOOPING | AF_FORCE_START, 0.1f, NULL, NULL, NULL, NULL, NULL);
            EVENT::SET_DECISION_MAKER(vehicleData.clonePed, joaat("EMPTY"));
                        ENTITY::ATTACH_ENTITY_TO_ENTITY(vehicleData.clonePed, vehicleData.objectModel, 0, vehicleData.cloneOffsets, vehicleData.cloneRotation, NULL, true, true, false, 0, true, NULL, NULL);
                if (seatbelt) {
        PED::SET_PED_CAN_BE_KNOCKED_OFF_VEHICLE(PLAYER::PLAYER_PED_ID(), 1); // Prevent the clone ped from being knocked off the vehicle
        } else {
         PED::SET_PED_CAN_BE_KNOCKED_OFF_VEHICLE(PLAYER::PLAYER_PED_ID(), 2); // Allow the clone ped to be knocked off the vehicle
        }
            PED::SET_RAGDOLL_BLOCKING_FLAGS(vehicleData.clonePed, RBF_IMPACT_OBJECT | RBF_PED_RAGDOLL_BUMP);
            PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(vehicleData.clonePed, true);
            PED::SET_PED_CAN_BE_KNOCKED_OFF_VEHICLE(vehicleData.clonePed, 1); // Prevent the clone ped from being knocked off the vehicle
            PED::SET_PED_CAN_RAGDOLL(vehicleData.clonePed, false); // Prevent the clone ped from ragdolling
            ENTITY::SET_ENTITY_COLLISION(vehicleData.clonePed, false, false); // Disable collision for the clone ped
            ENTITY::SET_ENTITY_INVINCIBLE(vehicleData.clonePed, true); // Make the clone ped invincible
            *getGlobalPtr(1415412) = 2; // Disable vehicle selling at the fence.
        } else if (playerVehicle != vehicleData.underVehicle && vehicleData.playerInVehicle) {
            vehicleData.playerInVehicle = false;
            

            // Delete the clone ped
            ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), true);
            PED::DELETE_PED(&vehicleData.clonePed);
        }
    }
    // Check if the "E" key is pressed
    if (PAD::IS_CONTROL_JUST_PRESSED(0, 0xCEFD9220)) { // 51 is the control ID for the "E" key
        // Get the player's current position
        Vector3 playerPos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true, true);

        // Get the coordinates of the underVehicle
        Vector3 underVehiclePos = ENTITY::GET_ENTITY_COORDS(vehicleData.underVehicle, true, true);

        // Calculate the distance between the player and the underVehicle
        float distance = (playerPos - underVehiclePos).Length();

        // Check if the player is within 7 units of the underVehicle
        if ((vehicleData.vehicleType == "car" || vehicleData.vehicleType == "bike") && attachHorse) {
    if (distance <= 2.0f) {
        // Warp the player ped into the vehicle
        TASK::CLEAR_PED_TASKS(PLAYER::PLAYER_PED_ID(), true, true);
        ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), false);
        TASK::TASK_WARP_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), vehicleData.underVehicle, -1);
    }
} else if ((vehicleData.vehicleType == "car" || vehicleData.vehicleType == "bike") && !attachHorse) {
    if (distance <= 5.0f) {
        // Warp the player ped into the vehicle
        TASK::CLEAR_PED_TASKS(PLAYER::PLAYER_PED_ID(), true, true);
        ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), false);
        TASK::TASK_WARP_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), vehicleData.underVehicle, -1);
    }
} else {
    if (distance <= 7.0f) {
        // Warp the player ped into the vehicle
        TASK::CLEAR_PED_TASKS(PLAYER::PLAYER_PED_ID(), true, true);
        ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), false);
        TASK::TASK_WARP_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), vehicleData.underVehicle, -1);
    }
}

    }
}
    if (vehicleData.playerInVehicle) { 
            ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), false);

        }
}



   





void CSpawnSubmenu::UpdateWheels() {
    // Check if the player is in the driver's seat
if (vehicleData.vehicleType == "car" || vehicleData.vehicleType == "bike") {
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    

    static float CurrentRotationAngle = 0.0f; 
    static float CurrentTurnAngle = 0.0f; 
    const float TurnSmoothing = 10.0f; // Smoothing factor for turning

    float vehSpeed = ENTITY::GET_ENTITY_SPEED(vehicleData.underVehicle);

    Vector3 vehForwardVector = ENTITY::GET_ENTITY_FORWARD_VECTOR(vehicleData.underVehicle);

    Vector3 vehVelocityVector = ENTITY::GET_ENTITY_VELOCITY(vehicleData.underVehicle, true);

    float VehicleDotProduct = vehForwardVector.Dot(vehVelocityVector);

    float directionMultiplier = (VehicleDotProduct >= 0) ? 1.0f : -1.0f;

    float rotationSpeedIncrement = 3.0f * vehSpeed * directionMultiplier;

    bool IsTurningLeft = PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_LEFT_ONLY"));
    bool IsTurningRight = PAD::IS_CONTROL_PRESSED(0, joaat("INPUT_MOVE_RIGHT_ONLY"));

    //  default turn angle 
    float targetTurnAngle = 0.0f;
    if (IsTurningLeft) {
        targetTurnAngle = -30.0f;
    } else if (IsTurningRight) {
        targetTurnAngle = 30.0f;
    }

    // Smoothly adjust the current turn angle towards the target turn angle
    CurrentTurnAngle += (targetTurnAngle - CurrentTurnAngle) / TurnSmoothing;

    CurrentRotationAngle += rotationSpeedIncrement;

    if (CurrentRotationAngle >= 360.0f) {
        CurrentRotationAngle -= 360.0f;
    } else if (CurrentRotationAngle < 0.0f) {
        CurrentRotationAngle += 360.0f;
    }

    // Suspension
    float originalFrontDistanceToGround = g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["frontDistanceToGrnd"];
    float frontDistanceToGrndMin = g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["frontSuspensionLowerLimit"];
    float frontDistanceToGrndMax = g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["frontSuspensionUpperLimit"];

    
    for (size_t i = 0; i < vehicleData.frontWheels.size(); ++i) {
    auto& wheel = vehicleData.frontWheels[i];
    Vector3& wheelPosition = vehicleData.frontWheelOffsets[i];  // Use a reference to directly modify the offset

    float orientationAngle = (wheelPosition.x > 0) ? 180.0f : 0.0f;  // Flip the right-side wheels
    float rotationAngle = (wheelPosition.x > 0) ? CurrentRotationAngle : -CurrentRotationAngle;  // Reverse rotation for left-side wheels

    float currentDistanceToGround = ENTITY::GET_ENTITY_HEIGHT_ABOVE_GROUND(wheel);

    float distanceDifference = originalFrontDistanceToGround - currentDistanceToGround;


    if (abs(distanceDifference) > 0.01f) {  // Small threshold to prevent unnecessary updates
        wheelPosition.z += distanceDifference; 
        }
    // not sure how this really works but it does
    if (wheelPosition.z < frontDistanceToGrndMin) {
        wheelPosition.z = frontDistanceToGrndMin;
    } else if (wheelPosition.z > frontDistanceToGrndMax) {
        wheelPosition.z = frontDistanceToGrndMax;
    }
    if (VEHICLE::GET_PED_IN_VEHICLE_SEAT(vehicleData.underVehicle, VS_DRIVER) == playerPed) {
            ENTITY::ATTACH_ENTITY_TO_ENTITY(wheel, vehicleData.objectModel, 0, wheelPosition, Vector3(rotationAngle, 0, -CurrentTurnAngle + orientationAngle), NULL, true, true, false, 0, true, NULL, NULL);
        } else {
            ENTITY::ATTACH_ENTITY_TO_ENTITY(wheel, vehicleData.objectModel, 0, wheelPosition, Vector3(rotationAngle, 0, orientationAngle), NULL, true, true, false, 0, true, NULL, NULL);
        }

}

    float originalRearDistanceToGround = g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["rearDistanceToGrnd"];
    float rearDistanceToGrndMin = g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["rearSuspensionLowerLimit"];
    float rearDistanceToGrndMax = g_Config["Config"]["Vehicles"][vehicleData.vehicleKey]["rearSuspensionUpperLimit"];

for (size_t i = 0; i < vehicleData.rearWheels.size(); ++i) {
    auto& wheel = vehicleData.rearWheels[i];
    Vector3& wheelPosition = vehicleData.rearWheelOffsets[i];  

    float orientationAngle = (wheelPosition.x > 0) ? 180.0f : 0.0f;  // Flip the right-side wheels
    float rotationAngle = (wheelPosition.x > 0) ? CurrentRotationAngle : -CurrentRotationAngle;  // Reverse rotation for left-side wheels

    float currentDistanceToGround = ENTITY::GET_ENTITY_HEIGHT_ABOVE_GROUND(wheel);

    float distanceDifference = originalRearDistanceToGround - currentDistanceToGround;


    if (abs(distanceDifference) > 0.01f) {  // Small threshold to prevent unnecessary updates
        wheelPosition.z += distanceDifference;  
    }
    if (wheelPosition.z < rearDistanceToGrndMin) {
        wheelPosition.z = rearDistanceToGrndMin;
    } else if (wheelPosition.z > rearDistanceToGrndMax) {
        wheelPosition.z = rearDistanceToGrndMax;
    }
    ENTITY::ATTACH_ENTITY_TO_ENTITY(wheel, vehicleData.objectModel, 0, wheelPosition,Vector3(rotationAngle, 0, orientationAngle), NULL, true, true, false, 0, true, NULL, NULL
    );
}
}
}


void CSpawnSubmenu::HorseFinder() {
    if (attachHorse) {
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    Ped mount = PED::_GET_LAST_MOUNT(playerPed);

    // Check if the player has a mount
    if (ENTITY::DOES_ENTITY_EXIST(mount)) {
        if (vehicleData.playerInVehicle && !vehicleData.horseAttached) {
            // Set the mount to be invisible
            ENTITY::SET_ENTITY_VISIBLE(mount, false);

            // Attach the mount to vehicleData.underVehicle
            ENTITY::ATTACH_ENTITY_TO_ENTITY(mount, vehicleData.underVehicle, 0, Vector3(0, 0, 0), Vector3(0, 0, 0), NULL, true, true, false, 0, true, NULL, NULL);
            vehicleData.horseAttached = true;
        } else if (!vehicleData.playerInVehicle && vehicleData.horseAttached) {
            // Detach the mount from any entity
            ENTITY::DETACH_ENTITY(mount, true, true);

            // Get the player's current coordinates
            Vector3 playerCoords = ENTITY::GET_ENTITY_COORDS(playerPed, true, true);

            // Set the mount's coordinates to the player's coordinates, but 5 units behind on the Y-axis
            Vector3 mountCoords = playerCoords;
            mountCoords.y -= 5.0f;
            ENTITY::SET_ENTITY_COORDS(mount, mountCoords.x, mountCoords.y, mountCoords.z, false, false, false, true);

            // Set the mount to be visible
            ENTITY::SET_ENTITY_VISIBLE(mount, true);
            vehicleData.horseAttached = false;
        }
    }
}
}




void CSpawnSubmenu::HoverCar() {
    if (hoverCar) {
        if (vehicleData.underVehicle) {
            if (vehicleData.vehicleType == "car" || vehicleData.vehicleType == "bike") {
                for (size_t i = 0; i < vehicleData.frontWheels.size(); ++i) {
                    ENTITY::SET_ENTITY_VISIBLE(vehicleData.frontWheels[i], false);
                }
                for (size_t i = 0; i < vehicleData.rearWheels.size(); ++i) {
                    ENTITY::SET_ENTITY_VISIBLE(vehicleData.rearWheels[i], false);
                }
            }
        }
    } else {
        if (vehicleData.underVehicle) {
            if (vehicleData.vehicleType == "car" || vehicleData.vehicleType == "bike") {
                for (size_t i = 0; i < vehicleData.frontWheels.size(); ++i) {
                    ENTITY::SET_ENTITY_VISIBLE(vehicleData.frontWheels[i], true);
                }
                for (size_t i = 0; i < vehicleData.rearWheels.size(); ++i) {
                    ENTITY::SET_ENTITY_VISIBLE(vehicleData.rearWheels[i], true);
                }
            }
        }
    }
}



void CSpawnSubmenu::DeleteVehicle() {
    vehicleData.engine = false;
                    VEHICLE::SET_VEHICLE_ENGINE_ON(vehicleData.underVehicle, false, true);
					firstEntry = true;
                    	 PED::_WARP_PED_OUT_OF_VEHICLE(PLAYER::PLAYER_PED_ID());
                         WAIT(1);
if (ENTITY::DOES_ENTITY_EXIST(vehicleData.clonePed)) {
    PED::DELETE_PED(&vehicleData.clonePed);
}
    if (ENTITY::DOES_ENTITY_EXIST(vehicleData.underVehicle)) {
        ENTITY::DELETE_ENTITY(&vehicleData.underVehicle);
    }
    if (ENTITY::DOES_ENTITY_EXIST(vehicleData.tankTop)) {
    ENTITY::DELETE_ENTITY(&vehicleData.tankTop);
}
    if (ENTITY::DOES_ENTITY_EXIST(vehicleData.leftPropeller)) {
    ENTITY::DELETE_ENTITY(&vehicleData.leftPropeller);
}
    if (ENTITY::DOES_ENTITY_EXIST(vehicleData.rightPropeller)) {
    ENTITY::DELETE_ENTITY(&vehicleData.rightPropeller);
}
    if (ENTITY::DOES_ENTITY_EXIST(vehicleData.rightThruster)) {
    ENTITY::DELETE_ENTITY(&vehicleData.rightThruster);
}
    if (ENTITY::DOES_ENTITY_EXIST(vehicleData.leftThruster)) {
    ENTITY::DELETE_ENTITY(&vehicleData.leftThruster);
}
    if (ENTITY::DOES_ENTITY_EXIST(vehicleData.cargobobDoor)) {
    ENTITY::DELETE_ENTITY(&vehicleData.cargobobDoor);
}
    if (ENTITY::DOES_ENTITY_EXIST(vehicleData.cargobobrearBlade)) {
    ENTITY::DELETE_ENTITY(&vehicleData.cargobobrearBlade);
}
    if (ENTITY::DOES_ENTITY_EXIST(vehicleData.cargobobfrontBlade)) {
    ENTITY::DELETE_ENTITY(&vehicleData.cargobobfrontBlade);
}
    if (ENTITY::DOES_ENTITY_EXIST(vehicleData.ospreyDoor)) {
    ENTITY::DELETE_ENTITY(&vehicleData.ospreyDoor);
}
vehicleData.playerInVehicle = false;
    if (ENTITY::DOES_ENTITY_EXIST(vehicleData.objectModel)) {
        ENTITY::DELETE_ENTITY(&vehicleData.objectModel);
    }
    for (auto& exhaust : vehicleData.exhausts) {
        if (ENTITY::DOES_ENTITY_EXIST(exhaust)) {
            ENTITY::DELETE_ENTITY(&exhaust);
        }
    }
	vehicleData.exhausts.clear();
	isParticleEffectActive = false;
    for (auto& wheel : vehicleData.frontWheels) {
        if (ENTITY::DOES_ENTITY_EXIST(wheel)) {
            ENTITY::DELETE_ENTITY(&wheel);
        }
    }
    vehicleData.frontWheels.clear();

    for (auto& wheel : vehicleData.rearWheels) {
        if (ENTITY::DOES_ENTITY_EXIST(wheel)) {
            ENTITY::DELETE_ENTITY(&wheel);
        }
    }
    vehicleData.rearWheels.clear();
    vehicleData.frontWheelOffsets.clear();
    vehicleData.rearWheelOffsets.clear();



    for (auto& headlight : vehicleData.headlights) {
        if (ENTITY::DOES_ENTITY_EXIST(headlight)) {
            ENTITY::DELETE_ENTITY(&headlight);
        }
    }
    vehicleData.headlights.clear();

    for (auto& brakelight : vehicleData.brakelights) {
        if (ENTITY::DOES_ENTITY_EXIST(brakelight)) {
            ENTITY::DELETE_ENTITY(&brakelight);
        }
    }
    vehicleData.brakelights.clear();

    for (auto& propeller : vehicleData.propellers) {
        if (ENTITY::DOES_ENTITY_EXIST(propeller)) {
            ENTITY::DELETE_ENTITY(&propeller);
        }
    }
    for (auto& topBlade : vehicleData.topblades) {
        if (ENTITY::DOES_ENTITY_EXIST(topBlade)) {
            ENTITY::DELETE_ENTITY(&topBlade);
        }
    }
    for (auto& rearBlade : vehicleData.rearblades) {
        if (ENTITY::DOES_ENTITY_EXIST(rearBlade)) {
            ENTITY::DELETE_ENTITY(&rearBlade);
        }
    }
    vehicleData.propellers.clear();
    vehicleData.propellerOffsets.clear();
    vehicleData.topblades.clear();
    vehicleData.topbladeOffsets.clear();
    vehicleData.rearblades.clear();
    vehicleData.rearbladeOffsets.clear();
    vehicleData.cloneOffsets = Vector3();
    vehicleData.cloneRotation = Vector3();
    vehicleData.underVehicle = NULL;
    vehicleData.objectModel = NULL;
}

void COptionsSubmenu::Init()
{

}



