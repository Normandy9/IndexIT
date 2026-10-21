#pragma once
#include <unordered_map>
#include <list>

namespace indexit {

template <typename K, typename V>
class LruCache {
private:
    size_t capacity_;
    std::list<std::pair<K, V>> items_;
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> map_;

public:
    explicit LruCache(size_t capacity) : capacity_(capacity) {}

    bool get(const K& key, V& out_value) {
        auto it = map_.find(key);
        if (it == map_.end()) {
            return false;
        }
        items_.splice(items_.begin(), items_, it->second);
        out_value = it->second->second;
        return true;
    }

    void put(const K& key, const V& value) {
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
        auto it = map_.find(key);
        if (it != map_.end()) {
            items_.erase(it->second);
            map_.erase(it);
        }
    }
    
    size_t size() {
        return items_.size();
    }
};

} // namespace indexit
