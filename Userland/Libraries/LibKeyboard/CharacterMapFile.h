/*
 * Copyright (c) 2020, Hüseyin Aslıtürk <asliturk@hotmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/JsonObject.h>
#include <LibKeyboard/CharacterMapData.h>

namespace Keyboard {

class CharacterMapFile {

public:
    static void load_from_file_helper(size_t, Keyboard::CharacterMapLayer&, Vector<u32>&);
    static ErrorOr<CharacterMapData> load_from_file(DeprecatedString const& filename);

private:
    static Vector<Vector<u32>> read_map(JsonObject const& json, DeprecatedString const& name);
};

}
