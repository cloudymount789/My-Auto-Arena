# My-Auto-Arena

`My-Auto-Arena` is a C++ auto-battler project scaffold for `Synera: Synergy Auto-Arena`.

## File Tree

```text
.
├── CMakeLists.txt
├── src
│   ├── main.cpp
│   └── core
│       ├── Tile.h / Tile.cpp
│       ├── Board.h / Board.cpp
│       └── Unit.h / Unit.cpp
├── tests
│   ├── CMakeLists.txt
│   ├── test_board.cpp
│   └── test_unit.cpp
└── docs
    ├── development-plan.md
    ├── dev_log.md
    └── code_analysis.md
```

## Core Classes and Data Structures

- `Tile`
  - Represents a single board cell with position and occupancy.
- `Board`
  - Owns board tiles and bench slots.
  - Exposes placement/clear/query interfaces for board and bench.
- `Unit`
  - Base class of all units with common combat stats.
  - Provides damage and mana accumulation behavior.
- `WarriorUnit`, `MageUnit`
  - First two concrete unit classes for early-phase development.

## Algorithm Notes

- **Grid Placement Validation**
  - Every placement call validates bounds first, then occupancy.
- **Occupancy Rule**
  - One tile holds at most one unit.
- **Bench Rule**
  - One bench slot holds at most one unit.
- **Pathfinding / Targeting / Synergy**
  - Planned for later phases:
    - Pathfinding: BFS first, then A* optional
    - Targeting: nearest enemy with tie-breakers
    - Synergy: trait-threshold based buffs

## Helper Functions

- No standalone global helper functions are introduced yet.
- Utility logic currently stays inside class member functions to keep ownership and invariants clear.