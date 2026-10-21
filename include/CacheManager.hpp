#pragma once
#include "LruCache.hpp"
#include <string>

namespace indexit {

class CacheManager {
private:
    LruCache<std::string, std::string> cache_;
public:
    CacheManager(size_t capacity = 1000) : cache_(capacity) {}
    
    std::optional<std::string> get(const std::string& key) {
        return cache_.get(key);
    }
    
    void put(const std::string& key, const std::string& value) {
        cache_.put(key, value);
    }
    
    void invalidate(const std::string& key) {
        cache_.remove(key);
    }
};

} // namespace indexit
