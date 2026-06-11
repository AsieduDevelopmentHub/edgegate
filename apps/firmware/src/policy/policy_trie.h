#pragma once

#include <stdint.h>
#include <string.h>

enum PolicyAction : uint8_t {
    POLICY_ALLOW = 0,
    POLICY_DENY = 1,
    POLICY_REDIRECT = 2,
    POLICY_THROTTLE = 3,
    POLICY_NONE = 255,
};

struct PolicyRule {
    char pattern[64];
    PolicyAction action;
    bool active;
};

class PolicyTrie {
public:
    static constexpr size_t MAX_RULES = 64;

    PolicyTrie() : rule_count_(0) {
        memset(rules_, 0, sizeof(rules_));
    }

    void clear() {
        rule_count_ = 0;
        memset(rules_, 0, sizeof(rules_));
    }

    bool addRule(const char* pattern, PolicyAction action) {
        if (rule_count_ >= MAX_RULES) return false;
        PolicyRule* r = &rules_[rule_count_++];
        strncpy(r->pattern, pattern, 63);
        r->pattern[63] = '\0';
        r->action = action;
        r->active = true;
        return true;
    }

    PolicyAction evaluate(const char* domain) const {
        PolicyAction result = POLICY_ALLOW;
        size_t best_len = 0;

        for (size_t i = 0; i < rule_count_; i++) {
            if (!rules_[i].active) continue;
            const char* pat = rules_[i].pattern;
            size_t pat_len = strlen(pat);
            if (pat_len == 0) continue;

            bool match = false;
            if (pat[0] == '*') {
                match = strstr(domain, pat + 1) != nullptr;
            } else if (pat_len <= strlen(domain)) {
                const char* tail = domain + strlen(domain) - pat_len;
                match = (strcasecmp(tail, pat) == 0) || (strcasecmp(domain, pat) == 0);
            }

            if (match && pat_len >= best_len) {
                best_len = pat_len;
                result = rules_[i].action;
            }
        }
        return result;
    }

    size_t ruleCount() const { return rule_count_; }

private:
    PolicyRule rules_[MAX_RULES];
    size_t rule_count_;
};
