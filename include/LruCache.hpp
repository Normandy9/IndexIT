#pragma once
#include <unordered_map>
#include <list>
#include <optional>
#include <mutex>

namespace indexit {

template <typename K, typename V>
class LruCache {
private:
    size_t capacity_;
    std::list<std::pair<K, V>> items_;
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> map_;
    std::mutex mtx_;

public:
    explicit LruCache(size_t capacity) : capacity_(capacity) {}

    std::optional<V> get(const K& key) {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = map_.find(key);
        if (it == map_.end()) {
            return std::nullopt;
        }
        items_.splice(items_.begin(), items_, it->second);
        return it->second->second;
    }

    void put(const K& key, const V& value) {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = map_.find(key);
        if (it != map_.end()) {
            items_.splice(items_.begin(), items_, it->second);
            it->second->second = value;
            return;
        }

        if (items_.size() >= capacity_) {
            auto last = items_.back();
            map_.erase(last.first);
            items_.pop_back();
        }

        items_.emplace_front(key, value);
        map_[key] = items_.begin();
    }
    
    void remove(const K& key) {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = map_.find(key);
        if (it != map_.end()) {
            items_.erase(it->second);
            map_.erase(it);
        }
    }
    
    size_t size() {
        std::lock_guard<std::mutex> lock(mtx_);
        return items_.size();
    }
};

} // namespace indexit
