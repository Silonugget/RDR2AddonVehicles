#include "script.h"
nlohmann::json g_Config;

nlohmann::json ReadConfigFile(const std::string& filePath) {
    std::ifstream file(filePath);
    nlohmann::json config;
    if (file.is_open()) {
        file >> config;
    } else {
        std::cerr << "Unable to open config file: " << filePath << std::endl;
    }
    return config;
}

// Function to handle vehicle selection
void HandleVehicleSelection(const std::string& vehicleKey) {
    std::cout << "Vehicle selected: " << vehicleKey << std::endl;

    // Call the SpawnVehicle function with the selected vehicle key
    g_SpawnSubmenu->SpawnVehicle(vehicleKey);
}

bool exampleBool = false;
const std::vector<std::string> exampleOptionVector = { "First", "Second", "Third", "Last" };

void InitializeMenu() {
    g_Config = ReadConfigFile("vehicleconfig.json");

    g_Menu->AddSubmenu("ADDON VEHICLES", "Iron Horses", Submenu_EntryMenu, 8, [](Submenu* sub) {
        sub->AddSubmenuOption("Spawn Vehicle", "Choose a vehicle to spawn", Submenu_Spawn);
        sub->AddSubmenuOption("Vehicle Options", "Edit options for vehicles", Submenu_Options);
        sub->AddRegularOption("Delete Vehicle", "Delete ride", [] {
         g_SpawnSubmenu->DeleteVehicle();   });
    });
    g_Menu->AddSubmenu("VEHICLE OPTIONS", "Edit options for vehicles", Submenu_Options, 8, [](Submenu* sub) {
        sub->AddBoolOption("Spawn In Vehicle", "Enable or disable auto warp into vehicle", &autoWarp, [] {
        });
        sub->AddBoolOption("Seatbelt", "Enable or disable seatbelt", &seatbelt, [] {
        });
        sub->AddBoolOption("Attach Horse", "Attaches LAST mount to vehicle", &attachHorse, [] {
        });
        sub->AddBoolOption("Play Sounds", "Turn vehicle sounds on or off", &playSounds, [] {
        });
        sub->AddBoolOption("Display UI", "Turn vehicle UI on or off", &uiToggle, [] {
        });
        sub->AddBoolOption("Hover Car", "Turn vehicle wheels on or off", &hoverCar, [] {
        });
    });

    g_Menu->AddSubmenu("SPAWN VEHICLE", "Choose a vehicle type", Submenu_Spawn, 8, [](Submenu* sub) {
        sub->AddSubmenuOption("Car", "Spawn a car", Submenu_Car);
        sub->AddSubmenuOption("Bike", "Spawn a bike", Submenu_Bike);
        sub->AddSubmenuOption("Plane", "Spawn a plane", Submenu_Plane);
        sub->AddSubmenuOption("Jet", "Spawn a jet", Submenu_Jet);
        sub->AddSubmenuOption("Heli", "Spawn a helicopter", Submenu_Heli);
        sub->AddSubmenuOption("Osprey", "Spawn an osprey", Submenu_Osprey);
        sub->AddSubmenuOption("Cargobob", "Spawn a cargobob", Submenu_Cargobob);
        sub->AddSubmenuOption("Boat", "Spawn a boat", Submenu_Boat);
        sub->AddSubmenuOption("Tank", "Spawn a tank", Submenu_Tank);
    });

    // Initialize submenus for each vehicle type
    g_Menu->AddSubmenu("CAR", "Spawn a car", Submenu_Car, 8, [](Submenu* sub) {
        for (const auto& [key, value] : g_Config["Config"]["Vehicles"].items()) {
            if (value["type"] == "car") {
                sub->AddRegularOption(key, "Spawn " + key, [key] {
                    HandleVehicleSelection(key);
                });
            }
        }
    });

    g_Menu->AddSubmenu("BIKE", "Spawn a bike", Submenu_Bike, 8, [](Submenu* sub) {
        for (const auto& [key, value] : g_Config["Config"]["Vehicles"].items()) {
            if (value["type"] == "bike") {
                sub->AddRegularOption(key, "Spawn " + key, [key] {
                    HandleVehicleSelection(key);
                });
            }
        }
    });

    g_Menu->AddSubmenu("PLANE", "Spawn a plane", Submenu_Plane, 8, [](Submenu* sub) {
        for (const auto& [key, value] : g_Config["Config"]["Vehicles"].items()) {
            if (value["type"] == "plane") {
                sub->AddRegularOption(key, "Spawn " + key, [key] {
                    HandleVehicleSelection(key);
                });
            }
        }
    });

    g_Menu->AddSubmenu("JET", "Spawn a jet", Submenu_Jet, 8, [](Submenu* sub) {
        for (const auto& [key, value] : g_Config["Config"]["Vehicles"].items()) {
            if (value["type"] == "jet") {
                sub->AddRegularOption(key, "Spawn " + key, [key] {
                    HandleVehicleSelection(key);
                });
            }
        }
    });

    g_Menu->AddSubmenu("HELI", "Spawn a helicopter", Submenu_Heli, 8, [](Submenu* sub) {
        for (const auto& [key, value] : g_Config["Config"]["Vehicles"].items()) {
            if (value["type"] == "heli") {
                sub->AddRegularOption(key, "Spawn " + key, [key] {
                    HandleVehicleSelection(key);
                });
            }
        }
    });

    g_Menu->AddSubmenu("OSPREY", "Silonugget Approved", Submenu_Osprey, 8, [](Submenu* sub) {
        for (const auto& [key, value] : g_Config["Config"]["Vehicles"].items()) {
            if (value["type"] == "osprey") {
                sub->AddRegularOption(key, "Silonugget.com", [key] {
                    HandleVehicleSelection(key);
                });
            }
        }
    });

    g_Menu->AddSubmenu("CARGOBOB", "Spawn a cargobob", Submenu_Cargobob, 8, [](Submenu* sub) {
        for (const auto& [key, value] : g_Config["Config"]["Vehicles"].items()) {
            if (value["type"] == "cargobob") {
                sub->AddRegularOption(key, "Spawn " + key, [key] {
                    HandleVehicleSelection(key);
                });
            }
        }
    });

    g_Menu->AddSubmenu("BOAT", "Spawn a boat", Submenu_Boat, 8, [](Submenu* sub) {
        for (const auto& [key, value] : g_Config["Config"]["Vehicles"].items()) {
            if (value["type"] == "boat") {
                sub->AddRegularOption(key, "Spawn " + key, [key] {
                    HandleVehicleSelection(key);
                });
            }
        }
    });

    g_Menu->AddSubmenu("TANK", "Spawn a tank", Submenu_Tank, 8, [](Submenu* sub) {
        for (const auto& [key, value] : g_Config["Config"]["Vehicles"].items()) {
            if (value["type"] == "tank") {
                sub->AddRegularOption(key, "Spawn " + key, [key] {
                    HandleVehicleSelection(key);
                });
            }
        }
    });

    // Initialize other submenus
    g_OptionsSubmenu->Init();
}


