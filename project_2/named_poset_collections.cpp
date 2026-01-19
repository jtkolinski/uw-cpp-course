#ifndef N
#define N 32
#endif
#include "named_poset_collections.h"

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <cctype>
#include <functional>
#include <limits>
#include <map>
#include <ranges>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
using Poset = std::array<std::bitset<N>, N>;
using NamedPosetMap = std::map<std::string, Poset>;
using CollectionContainer = std::unordered_map<long, NamedPosetMap>;

CollectionContainer &get_collections()
{
    static CollectionContainer collections;
    return collections;
}

auto &get_collection_map(long id)
{
    return get_collections()[id];
}
long get_new_collection_id()
{
    static bool all_used = false;
    static long counter = 0;
    if (all_used) {
        return -1;
    }
    if (counter == std::numeric_limits<long>::max()) {
        all_used = true;
    }
    return counter++;
}

bool is_valid_name(char const *name)
{
    if (name == nullptr)
        return false;
    std::string str = name;
    if (str.empty())
        return false;
    return std::ranges::all_of(str.begin(), str.end(), [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) or c == '_';
    });
}

bool is_valid_element(size_t x)
{
    return x < static_cast<size_t>(N);
}

bool collection_exists(long id)
{
    return get_collections().contains(id);
}
bool name_exists(long id, char const *name)
{
    if (!is_valid_name(name))
        return false;
    if (!collection_exists(id))
        return false;
    return get_collection_map(id).contains(name);
}

long _npc_new_collection(void)
{
    long id = get_new_collection_id();
    if (id != -1) {
        get_collections()[id] = NamedPosetMap();
    }
    return id;
}

void _npc_delete_collection(long id)
{
    if (collection_exists(id)) {
        get_collections().erase(id);
    }
}

bool _npc_new_poset(long id, char const *name)
{
    if (!is_valid_name(name) or !collection_exists(id) or
        name_exists(id, name)) {
        return false;
    }
    get_collection_map(id).emplace(name, Poset());
    return true;
}

void _npc_delete_poset(long id, char const *name)
{
    if (collection_exists(id) and name_exists(id, name)) {
        get_collection_map(id).erase(name);
    }
}

bool _npc_copy_poset(long id, char const *name_dst, char const *name_src)
{
    if (!is_valid_name(name_dst) or !is_valid_name(name_src)) {
        return false;
    }
    if (!name_exists(id, name_src)) {
        return false;
    }
    get_collection_map(id)[name_dst] = get_collection_map(id)[name_src];
    return true;
}

char const *_npc_first_poset(long id)
{
    if (!collection_exists(id))
        return nullptr;
    if (get_collection_map(id).empty())
        return nullptr;
    return get_collection_map(id).begin()->first.c_str();
}

char const *_npc_next_poset(long id, char const *name)
{
    if (!name_exists(id, name))
        return nullptr;
    auto next_poset_it = std::next(get_collection_map(id).find(name));
    if (next_poset_it != get_collection_map(id).end()) {
        return next_poset_it->first.c_str();
    }
    return nullptr;
}

bool _npc_add_relation(long id, char const *name, size_t x, size_t y)
{
    if (!name_exists(id, name))
        return false;
    if (!is_valid_element(x) or !is_valid_element(y))
        return false;

    Poset &poset = get_collection_map(id)[name];
    if (x == y or poset[x].test(y) or poset[y].test(x))
        return false;

    // Domknięcie przechodnie
    poset[x] |= poset[y];
    poset[x].set(y, 1);
    for (size_t i = 0; i < poset.size(); ++i) {
        if (poset[i].test(x)) {
            poset[i] |= poset[y];
            poset[i].set(y, 1);
        }
    }

    return true;
}

bool _npc_is_relation(long id, char const *name, size_t x, size_t y)
{
    if (!name_exists(id, name))
        return false;
    if (!is_valid_element(x) or !is_valid_element(y))
        return false;
    if (x == y)
        return true;

    Poset &poset = get_collection_map(id)[name];
    return poset[x].test(y);
}

bool _npc_remove_relation(long id, char const *name, size_t x, size_t y)
{
    if (!name_exists(id, name))
        return false;
    if (!is_valid_element(x) or !is_valid_element(y))
        return false;
    if (x == y)
        return false;

    Poset &poset = get_collection_map(id)[name];
    if (!poset[x].test(y))
        return false;

    for (size_t i = 0; i < poset.size(); ++i) {
        if (poset[x].test(i) and poset[i].test(y))
            return false;
    }
    poset[x].flip(y);

    return true;
}

size_t _npc_size(void)
{
    return get_collections().size();
}
size_t _npc_poset_size(void)
{
    return static_cast<size_t>(N);
}
size_t _npc_collection_size(long id)
{
    if (!collection_exists(id))
        return 0;
    return get_collection_map(id).size();
}

} // namespace

extern "C" {
long npc_new_collection(void)
{
    return _npc_new_collection();
}
void npc_delete_collection(long id)
{
    return _npc_delete_collection(id);
}
bool npc_new_poset(long id, char const *name)
{
    return _npc_new_poset(id, name);
}
void npc_delete_poset(long id, char const *name)
{
    return _npc_delete_poset(id, name);
}
bool npc_copy_poset(long id, char const *name_dst, char const *name_src)
{
    return _npc_copy_poset(id, name_dst, name_src);
}
char const *npc_first_poset(long id)
{
    return _npc_first_poset(id);
}
char const *npc_next_poset(long id, char const *name)
{
    return _npc_next_poset(id, name);
}
bool npc_add_relation(long id, char const *name, size_t x, size_t y)
{
    return _npc_add_relation(id, name, x, y);
}
bool npc_is_relation(long id, char const *name, size_t x, size_t y)
{
    return _npc_is_relation(id, name, x, y);
}
bool npc_remove_relation(long id, char const *name, size_t x, size_t y)
{
    return _npc_remove_relation(id, name, x, y);
}
size_t npc_size(void)
{
    return _npc_size();
}
size_t npc_poset_size(void)
{
    return _npc_poset_size();
}
size_t npc_collection_size(long id)
{
    return _npc_collection_size(id);
}

} // extern "C"

namespace cxx
{
long npc_new_collection(void)
{
    return _npc_new_collection();
}
void npc_delete_collection(long id)
{
    return _npc_delete_collection(id);
}
bool npc_new_poset(long id, char const *name)
{
    return _npc_new_poset(id, name);
}
void npc_delete_poset(long id, char const *name)
{
    return _npc_delete_poset(id, name);
}
bool npc_copy_poset(long id, char const *name_dst, char const *name_src)
{
    return _npc_copy_poset(id, name_dst, name_src);
}
char const *npc_first_poset(long id)
{
    return _npc_first_poset(id);
}
char const *npc_next_poset(long id, char const *name)
{
    return _npc_next_poset(id, name);
}
bool npc_add_relation(long id, char const *name, size_t x, size_t y)
{
    return _npc_add_relation(id, name, x, y);
}
bool npc_is_relation(long id, char const *name, size_t x, size_t y)
{
    return _npc_is_relation(id, name, x, y);
}
bool npc_remove_relation(long id, char const *name, size_t x, size_t y)
{
    return _npc_remove_relation(id, name, x, y);
}
size_t npc_size(void)
{
    return _npc_size();
}
size_t npc_poset_size(void)
{
    return _npc_poset_size();
}
size_t npc_collection_size(long id)
{
    return _npc_collection_size(id);
}

} // namespace cxx
