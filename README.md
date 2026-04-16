# TEINJECT
Framework for creating and loading DLL mods for the game The End is Nigh.
Project is in active development, thus details may change in the Future.

# Content
- teinject - main framework project
- teinject.loader - auxiliary loader exe project
- example_mod - showcases few possibilities through implementation of a number of questionable mechanics. It is intended to be a starting point to create your own mod and should minimise project configuration burden.
- eel - utilities library, referenced by teinject and exampled_mod
- TheEnd.exe - TEIN exe from Finxx's [https://github.com/Finxx1/TEiN-SDK](TEIN-SDK) with disabled dynamic address loading (required for patching)

# Creating mods
TODO

# Installation
Copy
- TheEnd.exe
- teinject.dll
- teinject.loader.exe
into TEIN installation directory (copy files to other location if any file name collisions happen). 
Optionally drop example_mod.dll into 'mods' subdirectory (create if none) if you want to see how it works.

**Launch the game through teinject.loader.exe.**

# Tileset settings
TEINJECT implements a number of new tileset settings. They might be moved to a separate submod. Related features should behave like in vanilla if no setting for it were specified at all.
- `teinject.enable_hollows_water` (bool) whether water is rendered behind the tiles as in the Hollows.
- `teinject.enable_particles` (bool) whether particles are enabled (in carts they are disabled by default, and in regular areas they are enabled)
- `teinject.enable_ash_trail` (bool) whether Ash is rendered with a "tail" which elongates with higher velocity.
- `teinject.enable_retro_gfx_tweaks` (bool) whether a set of graphics tweaks are applied (bullet rotation, chain links rotation, etc)
Following settings consist of objects `{common <value>, per_instance [<value> <value> ...]}` where `common` is a modifier applied to all entities of related type and `per_instance` is an array of modifiers where each value is applied to a particular entity of related type as if they were enumerated by their X and then by Y coordinates (zero is left bottom of the screen). If an entity doesn't have value in `per_instance` array it will use one from `common` field. If `common` is absent too, vanilla behavior applies.
Example: `teinject.static_turret_intervals {per_instance [[0.05 0.05 0.3 0.05 0.3 0.7] [0.05 0.2 0.05 0.5]]}`
List:
- `teinject.retinara_timer` (double) time of continuous unobstructed line of sight with Retinara before it kills (vanilla is 0.5)
- `teinject.static_turret_intervals` (double[]) time interval between consecutive shots, repeats from the start when an end of array is reached (for static turrets)
- `teinject.aim_turret_intervals` (double[]) same as above for aiming turrets
- `teinject.static_turret_bullet_velocity` (double[]) velocity of bullets, repeats from the start when an end of array is reached (for static turrets)
- `teinject.aim_turret_bullet_velocity` (double[]) same as above for aiming turrets
- `teinject.mine_extra_jumps` (int) extra jumps mine can endure before its destruction
- `teinject.xfloast_extra_jumps` (int) extra jumps x-floast can endure before its destruction
