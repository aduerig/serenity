/*
 * Copyright (c) 2020, Hüseyin Aslıtürk <asliturk@hotmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "CharacterMapFile.h"
#include <AK/ByteBuffer.h>
#include <AK/Utf8View.h>
#include <LibCore/File.h>

namespace Keyboard {

void CharacterMapFile::load_from_file_helper(size_t i, Keyboard::CharacterMapLayer& layer, Vector<u32>& codepoints)
{
    layer.entry_sizes[i] = codepoints.size();
    layer.entries[i] = new u32[codepoints.size()];
    for (size_t j = 0; j < codepoints.size(); j++) {
        layer.entries[i][j] = codepoints.at(j);
    }
}

ErrorOr<CharacterMapData> CharacterMapFile::load_from_file(DeprecatedString const& filename)
{
    auto path = filename;
    if (!path.ends_with(".json"sv)) {
        StringBuilder full_path;
        full_path.append("/res/keymaps/"sv);
        full_path.append(filename);
        full_path.append(".json"sv);
        path = full_path.to_deprecated_string();
    }

    auto file = TRY(Core::File::open(path, Core::File::OpenMode::Read));
    auto file_contents = TRY(file->read_until_eof());
    auto json_result = TRY(JsonValue::from_string(file_contents));
    auto const& json = json_result.as_object();

    Vector<Vector<u32>> map = read_map(json, "map");
    Vector<Vector<u32>> shift_map = read_map(json, "shift_map");
    Vector<Vector<u32>> alt_map = read_map(json, "alt_map");
    Vector<Vector<u32>> altgr_map = read_map(json, "altgr_map");
    Vector<Vector<u32>> shift_altgr_map = read_map(json, "shift_altgr_map");

    CharacterMapData character_map;
    for (size_t i = 0; i < CHAR_MAP_SIZE; i++) {
        load_from_file_helper(i, character_map.map, map.at(i));
        load_from_file_helper(i, character_map.shift_map, shift_map.at(i));
        load_from_file_helper(i, character_map.alt_map, alt_map.at(i));

        if (altgr_map.is_empty()) {
            // AltGr map was not found, using Alt map as fallback.
            load_from_file_helper(i, character_map.altgr_map, alt_map.at(i));
        } else {
            load_from_file_helper(i, character_map.altgr_map, altgr_map.at(i));
        }
        if (shift_altgr_map.is_empty()) {
            // Shift+AltGr map was not found, using Alt map as fallback.
            load_from_file_helper(i, character_map.shift_altgr_map, alt_map.at(i));
        } else {
            load_from_file_helper(i, character_map.shift_altgr_map, shift_altgr_map.at(i));
        }
    }

    return character_map;
}

Vector<Vector<u32>> CharacterMapFile::read_map(JsonObject const& json, DeprecatedString const& name)
{
    if (!json.has(name))
        return {};

    Vector<Vector<u32>> buffer;
    buffer.resize(CHAR_MAP_SIZE);

    // } else if (key_value.length() == 1) {
    //     buffer[i] = key_value.characters()[0];
    // } else {
    //     Utf8View m_utf8_view(key_value);
    //     buffer[i] = *m_utf8_view.begin();
    // }

    auto map_arr = json.get_array(name).value();
    for (size_t i = 0; i < map_arr.size(); i++) {
        auto key_value = map_arr.at(i).as_string();
        if (key_value.length() == 0) {
            buffer[i].append(0);
        } else {
            Utf8View m_utf8_view(key_value);
            for (auto it = m_utf8_view.begin(); it != m_utf8_view.end(); ++it) {
                buffer[i].append(*it);
            }
        }
    }

    return buffer;
}
}
