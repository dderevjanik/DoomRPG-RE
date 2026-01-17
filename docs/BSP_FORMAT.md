# DoomRPG BSP Map File Format

This document describes the binary format of `.bsp` map files used in DoomRPG.

## Overview

BSP files contain all map data including geometry (nodes, lines), sprites, events, bytecode scripts, strings, blockmap flags, and plane textures. The data is read sequentially using little-endian byte ordering.

## Byte Reading Functions

The engine uses these functions to read data from the binary file:

| Function | Size | Description |
|----------|------|-------------|
| `byteAtNext` | 1 byte | Reads unsigned byte |
| `shortAtNext` | 2 bytes | Reads signed 16-bit integer (little-endian: `[low][high]`) |
| `intAtNext` | 4 bytes | Reads signed 32-bit integer (little-endian: `[b0][b1][b2][b3]`) |
| `shiftCoordAt` | 1 byte | Reads byte and shifts left by 3 (`value << 3`) for coordinate scaling |

### Byte Order (Little-Endian)

```
short: (data[pos+1] << 8) | data[pos+0]
int:   (data[pos+3] << 24) | (data[pos+2] << 16) | (data[pos+1] << 8) | data[pos+0]
```

---

## File Structure

```
+---------------------------+
| Header                    |
+---------------------------+
| Nodes                     |
+---------------------------+
| Lines                     |
+---------------------------+
| Sprites                   |
+---------------------------+
| Tile Events               |
+---------------------------+
| ByteCodes                 |
+---------------------------+
| Strings                   |
+---------------------------+
| BlockMap                  |
+---------------------------+
| Plane Textures            |
+---------------------------+
```

---

## Section Details

### 1. Header (Offset 0)

| Offset | Size | Type | Field | Description |
|--------|------|------|-------|-------------|
| 0x00 | 16 | char[16] | mapName | Null-terminated map name string |
| 0x10 | 1 | byte | floorColorR | Floor color - Red component |
| 0x11 | 1 | byte | floorColorG | Floor color - Green component |
| 0x12 | 1 | byte | floorColorB | Floor color - Blue component |
| 0x13 | 1 | byte | ceilingColorR | Ceiling color - Red component |
| 0x14 | 1 | byte | ceilingColorG | Ceiling color - Green component |
| 0x15 | 1 | byte | ceilingColorB | Ceiling color - Blue component |
| 0x16 | 1 | byte | floorTex | Floor texture index |
| 0x17 | 1 | byte | ceilingTex | Ceiling texture index |
| 0x18 | 1 | byte | introColorR | Intro screen color - Red |
| 0x19 | 1 | byte | introColorG | Intro screen color - Green |
| 0x1A | 1 | byte | introColorB | Intro screen color - Blue |
| 0x1B | 1 | byte | loadMapID | Map ID for loading |
| 0x1C | 2 | short | mapSpawnIndex | Player spawn tile index |
| 0x1E | 1 | byte | mapSpawnDir | Player spawn direction |
| 0x1F | 2 | short | mapCameraSpawnIndex | Camera spawn tile index |

**Total Header Size: 33 bytes (0x21)**

**Note on Colors:**
- Floor/Ceiling colors are stored as BGR and converted to RGB565 format
- Intro color is stored as RGB and converted to ARGB format: `0xFF000000 | (R << 16) | (G << 8) | B`

---

### 2. Nodes Section

BSP tree nodes for spatial partitioning and rendering.

| Offset | Size | Type | Field | Description |
|--------|------|------|-------|-------------|
| 0 | 2 | short | nodesLength | Number of nodes |

**For each node (nodesLength times):**

| Size | Type | Field | Description |
|------|------|-------|-------------|
| 1 | shiftCoord | x1 | X1 coordinate (byte << 3) |
| 1 | shiftCoord | y1 | Y1 coordinate (byte << 3) |
| 1 | shiftCoord | x2 | X2 coordinate (byte << 3) |
| 1 | shiftCoord | y2 | Y2 coordinate (byte << 3) |
| 1 | byte | arg1_high | High byte of args1 |
| 1 | shiftCoord | arg1_low | Low portion of args1 (byte << 3) |
| 2 | short | args2_low | Low 16 bits of args2 |
| 2 | short | args2_high | High 16 bits of args2 |

**Node Structure Assembly:**
```c
args1 = (arg1_high << 16) | arg1_low
args2 = args2_low | (args2_high << 16)
```

**Node size: 10 bytes per node**

---

### 3. Lines Section

Wall/line definitions for map geometry.

| Offset | Size | Type | Field | Description |
|--------|------|------|-------|-------------|
| 0 | 2 | short | linesLength | Number of lines |

**For each line (linesLength times):**

| Size | Type | Field | Description |
|------|------|-------|-------------|
| 1 | shiftCoord | vert1.x | Vertex 1 X (byte << 3) |
| 1 | shiftCoord | vert1.y | Vertex 1 Y (byte << 3) |
| 1 | shiftCoord | vert2.x | Vertex 2 X (byte << 3) |
| 1 | shiftCoord | vert2.y | Vertex 2 Y (byte << 3) |
| 2 | short | texture | Texture ID |
| 4 | int | flags | Line flags (see below) |

**Line size: 10 bytes per line**

**Line Flags:**

| Bit | Hex | Description |
|-----|-----|-------------|
| 3 | 0x0008 | Offset +3 direction |
| 4 | 0x0010 | Offset -3 direction |
| 8 | 0x0100 | Vertical line (Y-axis aligned) |
| 9 | 0x0200 | Horizontal line (X-axis aligned) |
| 15 | 0x8000 | Reverse Z coordinate direction |

