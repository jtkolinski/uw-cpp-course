#ifndef FRUIT_PICKING_H
#define FRUIT_PICKING_H

#include <algorithm>
#include <compare>
#include <list>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

// Enum representing the taste of a fruit
enum class Taste { SWEET, SOUR };

// Enum representing the size of a fruit
enum class Size { LARGE, MEDIUM, SMALL };

// Enum representing the quality (health status) of a fruit
enum class Quality { HEALTHY, ROTTEN, WORMY };

// Stream output operator for Taste - outputs in Polish
inline std::ostream &operator<<(std::ostream &os, const Taste &taste)
{
    switch (taste) {
    case Taste::SWEET:
        os << "słodki";
        break;
    case Taste::SOUR:
        os << "kwaśny";
        break;
    }
    return os;
}

// Stream output operator for Size - outputs in Polish
inline std::ostream &operator<<(std::ostream &os, const Size &size)
{
    switch (size) {
    case Size::LARGE:
        os << "duży";
        break;
    case Size::MEDIUM:
        os << "średni";
        break;
    case Size::SMALL:
        os << "mały";
        break;
    }
    return os;
}

// Stream output operator for Quality - outputs in Polish
inline std::ostream &operator<<(std::ostream &os, const Quality &quality)
{
    switch (quality) {
    case Quality::HEALTHY:
        os << "zdrowy";
        break;
    case Quality::ROTTEN:
        os << "nadgniły";
        break;
    case Quality::WORMY:
        os << "robaczywy";
        break;
    }
    return os;
}

/**
 * Class representing a single fruit with taste, size, and quality attributes.
 * Supports constexpr operations for compile-time usage.
 */
class Fruit
{
  private:
    Taste _taste;
    Size _size;
    Quality _quality;

  public:
    // Constructor: creates a fruit with specified attributes
    explicit constexpr Fruit(Taste taste, Size size, Quality quality)
        : _taste(taste), _size(size), _quality(quality)
    {
    }

    // Default copy and move constructors
    constexpr Fruit(const Fruit &) = default;
    constexpr Fruit(Fruit &&) = default;

    // Explicit constructor from tuple
    explicit constexpr Fruit(const std::tuple<Taste, Size, Quality> &tuple)
        : _taste(std::get<0>(tuple)), _size(std::get<1>(tuple)),
          _quality(std::get<2>(tuple))
    {
    }

    // Explicit conversion to tuple
    explicit constexpr operator std::tuple<Taste, Size, Quality>() const
    {
        return std::make_tuple(_taste, _size, _quality);
    }

    ~Fruit() = default;

    // Default copy and move assignment operators
    constexpr Fruit &operator=(const Fruit &) = default;
    constexpr Fruit &operator=(Fruit &&) = default;

    // Equality comparison: fruits are equal if all attributes match
    constexpr bool operator==(const Fruit &other) const
    {
        return _taste == other._taste && _size == other._size &&
               _quality == other._quality;
    }

    // Makes a healthy fruit become rotten
    void go_rotten()
    {
        if (_quality == Quality::HEALTHY) {
            _quality = Quality::ROTTEN;
        }
    }

    // Makes a healthy fruit become infested with worms
    void become_worm_infested()
    {
        if (_quality == Quality::HEALTHY) {
            _quality = Quality::WORMY;
        }
    }

    // Getters for fruit attributes
    constexpr Taste taste() const
    {
        return _taste;
    }
    constexpr Size size() const
    {
        return _size;
    }
    constexpr Quality quality() const
    {
        return _quality;
    }
};

// Global constexpr constants for common fruit types
constexpr Fruit YUMMY_ONE(Taste::SWEET, Size::LARGE, Quality::HEALTHY);
constexpr Fruit ROTTY_ONE(Taste::SOUR, Size::SMALL, Quality::ROTTEN);

