#pragma once

#include <stdint.h>
#include <string.h>

struct DNSCacheEntry {
    char domain[64];
    char ip[16];
    uint32_t ttl;
    uint64_t expires_at;
    bool valid;
};

class DNSCache {
public:
    static constexpr size_t MAX = 512;

    DNSCache() : count_(0), head_(0) {
        memset(entries_, 0, sizeof(entries_));
    }

    DNSCacheEntry* lookup(const char* domain) {
        for (size_t i = 0; i < count_; i++) {
            if (entries_[i].valid && strncmp(entries_[i].domain, domain, 64) == 0) {
                return &entries_[i];
            }
        }
        return nullptr;
    }

    void insert(const char* domain, const char* ip, uint32_t ttl, uint64_t now_ms) {
        if (count_ < MAX) {
            DNSCacheEntry* e = &entries_[count_++];
            strncpy(e->domain, domain, 63);
            strncpy(e->ip, ip, 15);
            e->ttl = ttl;
            e->expires_at = now_ms + (ttl * 1000ULL);
            e->valid = true;
        } else {
            DNSCacheEntry* e = &entries_[head_];
            head_ = (head_ + 1) % MAX;
            strncpy(e->domain, domain, 63);
            strncpy(e->ip, ip, 15);
            e->ttl = ttl;
            e->expires_at = now_ms + (ttl * 1000ULL);
            e->valid = true;
        }
    }

    void evictExpired(uint64_t now_ms) {
        for (size_t i = 0; i < count_; i++) {
            if (entries_[i].valid && entries_[i].expires_at < now_ms) {
                entries_[i].valid = false;
            }
        }
    }

private:
    DNSCacheEntry entries_[MAX];
    size_t count_;
    size_t head_;
};