**Z Coordinate Calculation:**
```c
if (!(flags & 0x8000)) {
    vert1.z = 0;
    vert2.z = MAX(ABS(vert2.x - vert1.x), ABS(vert2.y - vert1.y));
} else {
    vert1.z = MAX(ABS(vert2.x - vert1.x), ABS(vert2.y - vert1.y));
    vert2.z = 0;
}
```

---

### 4. Sprites Section

Map sprite/entity definitions.

| Offset | Size | Type | Field | Description |
|--------|------|------|-------|-------------|
| 0 | 2 | short | numMapSprites | Number of sprites |

**For each sprite (numMapSprites times):**

| Size | Type | Field | Description |
|------|------|-------|-------------|
| 1 | shiftCoord | x | X position (byte << 3) |
| 1 | shiftCoord | y | Y position (byte << 3) |
| 1 | byte | info_low | Low 8 bits of info (sprite type) |
| 2 | short | info_high | High 16 bits of info (sprite flags) |

**Sprite size: 5 bytes per sprite**

**Info Field Assembly:**
```c
info = info_low | (info_high << 16)
```

**Sprite Info Bits:**

| Bits | Mask | Description |
|------|------|-------------|
| 0-8 | 0x1FF | Sprite ID (0-511) |
| 18 | 0x40000 | Is texture (not sprite) |
| 19 | 0x80000 | Y offset -1 |
| 20 | 0x100000 | Y offset +1 |
| 21 | 0x200000 | X offset +1 |
| 22 | 0x400000 | X offset -1 |
| 23 | 0x800000 | Has direction offset |
| 29 | 0x20000000 | Skip rendering |

**Render Mode:**
- Sprites 136, 137, 144: `renderMode = 7`
- All other sprites: `renderMode = 0`

---

### 5. Tile Events Section

Event triggers for map tiles.

| Offset | Size | Type | Field | Description |
|--------|------|------|-------|-------------|
| 0 | 2 | short | numTileEvents | Number of tile events |

**For each event (numTileEvents times):**

| Size | Type | Field | Description |
|------|------|-------|-------------|
| 4 | int | event | Event data (packed format) |

**Event size: 4 bytes per event**

**Event Data Format:**
```
Bits 0-9:   Tile index (0-1023)
Bits 19-24: Event flags (if set, marks as BIT_AM_EVENTS on automap)
```

---

### 6. ByteCodes Section

Script bytecode for map logic.

| Offset | Size | Type | Field | Description |
|--------|------|------|-------|-------------|
| 0 | 2 | short | numByteCodes | Number of bytecode entries |

**For each bytecode (numByteCodes times):**

| Size | Type | Field | Description |
|------|------|-------|-------------|
| 1 | byte | id | Bytecode instruction ID |
| 4 | int | arg1 | First argument |
| 4 | int | arg2 | Second argument |

**ByteCode size: 9 bytes per entry**

**Notable Bytecode IDs:**
- `34` (EV_CHANGESPRITE): Changes sprite appearance. `arg1 >> 13` = new sprite ID

---

### 7. Strings Section

Map text strings for dialogs, messages, etc.

| Offset | Size | Type | Field | Description |
|--------|------|------|-------|-------------|
| 0 | 2 | short | numStrings | Number of strings |

**For each string (numStrings times):**

| Size | Type | Field | Description |
|------|------|-------|-------------|
| 2 | short | strSize | Length of string |
| strSize | char[] | data | String data (NOT null-terminated in file) |

---

### 8. BlockMap Section

Collision/traversal flags for 32x32 tile grid (1024 tiles total).

**Format:** 256 bytes, each byte contains flags for 4 tiles (2 bits each).

```
For j = 0 to 255:
    flags = read_byte()
    mapFlags[i+0] |= (flags >> 0) & 3   // Bits 0-1
    mapFlags[i+1] |= (flags >> 2) & 3   // Bits 2-3
    mapFlags[i+2] |= (flags >> 4) & 3   // Bits 4-5
    mapFlags[i+3] |= (flags >> 6) & 3   // Bits 6-7
    i += 4
```

**Total BlockMap Size: 256 bytes**

**Flag Values:**
- `0`: Passable
- `1`: Blocked (wall)
- `2`: Special (door, etc.)
- `3`: Reserved

---

### 9. Plane Textures Section

Floor and ceiling texture indices per tile.

**Format:** 2 planes × 1024 tiles = 2048 bytes

```
For plane = 0 to 1:    // 0 = floor, 1 = ceiling
    For tile = 0 to 1023:
        textureId = read_byte()
        // Store in planeTextures[(plane * 1024) + tile]
```

**Maximum unique textures per map: 24**

---

## Complete File Layout Example

```
Offset    Size      Section
------    ----      -------
0x0000    33        Header
0x0021    2         Nodes count
0x0023    N*10      Nodes data
...       2         Lines count
...       N*10      Lines data
...       2         Sprites count
...       N*5       Sprites data
...       2         Tile events count
...       N*4       Tile events data
...       2         ByteCodes count
...       N*9       ByteCodes data
...       2         Strings count
...       Variable  Strings data
...       256       BlockMap
...       2048      Plane textures
```

---

## Related Files

| File | Description |
|------|-------------|
| `/mappings.bin` | Texture/sprite ID mappings and offsets |
| `/bitshapes.bin` | Sprite shape data |
| `/texels.bin` | Texture pixel data |
| `/palettes.bin` | Color palette data |
| `/menu.bsp` | Menu screen map |

---

## Code References

Main loading functions in `Render.c`:
- `Render_beginLoadMap()` - Loads header
- `Render_beginLoadMapData()` - Loads all other sections

Byte reading utilities in `DoomRPG.c`:
- `DoomRPG_byteAtNext()`
- `DoomRPG_shortAtNext()`
- `DoomRPG_intAtNext()`
- `DoomRPG_shiftCoordAt()`
