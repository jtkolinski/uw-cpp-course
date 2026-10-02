# Playlists

Implement `cxx::playlist<T, P>`, a container for a sequence of tracks of type `T`
with playback parameters of type `P` for each occurrence.

A track may appear multiple times with different parameters. Since tracks can
be large, store only one copy of each distinct track and a separate set of
parameters for each occurrence. The container owns its data and must minimize
unnecessary copies.

## Main operations

- Append a track with playback parameters, inspect or remove the first entry,
  remove all occurrences of a track, clear the playlist, and query its size.
- Traverse entries in playback order, including repetitions, using
  `play_iterator`.
- Traverse distinct tracks in sorted order using `sorted_iterator`, and query
  how many times each track occurs.
- Read or modify the playback parameters of an entry through its iterator.

Accessing or removing the first entry of an empty playlist throws
`std::out_of_range`. Removing a track that is absent throws
`std::invalid_argument`.

## Copying and exception safety

Use **copy-on-write**: copies share storage until a modification requires an
independent copy. Exposing a mutable parameter reference also prevents sharing
until that reference is invalidated by a subsequent structural modification.
Moving a playlist must leave the source empty and valid.

Every operation must provide at least the **strong exception guarantee**:
propagate exceptions without changing the observable state. Failed modifications
must also preserve existing iterators. Operations such as moving and destruction
must not throw.

When no storage copy is needed, copying a playlist, accessing its front, removing
its front, and querying its size must take constant time; appending takes
`O(log n)`. Copying the underlying storage takes `O(n log n)`, where `n` counts
all track occurrences.

*Condensed English summary of the original assignment.*
