# Half-Life Co-op / SevenKewp plugins
This repo exists primarly to support custom maps for the [Half-Life Co-op](https://github.com/wootguy/SevenKewp) mod. I expect mostly Sven Co-op map script ports and HL mod entities will go here. Wacky/fun server plugins will go somwehere else.

Plugins for Half-Life Co-op are compiled into .dll/.so files and loaded by the game's plugin manager, similarly to metamod. Unlike AngelScript, there is no separation between map and server code (map scripts and plugin scripts). Everything is a plugin, but you can tell the game to load a plugin only on specific maps by using using the `map_plugin` cfg command.

Development of these plugins is nearly the same as coding for the mod directly. You'll use the same headers and create entities in the same way. Hooks are used to add code into existing game functions.

## Installation / dev setup / documentation / etc.
Coming eventually...

but here's some basic instructions for building:
- `git clone --recurse-submodules https://github.com/wootguy/SevenKewp_plugins`
- run `build_all.bat` or `build_all.sh`
- copy the DLLs or SOs to `valve_downloads/plugins/maps/`

Check the plugins here to see how to create custom entities and stuff.

<details>
   <summary>Reasoning for making yet another plugin system</summary>  
  
## Why not AngelScript? 
In my experience with Sven Co-op, AngelScript has terrible performance. It's useful for things that are triggered occasionally, but once you need to process things every frame the engine starts to choke. On top of that, there is no performance profiler to help you find which scripts are causing slowdowns. `perf` will tell you that angelscript is hogging the cpu, but the symbol names will be too generic.
<details>
  <summary>Useless perf output</summary>
   
  ![image](https://github.com/wootguy/SevenKewp_plugins/assets/12087544/b64f2a99-a50e-4498-b2df-d032ec6e3ece)  
  
  This is what I often saw on a populated server with lots of metamod and angelscript plugins. Pretty much useless. Yep, lots of angelscript stuff is happening. Yep, I've got metamod installed - there's the hook that perf thinks everything runs inside.
</details>

Debugging crashes is also harder with AngelScript because there is not enough information in the core dumps. Sometimes there will be a clue on what type of function was running when the game crashed, but there's no script or plugin name listed. If you have dozens of scripts it becomes a guessing game to find which script caused a crash. Adding print statements to the beginning and end of every function of every script is something I've had to do in the past to pinpoint an occasional crash. It was a tedious and error-prone process and in the end there are still crashes I never found the cause for.

## Why not Metamod?
Metamod makes profiling and debugging harder because every engine/game API call is passed through metamod's main hook function. It can look like 90% of the time is spent in these hook functions, which is techinically true but also misleading because that's not where the actual work is being done.

Stepping through the code with a debugger is harder because when you try to step into an engine function from the game code, you'll hit metamod instead. You then need to step through a few more function calls and for-loops until you get to the engine function you were trying to step into.

Those reasons alone aren't dealbreakers, but metamod also has poor performance on Linux. I don't know why that is, but I think plugin performance can be better if the plugin manager is integrated into the game code. An update to metamod might also be just as effective though. Anyway, it shouldn't be much effort to port plugins to/from metamod if one system turns out better than the other.
</details>