// Stream output operator: outputs fruit in format "[ taste size quality ]"
inline std::ostream &operator<<(std::ostream &os, const Fruit &fruit)
{
    os << "[" << fruit.taste() << " " << fruit.size() << " " << fruit.quality()
       << "]";
    return os;
}

/**
 * Class representing a person who picks fruits.
 * Maintains a collection of picked fruits in order and handles
 * fruit interactions (rotting, worm infestation).
 */
class Picker
{
  private:
    std::string _name;
    std::list<Fruit> _fruits;

  public:
    // Constructor: creates a picker with given name (defaults to "Anonim" if
    // empty)
    Picker(std::string_view name = "Anonim")
        : _name(name.empty() ? "Anonim" : std::move(name)), _fruits()
    {
    }

    // Default copy and move constructors
    Picker(const Picker &) = default;
    Picker(Picker &&) = default;

    // Default copy and move assignment operators
    Picker &operator=(const Picker &) = default;
    Picker &operator=(Picker &&) = default;

    // Returns the picker's name
    const std::string &get_name() const
    {
        return _name;
    }

    /**
     * Adds a fruit to this picker's collection.
     * Handles fruit interactions:
     * - If new fruit is healthy and last fruit is rotten → new fruit rots
     * - If new fruit is rotten and last fruit is healthy → last fruit rots
     * - If new fruit is wormy → all healthy sweet fruits become wormy
     */
    Picker &operator+=(Fruit new_fruit)
    {
        if (!_fruits.empty()) {
            Fruit &last = _fruits.back();

            // Healthy fruit becomes rotten if placed after rotten fruit
            if (new_fruit.quality() == Quality::HEALTHY &&
                last.quality() == Quality::ROTTEN) {
                new_fruit.go_rotten();
            }
            // Last fruit becomes rotten if new fruit is rotten
            else if (new_fruit.quality() == Quality::ROTTEN &&
                     last.quality() == Quality::HEALTHY) {
                last.go_rotten();
            }
        }

        // Wormy fruit infects all healthy sweet fruits
        if (new_fruit.quality() == Quality::WORMY) {
            for (auto &f : _fruits) {
                if (f.quality() == Quality::HEALTHY &&
                    f.taste() == Taste::SWEET) {
                    f.become_worm_infested();
                }
            }
        }

        _fruits.push_back(new_fruit);
        return *this;
    }

    /**
     * Steals the first fruit from another picker.
     * Does nothing if stealing from self or if other picker has no fruits.
     * Note: Other picker cannot be const because we modify it by stealing.
     */
    Picker &operator+=(Picker &other)
    {
        // Cannot steal from yourself
        if (this == &other)
            return *this;

        if (!other._fruits.empty()) {
            Fruit stolen = std::move(other._fruits.front());
            other._fruits.pop_front();
            *this += std::move(stolen);
        }
        return *this;
    }

    /**
     * Gives the first fruit to another picker.
     * Does nothing if giving to self or if this picker has no fruits.
     */
    Picker &operator-=(Picker &other)
    {
        // Cannot give to yourself
        if (this == &other)
            return *this;

        if (!_fruits.empty()) {
            Fruit given = std::move(_fruits.front());
            _fruits.pop_front();
            other += std::move(given);
        }
        return *this;
    }

    // Returns total number of fruits picked
    size_t count_fruits() const
    {
        return _fruits.size();
    }

    // Returns number of fruits with given taste
    size_t count_taste(Taste taste) const
    {
        return std::count_if(_fruits.begin(),
            _fruits.end(),
            [taste](const Fruit &f) { return f.taste() == taste; });
    }

    // Returns number of fruits with given size
    size_t count_size(Size size) const
    {
        return std::count_if(_fruits.begin(),
            _fruits.end(),
            [size](const Fruit &f) { return f.size() == size; });
    }

    // Returns number of fruits with given quality
    size_t count_quality(Quality quality) const
    {
        return std::count_if(_fruits.begin(),
            _fruits.end(),
            [quality](const Fruit &f) { return f.quality() == quality; });
    }

