#ifndef NAMED_POSET_COLLECTIONS_H
#define NAMED_POSET_COLLECTIONS_H

#ifndef __cplusplus
#include <stdbool.h>
#include <stddef.h>

long npc_new_collection(void);
void npc_delete_collection(long id);
bool npc_new_poset(long id, char const *name);
void npc_delete_poset(long id, char const *name);
bool npc_copy_poset(long id, char const *name_dst, char const *name_src);
char const *npc_first_poset(long id);
char const *npc_next_poset(long id, char const *name);
bool npc_add_relation(long id, char const *name, size_t x, size_t y);
bool npc_is_relation(long id, char const *name, size_t x, size_t y);
bool npc_remove_relation(long id, char const *name, size_t x, size_t y);
size_t npc_size(void);
size_t npc_poset_size(void);
size_t npc_collection_size(long id);

#else
#include <cstddef>

namespace cxx
{
long npc_new_collection(void);
void npc_delete_collection(long id);
bool npc_new_poset(long id, char const *name);
void npc_delete_poset(long id, char const *name);
bool npc_copy_poset(long id, char const *name_dst, char const *name_src);
char const *npc_first_poset(long id);
char const *npc_next_poset(long id, char const *name);
bool npc_add_relation(long id, char const *name, size_t x, size_t y);
bool npc_is_relation(long id, char const *name, size_t x, size_t y);
bool npc_remove_relation(long id, char const *name, size_t x, size_t y);
size_t npc_size(void);
size_t npc_poset_size(void);
size_t npc_collection_size(long id);
} // namespace cxx
#endif

#endif