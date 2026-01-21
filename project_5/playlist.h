#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <algorithm>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <stdexcept>
#include <utility>

namespace cxx {
template <typename T, typename P> class playlist {
  private:
    class playlist_data {
      public:
        // Forward declarations for circular dependencies between ListMetadata
        // and MapMetadata.
        struct ListMetadata;
        struct MapMetadata;

        using SongMap = std::map<T, MapMetadata>;
        using SongList = std::list<ListMetadata>;

        // Holds list of iterators to song occurrences in the main list.
        struct MapMetadata {
            std::list<typename SongList::iterator> songs;
        };
        // Holds iterator to map entry (for track data) and parameters.
        struct ListMetadata {
            typename SongMap::iterator song_info;
            P song_params;
        };

        SongList song_list;
        SongMap song_map;
        bool shareable = true;

        playlist_data() = default;
        playlist_data(const playlist_data &other)
            : song_list(), song_map(), shareable(true) {
            for (const auto &it : other.song_list)
                push_back(it.song_info->first, it.song_params);
        }
        playlist_data(playlist_data &&other) = default;

        // Transactional insert to ensure strong exception guarantee.
        void push_back(const T &track, const P &params) {
            auto [it, inserted] = song_map.try_emplace(track);
            try {
                auto list_it = song_list.emplace(song_list.end(),
                                                 ListMetadata{it, params});
                try {
                    it->second.songs.emplace_back(list_it);
                }
                catch (...) {
                    song_list.erase(list_it);
                    if (inserted)
                        song_map.erase(it);
                    throw;
                }
            }
            catch (...) {
                if (inserted)
                    song_map.erase(it);
                throw;
            }
        }
    };

    std::shared_ptr<playlist_data> data;

    static playlist_data &empty_data() noexcept {
        static playlist_data empty;
        return empty;
    }

    // Unshares data if shared, creating a deep copy.
    void detach() {
        if (!data)
            return;
        if (data.use_count() == 1)
            return;
        data = std::make_shared<playlist_data>(*data);
        data->shareable = true;
    }

  public:
    // Iterators
    struct play_iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = void;
        using reference = const T &;

        play_iterator() = default;

        play_iterator &operator=(const play_iterator &) = default;

        reference operator*() const noexcept { return iter->song_info->first; }

        play_iterator &operator++() noexcept {
            ++iter;
            return *this;
        }
        play_iterator operator++(int) noexcept {
            auto ret = *this;
            ++iter;
            return ret;
        }

        bool operator==(const play_iterator &other) const noexcept {
            return iter == other.iter;
        }
        bool operator!=(const play_iterator &other) const noexcept {
            return iter != other.iter;
        }

      private:
        friend class playlist;

        using iterator_type =
            typename playlist::playlist_data::SongList::iterator;
        iterator_type iter;

        explicit play_iterator(iterator_type it) : iter(it) {}
    };

    play_iterator play_begin() const noexcept {
        auto &d = data ? *data : empty_data();
        return play_iterator(d.song_list.begin());
    }
    play_iterator play_end() const noexcept {
        auto &d = data ? *data : empty_data();
        return play_iterator(d.song_list.end());
    }

    struct sorted_iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = void;
        using reference = const T &;

        sorted_iterator() = default;

        sorted_iterator &operator=(const sorted_iterator &) = default;

        reference operator*() const noexcept { return iter->first; }

        sorted_iterator &operator++() noexcept {
            ++iter;
            return *this;
        }
        sorted_iterator operator++(int) noexcept {
            auto ret = *this;
            ++iter;
            return ret;
        }

        bool operator==(const sorted_iterator &other) const noexcept {
            return iter == other.iter;
        }
        bool operator!=(const sorted_iterator &other) const noexcept {
            return iter != other.iter;
        }

      private:
        friend class playlist;

        using iterator_type =
            typename playlist::playlist_data::SongMap::const_iterator;
        iterator_type iter;

