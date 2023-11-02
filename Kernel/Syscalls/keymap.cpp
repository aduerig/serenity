/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <Kernel/Devices/HID/Management.h>
#include <Kernel/Tasks/Process.h>

namespace Kernel {

constexpr size_t map_name_max_size = 50;

ErrorOr<void> Process::setkeymap_helper(size_t i, Keyboard::CharacterMapLayer& layer, Keyboard::CharacterMapLayer const& layer_from_user)
{
    layer.entry_sizes[i] = layer_from_user.entry_sizes[i];
    layer.entries[i] = (u32*)kmalloc(sizeof(u32) * layer_from_user.entry_sizes[i]);
    return TRY(copy_n_from_user(layer.entries[i], layer_from_user.entries[i], sizeof(u32) * layer_from_user.entry_sizes[i]));
}

ErrorOr<FlatPtr> Process::sys$setkeymap(Userspace<Syscall::SC_setkeymap_params const*> user_params)
{
    VERIFY_NO_PROCESS_BIG_LOCK(this);
    TRY(require_promise(Pledge::setkeymap));

    auto credentials = this->credentials();
    if (!credentials->is_superuser())
        return EPERM;

    auto params = TRY(copy_typed_from_user(user_params));

    Keyboard::CharacterMapData character_map_data;

    for (size_t i = 0; i < CHAR_MAP_SIZE; i++) {
        TRY(setkeymap_helper(i, character_map_data.map, params.map));
        TRY(setkeymap_helper(i, character_map_data.shift_map, params.shift_map));
        TRY(setkeymap_helper(i, character_map_data.alt_map, params.alt_map));
        TRY(setkeymap_helper(i, character_map_data.altgr_map, params.altgr_map));
        TRY(setkeymap_helper(i, character_map_data.shift_altgr_map, params.shift_altgr_map));
    }

    auto map_name = TRY(get_syscall_path_argument(params.map_name));
    if (map_name->length() > map_name_max_size)
        return ENAMETOOLONG;

    HIDManagement::the().set_maps(move(map_name), character_map_data);
    return 0;
}

ErrorOr<void> Process::getkeymap_helper(size_t i, Keyboard::CharacterMapLayer const& layer, Keyboard::CharacterMapLayer& layer_from_user)
{
    layer_from_user.entry_sizes[i] = layer.entry_sizes[i];
    layer_from_user.entries[i] = new u32[layer.entry_sizes[i]];
    // layer_from_user.entries[i] = (u32*)malloc(sizeof(u32) * layer.entry_sizes[i]);
    return TRY(copy_n_from_user(layer_from_user.entries[i], layer.entries[i], sizeof(u32) * layer.entry_sizes[i]));
}

ErrorOr<FlatPtr> Process::sys$getkeymap(Userspace<Syscall::SC_getkeymap_params const*> user_params)
{
    VERIFY_NO_PROCESS_BIG_LOCK(this);
    TRY(require_promise(Pledge::getkeymap));
    auto params = TRY(copy_typed_from_user(user_params));

    return HIDManagement::the().keymap_data().with([&](auto const& keymap_data) -> ErrorOr<FlatPtr> {
        if (params.map_name.size < keymap_data.character_map_name->length())
            return ENAMETOOLONG;

        TRY(copy_to_user(params.map_name.data, keymap_data.character_map_name->characters(), keymap_data.character_map_name->length()));

        auto const& character_maps = keymap_data.character_map;
        for (size_t i = 0; i < CHAR_MAP_SIZE; i++) {
            TRY(getkeymap_helper(i, character_maps.map, params.map));
            TRY(getkeymap_helper(i, character_maps.shift_map, params.shift_map));
            TRY(getkeymap_helper(i, character_maps.alt_map, params.alt_map));
            TRY(getkeymap_helper(i, character_maps.altgr_map, params.altgr_map));
            TRY(getkeymap_helper(i, character_maps.shift_altgr_map, params.shift_altgr_map));
        }
        return 0;
    });
}

}
