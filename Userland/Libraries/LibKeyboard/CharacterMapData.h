/*
 * Copyright (c) 2020, Hüseyin Aslıtürk <asliturk@hotmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Types.h>

#define CHAR_MAP_SIZE 0x80

namespace Keyboard {

struct CharacterMapLayer {
    size_t entry_count;
    u8 entry_sizes[CHAR_MAP_SIZE];
    u32* entries[CHAR_MAP_SIZE];
};

struct CharacterMapData {
    struct CharacterMapLayer map;
    struct CharacterMapLayer shift_map;
    struct CharacterMapLayer alt_map;
    struct CharacterMapLayer altgr_map;
    struct CharacterMapLayer shift_altgr_map;
};

// old impl, probably remove before PR
struct CharacterMapDataSingleU32 {
    u32 map[CHAR_MAP_SIZE];
    u32 shift_map[CHAR_MAP_SIZE];
    u32 alt_map[CHAR_MAP_SIZE];
    u32 altgr_map[CHAR_MAP_SIZE];
    u32 shift_altgr_map[CHAR_MAP_SIZE];
};

}
