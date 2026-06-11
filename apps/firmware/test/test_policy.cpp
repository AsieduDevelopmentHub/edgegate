#include <unity.h>
#include "../src/policy/policy_trie.h"

void test_policy_deny_exact_match() {
    PolicyTrie trie;
    trie.addRule("ads.example.com", POLICY_DENY);
    TEST_ASSERT_EQUAL(POLICY_DENY, trie.evaluate("ads.example.com"));
}

void test_policy_allow_default() {
    PolicyTrie trie;
    trie.addRule("ads.example.com", POLICY_DENY);
    TEST_ASSERT_EQUAL(POLICY_ALLOW, trie.evaluate("safe.example.com"));
}

void test_policy_suffix_match() {
    PolicyTrie trie;
    trie.addRule("example.com", POLICY_DENY);
    TEST_ASSERT_EQUAL(POLICY_DENY, trie.evaluate("sub.example.com"));
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_policy_deny_exact_match);
    RUN_TEST(test_policy_allow_default);
    RUN_TEST(test_policy_suffix_match);
    UNITY_END();
}

void loop() {}
