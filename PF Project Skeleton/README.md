# Switchback Rails - Railway Simulation Game

> **Build requirement:** C++17 and SFML 3.x. The graphical viewer uses the SFML 3 event API; SFML 2.x is not supported by this version.

A deterministic railway simulation implementing deferred switch mechanics, dynamic train scheduling, and collision detection.

## Steps to Start

Follow these steps to set up and run the project:

1. **Install Required Libraries**
   Install a C++17 compiler, GNU Make, and SFML 3 development libraries. The supplied `libraries.sh` installs the distribution's `libsfml-dev`; verify that its version is SFML 3.x before building.
   ```bash
   bash libraries.sh
   pkg-config --modversion sfml-graphics
   ```

2. **Clean Previous Build Files**
   ```bash
   make clean
   ```

3. **Build the Project**
   ```bash
   make clean
   make
   ```

4. **Run the Simulation**
   ```bash
   ./switchback_rails data/levels/easy_level.lvl
   ./switchback_rails data/levels/medium_level.lvl
   ./switchback_rails data/levels/hard_level.lvl
   ./switchback_rails data/levels/complex_network.lvl
   ```

## Project Structure

```
├── core/              # Core simulation logic
│   ├── simulation.*   # Main tick loop with 7-phase execution
│   ├── trains.*       # Train movement, routing, and collision detection
│   ├── switches.*     # Switch counter logic and deferred flips
│   ├── grid.*         # Grid utilities and track validation
│   └── io.*           # Level file parsing and CSV output
├── sfml/              # SFML visual interface
├── data/levels/       # Level files (.lvl)
└── out/               # Generated traces and metrics

```

## Build & Run

```bash
make            # Compile the game
make run        # Run with default level
make clean      # Clean build files

# Run specific level
./switchback_rails data/levels/simple_test.lvl
./switchback_rails data/levels/full_network.lvl
./switchback_rails data/levels/complex_network.lvl
```

## Graphical Viewer

Launch the SFML 3 viewer with:

```bash
./switchback_rails data/levels/easy_level.lvl --view
```

Run headless with an optional maximum tick count:

```bash
./switchback_rails data/levels/easy_level.lvl 200
```

## Controls

- **SPACE**: Pause/Resume simulation
- **. (period)**: Step forward one tick
- **Left-click**: Toggle safety tile (=)
- **Right-click**: Toggle switch state
- **Middle-drag**: Pan camera
- **Mouse wheel**: Zoom in/out
- **ESC**: Exit and save metrics

## Levels

1. **easy_level.lvl** - 2 trains, simple railway with minimal switches (NORMAL weather)
2. **medium_level.lvl** - 5 trains, intersecting routes with crossings (NORMAL weather)
3. **hard_level.lvl** - 8 trains, complex network challenge with multiple intersections (NORMAL weather)
4. **complex_network.lvl** - 10 trains, complex interconnected network (NORMAL weather)

All trains spawn from 'S' (source) tiles and navigate to 'D' (destination) tiles.

### Changing Weather

Edit any `.lvl` file and change the `WEATHER:` line:
- `NORMAL` - Constant speed, standard behavior
- `RAIN` - Occasional slowdowns every 5 moves
- `FOG` - Signal lights delayed by 1 tick (visual challenge)

### Collision Priority System 🚂

When two trains would collide, instead of crashing both, the system uses **distance-based priority**:

- **Higher Distance = Higher Priority**: The train further from its destination gets to move
- **Lower Distance = Waits**: The closer train waits for the next tick
- **Equal Distance = Both Crash**: Tie-breaker scenario

**Example**: Train A (20 tiles away) meets Train B (8 tiles away) at a crossing
→ Train A continues, Train B waits

This creates more realistic and efficient train traffic flow!

## Output Files

After simulation, check `out/` directory:
- `trace.csv` - Complete train movement history
- `switches.csv` - Switch state changes per tick
- `signals.csv` - Signal light states (GREEN/YELLOW/RED)
- `metrics.txt` - Final statistics and efficiency metrics

## Features

✓ Deferred switch flips (after movement)  
✓ Direction-conditioned switches (PER_DIR & GLOBAL)  
✓ Spawn queue (trains wait if spawn tile occupied)  
✓ **Distance-based collision priority** (higher distance = higher priority)  
✓ 3 collision types (same-destination, head-on swap, crossing)  
✓ Signal lights (GREEN/YELLOW/RED)  
✓ Weather effects (NORMAL/RAIN/FOG)  
✓ Safety tiles (=) for 1-tick delay  
✓ Emergency halt (3×3 zone)  
✓ Deterministic simulation with SEED  
✓ Fast spawn timing (every 4 ticks)  

## Specification Compliance

Fully compliant with the Switchback Rails specification:
- 7-phase tick execution (spawn → determination → counters → flip queue → movement → deferred flip → arrivals)
- Deterministic spawn rules with queue retry
- All collision detection types implemented
- Deferred switch flips with counter reset
- Signal lights per specification
- Complete weather system
- Proper evidence file generation

Built using only Programming Fundamentals concepts (no classes/structs).