        explicit sorted_iterator(iterator_type it) : iter(it) {}
    };

    sorted_iterator sorted_begin() const noexcept {
        auto &d = data ? *data : empty_data();
        return sorted_iterator(d.song_map.begin());
    }
    sorted_iterator sorted_end() const noexcept {
        auto &d = data ? *data : empty_data();
        return sorted_iterator(d.song_map.end());
    }

    // Playlist constructors and operators

    playlist() noexcept : data(nullptr) {}
    playlist(const playlist &other) {
        if (!other.data) {
            data.reset();
        }
        else if (other.data->shareable) {
            data = other.data;
        }
        else {
            // Must copy if the source is not shareable (locked by params()).
            data = std::make_shared<playlist_data>(*other.data);
        }
    }
    playlist(playlist &&other) noexcept : data(std::move(other.data)) {
        other.data.reset(); // empty playlist represented as nullptr
    }

    ~playlist() = default;

    playlist &operator=(playlist other) {
        std::swap(data, other.data);
        return *this;
    }

    // Playlist methods

    const std::pair<const T &, const P &> front() const {
        if (!data || data->song_list.empty())
            throw std::out_of_range("Cannot call front: playlist is empty");
        return std::pair<const T &, const P &>(
            data->song_list.front().song_info->first,
            data->song_list.front().song_params);
    }

    void push_back(const T &track, const P &params) {
        if (!data) {
            auto tmp = std::make_shared<playlist_data>();
            tmp->push_back(track, params);
            data = std::move(tmp);
            return;
        }

        if (data.use_count() == 1) {
            data->push_back(track, params);
            data->shareable = true;
            return;
        }

        // Modify a copy, then swap to guarantee exception safety.
        auto tmp = std::make_shared<playlist_data>(*data);
        tmp->push_back(track, params);
        data = std::move(tmp);
    }

    void pop_front() {
        if (!data || data->song_list.empty())
            throw std::out_of_range("Cannot call pop_front: playlist is empty");
        detach();

        auto &front = data->song_list.front();
        auto &map_data_iter = front.song_info;

        map_data_iter->second.songs.pop_front();
        if (map_data_iter->second.songs.empty())
            data->song_map.erase(map_data_iter);

        data->song_list.pop_front();
        if (data->song_list.empty())
            data.reset();
        else
            data->shareable = true;
    }

    void remove(const T &track) {
        if (!data || data->song_map.find(track) == data->song_map.end())
            throw std::invalid_argument(
                "Cannot call remove: track not present on playlist");
        detach();

        auto map_it = data->song_map.find(track);
        auto &map_data = map_it->second;
        while (!map_data.songs.empty()) {
            auto &song_list_iter = map_data.songs.front();
            data->song_list.erase(song_list_iter);
            map_data.songs.pop_front();
        }
        data->song_map.erase(track);
        if (data->song_list.empty())
            data.reset();
        else
            data->shareable = true;
    }

    size_t size() const noexcept { return data ? data->song_list.size() : 0; }

    void clear() {
        if (!data)
            return;
        if (data.use_count() > 1) {
            data.reset();
            return;
        }

        data->song_list.clear();
        data->song_map.clear();
        data.reset();
    }

    const std::pair<const T &, const P &>
    play(const play_iterator &it) const noexcept {
        return std::pair<const T &, const P &>(*it, it.iter->song_params);
    }

    const std::pair<const T &, size_t>
    pay(const sorted_iterator &it) const noexcept {
        return std::pair<const T &, size_t>(*it, it.iter->second.songs.size());
    }

    // Undefined behavior on invalid iterator.
    P &params(const play_iterator &it) {
        if (data.use_count() > 1) {
            // Must detach because we are returning a mutable reference.
            // This invalidates the iterator, so we must advance a new one.
            auto dist = std::distance(data->song_list.begin(), it.iter);
            detach();

            auto new_it = data->song_list.begin();
            std::advance(new_it, dist);

            data->shareable = false;
            return new_it->song_params;
        }
        // Mark as non-shareable since we handed out a mutable reference.
        data->shareable = false;
        return it.iter->song_params;
    }

    const P &params(const play_iterator &it) const noexcept {
        return it.iter->song_params;
    }
};
} // namespace cxx

#endif