    /**
     * Three-way comparison operator for ranking pickers.
     * Priority order:
     * 1. Number of healthy fruits (more is better)
     * 2. Number of sweet fruits (more is better)
     * 3. Number of large fruits (more is better)
     * 4. Number of medium fruits (more is better)
     * 5. Number of small fruits (more is better)
     * 6. Total number of fruits (more is better)
     */
    auto operator<=>(const Picker &other) const
    {
        // 1. Number of healthy fruits
        auto healthy_cmp = other.count_quality(Quality::HEALTHY) <=>
                           count_quality(Quality::HEALTHY);
        if (healthy_cmp != 0)
            return healthy_cmp;

        // 2. Number of sweet fruits
        auto sweet_cmp =
            other.count_taste(Taste::SWEET) <=> count_taste(Taste::SWEET);
        if (sweet_cmp != 0)
            return sweet_cmp;

        // 3. Number of large fruits
        auto large_cmp =
            other.count_size(Size::LARGE) <=> count_size(Size::LARGE);
        if (large_cmp != 0)
            return large_cmp;

        // 4. Number of medium fruits
        auto medium_cmp =
            other.count_size(Size::MEDIUM) <=> count_size(Size::MEDIUM);
        if (medium_cmp != 0)
            return medium_cmp;

        // 5. Number of small fruits
        auto small_cmp =
            other.count_size(Size::SMALL) <=> count_size(Size::SMALL);
        if (small_cmp != 0)
            return small_cmp;

        // 6. Total number of fruits
        return other.count_fruits() <=> count_fruits();
    }

    /**
     * Equality operator: pickers are equal if they have the same name
     * and the same fruits in the same order.
     */
    bool operator==(const Picker &other) const
    {
        if (_name != other._name)
            return false;
        if (_fruits.size() != other._fruits.size())
            return false;

        auto it1 = _fruits.begin();
        auto it2 = other._fruits.begin();

        while (it1 != _fruits.end()) {
            if (!(*it1 == *it2))
                return false;
            ++it1;
            ++it2;
        }

        return true;
    }

    friend std::ostream &operator<<(std::ostream &os, const Picker &picker);
};

// Stream output operator: outputs picker's name and all their fruits
inline std::ostream &operator<<(std::ostream &os, const Picker &picker)
{
    os << picker._name << ":";
    for (const auto &fruit : picker._fruits) {
        os << "\n" << "\t" << fruit;
    }
    return os;
}

/**
 * Class representing a ranking of fruit pickers.
 * Maintains pickers in sorted order (best to worst).
 * When pickers are tied, maintains stable ordering (earlier insertion stays
 * earlier).
 */
class Ranking
{
  private:
    std::vector<Picker> _ranking;

  public:
    // Default constructor: creates empty ranking
    Ranking() : _ranking() {}

    /**
     * Constructor from initializer list: creates ranking and sorts it.
     * Pickers are sorted from best to worst.
     */
    Ranking(std::initializer_list<Picker> list) : _ranking(list)
    {
        std::stable_sort(_ranking.begin(),
            _ranking.end(),
            [](const Picker &a, const Picker &b) { return a < b; });
    }

    // Default copy and move constructors
    Ranking(const Ranking &) = default;
    Ranking(Ranking &&) = default;

    // Default copy and move assignment operators
    Ranking &operator=(const Ranking &) = default;
    Ranking &operator=(Ranking &&) = default;

    /**
     * Adds a picker to the ranking in the correct position.
     * In case of a tie, the new picker is placed after existing pickers with
     * the same score (stable insertion).
     */
    Ranking &operator+=(const Picker &picker)
    {
        auto it = _ranking.begin();
        // Find position: skip all pickers that are better than or equal to new
        // picker
        while (it != _ranking.end() && !(*it > picker))
            ++it;
        _ranking.insert(it, picker);
        return *this;
    }

