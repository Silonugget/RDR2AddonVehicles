// Licensed under the MIT License.

#pragma once


#include "common.hpp"
#include "Submenus/CVehAudio.hpp"
#include "UI/Menu.hpp"
#include "UI/UIUtil.h"
#include "Submenus/Examples.hpp"
#include "UI/json.hpp" // Include the JSON library
#include <fstream>

void ScriptMain();
void vehicleupdateMain(); // Declaration of vehicleupdateMain
void WaitAndDraw(unsigned ms);
extern bool autoWarp;
extern bool seatbelt;
extern bool attachHorse;
extern bool playSounds;
extern bool uiToggle;
extern bool hoverCar;