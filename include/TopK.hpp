#pragma once
#include <vector>
#include <queue>
#include <algorithm>

namespace indexit {

template <typename T, typename Compare>
class TopK {
private:
    size_t k_;
    std::priority_queue<T, std::vector<T>, Compare> pq_;

public:
    explicit TopK(size_t k) : k_(k) {}

    void add(const T& item) {
        pq_.push(item);
        if (pq_.size() > k_) {
            pq_.pop();
        }
    }

    std::vector<T> get() {
        std::vector<T> result;
        result.reserve(pq_.size());
        while (!pq_.empty()) {
            result.push_back(pq_.top());
            pq_.pop();
        }
        std::reverse(result.begin(), result.end());
        
        // Push them back to keep state if needed? Actually usually TopK is consumed once.
        // Let's just return what we have. If we need to reuse it, we should make a copy.
        // For our search, we just consume it.
        return result;
    }
};

} // namespace indexit
