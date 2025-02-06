# Red Dead Redemption 2 Addon Vehicles Mod (C++)

This project contains C++ source code for the RDR2 Addon Vehicles mod. It provides logic for various vehicle types, spawning, handling, and the menu.

## Installation

You can find the latest release and installation instructions on **[NexusMods](https://www.nexusmods.com/reddeadredemption2/mods/5285).**

### Requirements:
- **AB Scripthook**  
- **LML Mod Loader**  
- **Advanced Graphics Setting** (must be set to **Vulkan** in-game)

## Supported Vehicles

This project includes logic for the following vehicle types:
- **Cars**  
- **Motorbikes**  
- **Propeller Planes**  
- **Jets**  
- **Helicopters**  
- **Ospreys** (with VTOL capabilities)  
- **Cargobob**  
- **Tanks**  
- **Boats**  

## Help & Info

- **Output Directory (Visual Studio)**:  
  If using **Visual Studio**, you **must** set an **output directory** for the project.

- **Menu System**:  
  - The **vehicle menu** is created in `script.cpp`.  
  - The menu uses **static types**, but dynamically fills entries using `vehicleconfig.json`, which should be placed in the **game directory**.

- **Vehicle Spawning & Handling**:  
  - Found in `Submenus/Examples.cpp`.  
  - Includes logic for all **vehicle types, weapons, and spawning mechanics**.

- **Menu Framework**:  
  - Built using **[NativeMenuBase](https://github.com/Halen84/RDR2-Native-Menu-Base/tree/master/src/NativeMenuBase)**.

  
- If you make improvements, **please share knowledge and contribute to this repo**.  

## To-Do List (if/when I ever have time again...)

- **Refactor `Examples.cpp`**:  
  - Moving vehicles to their own files and remoing redundant loops could **increase FPS** on lower-end systems.  

- **Improve Menu**:  
  - Generate vehicle types dynamically from the config instead of just vehicles.  

- **Error Checking & Debugging**:  
  - Implement additional checks to improve stability.  

## Credits

- **Drao (Luman Studio)** – Shared knowledge on **car wheel rotation**.  
- **RicX (zelbeus)** – Created an **open-source car prop system** for RedM.  
- **WesternAndCo** – Helped with **getting started in C++ modding**.  
- **NotBakou** – Shared knowledge on **LML addon `.ydr` files** and provided a **plane prop model**.  
- **Mooreiche** – Shared **config files for driveable coach4 meta files**.  
- **Halen84** – Created **NativeMenuBase**, which powers the mod menu.  

---

