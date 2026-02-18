#pragma once
#include <vector>
#include <algorithm>
#include <utility>
#include <stdexcept>
#include <cstddef>

template <typename K, typename V>
class SortedVectorMap {
    std::vector<std::pair<K, V>> data_;

public:
    bool empty() const noexcept { return data_.empty(); }
    std::size_t size() const noexcept { return data_.size(); }

    bool contains(const K& key) const {
        return find(key) != data_.cend();
    }

    V& at(const K& key) {
        auto it = find(key);
        if (it == data_.end())
            throw std::out_of_range("key not found");
        return it->second;
    }

    const V& at(const K& key) const {
        auto it = find(key);
        if (it == data_.cend())
            throw std::out_of_range("key not found");
        return it->second;
    }

    V& operator[](const K& key) {
        auto it = lower(key);
        if (it == data_.end() || it->first != key)
            it = data_.insert(it, std::make_pair(key, V{}));
        return it->second;
    }

    bool insert(const K& key, const V& value) {
        auto it = lower(key);
        if (it != data_.end() && it->first == key)
            return false;
        data_.insert(it, std::make_pair(key, value));
        return true;
    }

    bool erase(const K& key) {
        auto it = find(key);
        if (it == data_.end())
            return false;
        data_.erase(it);
        return true;
    }

    auto begin() noexcept { return data_.begin(); }
    auto end() noexcept { return data_.end(); }
    auto begin() const noexcept { return data_.cbegin(); }
    auto end() const noexcept { return data_.cend(); }

private:
    auto lower(const K& key) {
        return std::lower_bound(
            data_.begin(), data_.end(), key,
            [](const std::pair<K, V>& p, const K& k) { return p.first < k; }
        );
    }

    auto lower(const K& key) const {
        return std::lower_bound(
            data_.cbegin(), data_.cend(), key,
            [](const std::pair<K, V>& p, const K& k) { return p.first < k; }
        );
    }

    auto find(const K& key) {
        auto it = lower(key);
        if (it != data_.end() && it->first == key) return it;
        return data_.end();
    }

    auto find(const K& key) const {
        auto it = lower(key);
        if (it != data_.cend() && it->first == key) return it;
        return data_.cend();
    }
};