void main() {
    g_Menu = std::make_unique<CNativeMenu>();

    InitializeMenu(); // Make sure to call InitializeMenu() before calling any other CNativeMenu (g_Menu) function
    g_Menu->GoToSubmenu(Submenu_EntryMenu); // We only need to do this manually ONCE. It's automatic. See comment inside function.

    if (!UIUtil::GetScreenDimensions()) {
        PRINT_WARN("Failed to get Red Dead Redemption 2 game window dimensions. The UI may be sized incorrectly.");
    }
    g_SoundEngine->Init();
    while (true) {
        // This is required. Do not remove.
        // This makes the menu render everything.
        g_Menu->Update();

        WAIT(0);
    }
}

#pragma warning(disable:28159)
void WaitAndDraw(unsigned ms) {
    DWORD time = GetTickCount() + ms;
    bool waited = false;
    while (GetTickCount() < time || !waited) {
        WAIT(0);
        waited = true;
        if (g_Menu) {
            g_Menu->Update();
        }
    }
}
#pragma warning(default:28159)

void ScriptMain() {
    main();
}
void vehicleupdateMain() {
    while (true) {
                g_SpawnSubmenu->UpdateWheels();
                g_SpawnSubmenu->TrackVehicleEntryExit();
                g_SpawnSubmenu->IncreaseVehicleSpeed();
                g_SpawnSubmenu->PlaneUpdate();
                g_SpawnSubmenu->HeliUpdate();
                g_SpawnSubmenu->OspreyUpdate();
                g_SpawnSubmenu->TankUpdate();
                g_SpawnSubmenu->ToggleEngine();
                g_SpawnSubmenu->DisplayVehicleStatus();
                g_SpawnSubmenu->FirstPersonCamAndRadius();
                g_SpawnSubmenu->HorseFinder();
                g_SpawnSubmenu->VehWeaponsUpdate();
                g_SpawnSubmenu->HoverCar();
                g_SoundEngine->CheckIfInVehicle();
                
				WAIT(0);
    }
}