    // Move version of operator+=
    Ranking &operator+=(Picker &&picker)
    {
        auto it = _ranking.begin();
        while (it != _ranking.end() && !(*it > picker))
            ++it;
        _ranking.insert(it, std::move(picker));
        return *this;
    }

    /**
     * Merges another ranking into this one.
     * Maintains stable ordering: when pickers are tied, those already in this
     * ranking appear before those from the other ranking.
     */
    Ranking &operator+=(const Ranking &other)
    {
        std::vector<Picker> result;
        result.reserve(_ranking.size() + other._ranking.size());
        auto it1 = _ranking.begin();
        auto it2 = other._ranking.begin();

        // Merge two sorted sequences maintaining stable order
        while (it1 != _ranking.end() && it2 != other._ranking.end()) {
            // If picker from this ranking is better, take it
            if (*it1 < *it2) {
                result.push_back(*it1);
                ++it1;
            }
            // If picker from other ranking is better, take it
            else if (*it2 < *it1) {
                result.push_back(*it2);
                ++it2;
            }
            // Tie: take from this ranking first (stable sort - earlier stays
            // earlier)
            else {
                result.push_back(*it1);
                ++it1;
            }
        }

        // Append remaining pickers
        std::move(it1, _ranking.end(), std::back_inserter(result));
        result.insert(result.end(), it2, other._ranking.end());
        _ranking = std::move(result);
        return *this;
    }

    // Move version of operator+=
    Ranking &operator+=(Ranking &&other)
    {
        if (this == &other)
            return *this;

        std::vector<Picker> result;
        result.reserve(_ranking.size() + other._ranking.size());
        auto it1 = _ranking.begin();
        auto it2 = other._ranking.begin();

        while (it1 != _ranking.end() && it2 != other._ranking.end()) {
            if (*it1 < *it2) {
                result.push_back(std::move(*it1));
                ++it1;
            }
            else if (*it2 < *it1) {
                result.push_back(std::move(*it2));
                ++it2;
            }
            else {
                // Tie: prefer from this ranking (stable sort)
                result.push_back(std::move(*it1));
                ++it1;
            }
        }

        std::move(it1, _ranking.end(), std::back_inserter(result));
        std::move(it2, other._ranking.end(), std::back_inserter(result));
        _ranking = std::move(result);
        return *this;
    }

    // Creates a new ranking by merging two rankings (const version)
    Ranking operator+(const Ranking &other) const
    {
        Ranking result = *this;
        result += other;
        return result;
    }

    // Creates a new ranking by merging two rankings (move version)
    Ranking operator+(Ranking &&other) const
    {
        Ranking result = *this;
        result += std::move(other);
        return result;
    }

    /**
     * Removes the first (highest-ranked) picker that is equal to the given
     * picker. Equality is determined by Picker::operator==.
     */
    void operator-=(const Picker &picker)
    {
        for (auto it = _ranking.begin(); it != _ranking.end(); ++it) {
            if (*it == picker) {
                _ranking.erase(it);
                return;
            }
        }
    }

    /**
     * Returns the picker at the given position (0-indexed).
     * If index is out of bounds, returns the last picker.
     * Throws a Runtime Error for empty rankings.
     */
    const Picker &operator[](size_t idx) const
    {
        if (!_ranking.size())
            throw std::runtime_error("Ranking is empty.");

        return _ranking[std::min(idx, _ranking.size() - 1)];
    }

    // Returns the total number of pickers in the ranking
    size_t count_pickers() const
    {
        return _ranking.size();
    }
};

// Stream output operator: outputs all pickers in ranking order
inline std::ostream &operator<<(std::ostream &os, const Ranking &ranking)
{
    for (size_t i = 0; i < ranking.count_pickers(); ++i) {
        if (i > 0)
            os << "\n";
        os << ranking[i];
    }
    // Non-empty ranking ends with newline
    if (ranking.count_pickers() > 0) {
        os << "\n";
    }
    return os;
}

#endif