#pragma once
#include "LruCache.hpp"
#include <string>
#include <unordered_map>
#include <list>
#include <cstddef>

namespace indexit {

class CacheManager {
private:
    LruCache<std::string, std::string> cache_;
    std::unordered_map<std::string, size_t> itemBytes_;
    std::list<std::string> order_;
    size_t maxBytes_;
    size_t currentBytes_;

public:
    explicit CacheManager(size_t maxItems = 1000, size_t maxBytes = 32 * 1024 * 1024)
        : cache_(maxItems), maxBytes_(maxBytes), currentBytes_(0) {}

    bool get(const std::string& key, std::string& out_value) {
        bool found = cache_.get(key, out_value);
        if (found) {
            // Update order for LRU
            for (auto it = order_.begin(); it != order_.end(); ++it) {
                if (*it == key) {
                    order_.splice(order_.begin(), order_, it);
                    break;
                }
            }
        }
        return found;
    }

    void put(const std::string& key, const std::string& value) {
        size_t entryBytes = key.size() + value.size();
        if (entryBytes > maxBytes_) return; // Skip if single item exceeds total cap

        // Invalidate previous entry if exists
        auto it = itemBytes_.find(key);
        if (it != itemBytes_.end()) {
            currentBytes_ -= it->second;
            itemBytes_.erase(it);
            for (auto oit = order_.begin(); oit != order_.end(); ++oit) {
                if (*oit == key) {
                    order_.erase(oit);
                    break;
                }
            }
        }

        // Evict LRU items while over byte cap
        while (currentBytes_ + entryBytes > maxBytes_ && !order_.empty()) {
            std::string oldest = order_.back();
            order_.pop_back();
            auto szIt = itemBytes_.find(oldest);
            if (szIt != itemBytes_.end()) {
                currentBytes_ -= szIt->second;
                itemBytes_.erase(szIt);
            }
            cache_.remove(oldest);
        }

        cache_.put(key, value);
        order_.push_front(key);
        itemBytes_[key] = entryBytes;
        currentBytes_ += entryBytes;
    }

    void invalidate(const std::string& key) {
        auto it = itemBytes_.find(key);
        if (it != itemBytes_.end()) {
            currentBytes_ -= it->second;
            itemBytes_.erase(it);
            for (auto oit = order_.begin(); oit != order_.end(); ++oit) {
                if (*oit == key) {
                    order_.erase(oit);
                    break;
                }
            }
        }
        cache_.remove(key);
    }

    bool contains(const std::string& key) const {
        return itemBytes_.find(key) != itemBytes_.end();
    }

    void clear() {
        cache_.clear();
        order_.clear();
        itemBytes_.clear();
        currentBytes_ = 0;
    }

    size_t size() const {
        return cache_.size();
    }

    size_t currentBytes() const {
        return currentBytes_;
    }

    size_t maxBytes() const {
        return maxBytes_;
    }
};

} // namespace indexit
