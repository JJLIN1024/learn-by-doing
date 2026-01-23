#include <iostream>
#include <cstdint>
#include <cassert>
#include <array>
#include <vector>
#include <bit> // C++20
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <compare> // C++20

using namespace std;

// --- 常數定義 ---
constexpr size_t MAX_POOL_SIZE = 20000000;
constexpr size_t MAX_PRICE_LEVELS = 10000000;
constexpr int64_t PRICE_SCALE = 10000; 
constexpr uint32_t NULL_IDX = std::numeric_limits<uint32_t>::max();

enum class Side : uint8_t { BUY = 0, SELL = 1 };

// --- 1. Price Class (保留測試相容性) ---
class Price {
    int64_t raw_value;
    static constexpr int64_t POW10[] = {1, 10, 100, 1000, 10000};
public:
    constexpr Price() : raw_value(0) {}
    constexpr explicit Price(int64_t v) : raw_value(v) {}
    
    // C++20 飛碟運算子：自動生成 <, >, <=, >=, ==, !=
    auto operator<=>(const Price&) const = default;

    [[nodiscard]] constexpr int64_t toInternal() const { return raw_value; }
    [[nodiscard]] constexpr int toIdx() const { return static_cast<int>(raw_value); }

    // 運算子重載
    constexpr Price operator+(const Price& other) const { return Price(raw_value + other.raw_value); }
    constexpr Price operator-(const Price& other) const { return Price(raw_value - other.raw_value); }

    // 字串解析 (保留給測試用)
    static constexpr Price fromString(const char* s) {
        int64_t int_part = 0, frac_part = 0;
        bool negative = (*s == '-');
        if (negative) s++;
        
        while (*s >= '0' && *s <= '9') {
            int_part = int_part * 10 + (*s - '0');
            s++;
        }
        
        int digits = 0;
        if (*s == '.') {
            s++;
            while (*s >= '0' && *s <= '9') {
                if (digits < 4) {
                    frac_part = frac_part * 10 + (*s - '0');
                    digits++;
                }
                s++;
            }
        }
        frac_part *= POW10[4 - digits];
        int64_t total = int_part * PRICE_SCALE + frac_part;
        return Price(negative ? -total : total);
    }
};

// --- 2. 記憶體管理 (Huge Pages) ---
struct HugeMem {
    void* addr;
    size_t size;
    HugeMem(size_t size) : size(size) {
        // 優先嘗試 Huge Pages
        addr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
        if (addr == MAP_FAILED) {
            // 退回一般 Pages
            addr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (addr == MAP_FAILED) throw std::runtime_error("mmap failed");
        }
        if (mlock(addr, size) != 0) { /* warning suppressed */ }
        std::memset(addr, 0, size); // Fault-in
    }
    ~HugeMem() { munlock(addr, size); munmap(addr, size); }
};

// --- 3. 核心資料結構 ---
struct alignas(32) OrderNode {
    uint64_t id;
    int64_t qty;
    int32_t price;
    uint32_t next;
    uint32_t prev;
};

struct OrderListHead {
    uint32_t head = NULL_IDX;
    uint32_t tail = NULL_IDX;
};

// C++20 BitMask
template<size_t SIZE>
class BitMask {
    static constexpr size_t BITS = 64;
    static constexpr size_t WORDS = (SIZE + BITS - 1) / BITS;
    static constexpr size_t SUMMARIES = (WORDS + BITS - 1) / BITS;
    std::array<uint64_t, WORDS> words{};
    std::array<uint64_t, SUMMARIES> summary{};
    
    static constexpr uint64_t mask_ge(size_t idx) { return ~0ULL << (idx & 63); }
    static constexpr uint64_t mask_le(size_t idx) { return ~0ULL >> (63 - (idx & 63)); }
public:
    void set(size_t idx) {
        if (idx >= SIZE) return;
        const size_t w = idx / BITS;
        if (!(words[w] & (1ULL << (idx % BITS)))) {
            words[w] |= (1ULL << (idx % BITS));
            summary[w / BITS] |= (1ULL << (w % BITS));
        }
    }
    void unset(size_t idx) {
        if (idx >= SIZE) return;
        const size_t w = idx / BITS;
        words[w] &= ~(1ULL << (idx % BITS));
        if (words[w] == 0) summary[w / BITS] &= ~(1ULL << (w % BITS));
    }
    [[nodiscard]] int next_set_bit(int start_idx) const {
        if (start_idx < 0) start_idx = 0;
        size_t idx = static_cast<size_t>(start_idx);
        if (idx >= SIZE) return -1;
        
        size_t w = idx / BITS;
        size_t b = idx % BITS;
        uint64_t word = words[w] & mask_ge(b);
        if (word) return (w * BITS) + std::countr_zero(word);

        size_t s_idx = w / BITS;
        size_t s_bit = w % BITS;
        uint64_t sum_word = (s_bit < 63) ? (summary[s_idx] & mask_ge(s_bit + 1)) : 0;
        
        while (sum_word == 0) {
            if (++s_idx >= SUMMARIES) return -1;
            sum_word = summary[s_idx];
        }
        size_t next_w = (s_idx * BITS) + std::countr_zero(sum_word);
        return (next_w * BITS) + std::countr_zero(words[next_w]);
    }
    [[nodiscard]] int prev_set_bit(int start_idx) const {
        if (start_idx < 0) return -1;
        size_t idx = (start_idx >= (int)SIZE) ? SIZE - 1 : static_cast<size_t>(start_idx);
        
        size_t w = idx / BITS;
        size_t b = idx % BITS;
        uint64_t word = words[w] & mask_le(b);
        if (word) return (w * BITS) + (63 - std::countl_zero(word)); // GCC builtin for bit reverse

        size_t s_idx = w / BITS;
        size_t s_bit = w % BITS;
        uint64_t sum_word = (s_bit > 0) ? (summary[s_idx] & mask_le(s_bit - 1)) : 0;
        
        while (sum_word == 0) {
            if (s_idx-- == 0) return -1;
            sum_word = summary[s_idx];
        }
        size_t prev_w = (s_idx * BITS) + (63 - std::countl_zero(sum_word));
        return (prev_w * BITS) + (63 - std::countl_zero(words[prev_w]));
    }
};

// Fenwick Tree (整合版) - 用於 queryRange 測試
template<size_t SIZE>
struct FlatFenwick {
    int64_t tree[SIZE + 1]; // 直接嵌入 DataLayout
    void update(int idx, int64_t delta) {
        for (idx++; idx < (int)SIZE + 1; idx += idx & -idx) tree[idx] += delta;
    }
    int64_t query(int idx) const {
        int64_t sum = 0;
        for (idx++; idx > 0; idx -= idx & -idx) sum += tree[idx];
        return sum;
    }
    int64_t queryRange(int low, int high) const {
        if (low > high) return 0;
        return query(high) - query(low - 1);
    }
};

// --- 4. HFT OrderBook ---
class OrderBook {
    struct DataLayout {
        OrderNode nodes[MAX_POOL_SIZE];
        uint32_t free_head; 
        
        OrderListHead bids[MAX_PRICE_LEVELS];
        OrderListHead asks[MAX_PRICE_LEVELS];
        
        BitMask<MAX_PRICE_LEVELS> bid_mask;
        BitMask<MAX_PRICE_LEVELS> ask_mask;
        
        // 統計用 (為了讓測試通過)
        FlatFenwick<MAX_PRICE_LEVELS> bid_tree;
        FlatFenwick<MAX_PRICE_LEVELS> ask_tree;
    };

    HugeMem memory;
    DataLayout* d;

public:
    OrderBook() : memory(sizeof(DataLayout)) {
        d = new (memory.addr) DataLayout();
        // 初始化 Free List
        for (uint32_t i = 0; i < MAX_POOL_SIZE - 1; ++i) d->nodes[i].next = i + 1;
        d->nodes[MAX_POOL_SIZE - 1].next = NULL_IDX;
        d->free_head = 0;
    }

    // --- API: Add Order ---
    int64_t addOrder(uint64_t id, Side side, int64_t raw_price, int64_t raw_qty) {
        if (raw_price < 0 || raw_price >= (int64_t)MAX_PRICE_LEVELS) return raw_qty; // 簡單邊界檢查
        
        int p_idx = static_cast<int>(raw_price);
        int64_t remaining = raw_qty;

        if (side == Side::BUY) {
            // 1. Match against Asks
            while (remaining > 0) {
                int best_ask = d->ask_mask.next_set_bit(0);
                if (best_ask == -1 || best_ask > p_idx) break;

                remaining = match_level(d->asks[best_ask], d->ask_mask, d->ask_tree, best_ask, remaining);
            }
            // 2. Post remaining
            if (remaining > 0) {
                post_order(d->bids[p_idx], d->bid_mask, d->bid_tree, id, p_idx, remaining);
            }
        } else {
            // 1. Match against Bids
            while (remaining > 0) {
                int best_bid = d->bid_mask.prev_set_bit(MAX_PRICE_LEVELS - 1);
                if (best_bid == -1 || best_bid < p_idx) break;

                remaining = match_level(d->bids[best_bid], d->bid_mask, d->bid_tree, best_bid, remaining);
            }
            // 2. Post remaining
            if (remaining > 0) {
                post_order(d->asks[p_idx], d->ask_mask, d->ask_tree, id, p_idx, remaining);
            }
        }
        return remaining;
    }

    // --- API: Query Range (為了測試) ---
    int64_t queryRange(int low, int high, Side side) {
        if (low < 0) low = 0;
        if (high >= (int)MAX_PRICE_LEVELS) high = MAX_PRICE_LEVELS - 1;
        
        if (side == Side::BUY) return d->bid_tree.queryRange(low, high);
        else return d->ask_tree.queryRange(low, high);
    }

private:
    // 通用撮合邏輯
    int64_t match_level(OrderListHead& list, BitMask<MAX_PRICE_LEVELS>& mask, 
                        FlatFenwick<MAX_PRICE_LEVELS>& tree, int price_idx, int64_t qty) {
        uint32_t curr = list.head;
        while (curr != NULL_IDX && qty > 0) {
            OrderNode& node = d->nodes[curr];
            int64_t trade = std::min(qty, node.qty);
            
            node.qty -= trade;
            qty -= trade;
            tree.update(price_idx, -trade); // 更新統計

            uint32_t next_node = node.next;
            if (node.qty == 0) {
                remove_node(list, curr);
                free_node(curr);
                if (list.head == NULL_IDX) mask.unset(price_idx);
            }
            curr = next_node;
        }
        return qty;
    }

    // 通用掛單邏輯
    void post_order(OrderListHead& list, BitMask<MAX_PRICE_LEVELS>& mask, 
                    FlatFenwick<MAX_PRICE_LEVELS>& tree, uint64_t id, int price_idx, int64_t qty) {
        uint32_t idx = alloc_node();
        if (idx == NULL_IDX) return; // Pool full
        
        OrderNode& node = d->nodes[idx];
        node.id = id;
        node.price = price_idx;
        node.qty = qty;
        
        push_back(list, idx);
        mask.set(price_idx);
        tree.update(price_idx, qty); // 更新統計
    }

    // --- Helpers ---
    uint32_t alloc_node() {
        uint32_t idx = d->free_head;
        if (idx != NULL_IDX) {
            d->free_head = d->nodes[idx].next;
            d->nodes[idx].next = d->nodes[idx].prev = NULL_IDX;
        }
        return idx;
    }
    void free_node(uint32_t idx) {
        d->nodes[idx].next = d->free_head;
        d->free_head = idx;
    }
    void push_back(OrderListHead& list, uint32_t idx) {
        OrderNode& node = d->nodes[idx];
        node.next = NULL_IDX;
        node.prev = list.tail;
        if (list.tail != NULL_IDX) d->nodes[list.tail].next = idx;
        else list.head = idx;
        list.tail = idx;
    }
    void remove_node(OrderListHead& list, uint32_t idx) {
        OrderNode& node = d->nodes[idx];
        if (node.prev != NULL_IDX) d->nodes[node.prev].next = node.next;
        else list.head = node.next;
        if (node.next != NULL_IDX) d->nodes[node.next].prev = node.prev;
        else list.tail = node.prev;
    }
};

// --- Tests ---
constexpr int MAX_PRICE = MAX_PRICE_LEVELS; // 相容舊測試變數

void run_comprehensive_tests() {
    OrderBook ob;

    std::cout << "Testing Case 1: Boundary Prices..." << std::endl;
    ob.addOrder(1, Side::BUY, 0, 100);
    ob.addOrder(2, Side::SELL, MAX_PRICE - 1, 100);
    assert(ob.queryRange(0, 0, Side::BUY) == 100);
    assert(ob.queryRange(MAX_PRICE - 1, MAX_PRICE - 1, Side::SELL) == 100);

    std::cout << "Testing Case 2: BitMask Cross-word jump..." << std::endl;
    ob.addOrder(3, Side::SELL, 63, 10);
    ob.addOrder(4, Side::SELL, 64, 10);
    int rem_cross = ob.addOrder(5, Side::BUY, 65, 25);
    assert(rem_cross == 5); 
    assert(ob.queryRange(63, 64, Side::SELL) == 0);
    assert(ob.queryRange(65, 65, Side::BUY) == 5); // 這裡會掛單

    std::cout << "Testing Case 3: Perfect Fill..." << std::endl;
    ob.addOrder(6, Side::SELL, 200, 50);
    int rem_perfect = ob.addOrder(7, Side::BUY, 200, 50);
    assert(rem_perfect == 0);
    assert(ob.queryRange(200, 200, Side::SELL) == 0);

    std::cout << "Testing Case 4: Aggressive Matching..." << std::endl;
    ob.addOrder(8, Side::SELL, 110, 10);
    ob.addOrder(9, Side::SELL, 120, 10);
    int rem_agg = ob.addOrder(10, Side::BUY, 500, 20); 
    assert(rem_agg == 0);
    assert(ob.queryRange(110, 120, Side::SELL) == 0);

    std::cout << "Testing Case 5: Complex Range Query..." << std::endl;
    ob.addOrder(11, Side::BUY, 2000, 10);
    ob.addOrder(12, Side::BUY, 3000, 20);
    ob.addOrder(13, Side::BUY, 4000, 30);
    assert(ob.queryRange(2000, 3000, Side::BUY) == 30);
    assert(ob.queryRange(2000, 4000, Side::BUY) == 60);

    std::cout << "Testing Case 6: Memory Pool Reallocation..." << std::endl;
    for(int i=0; i<1000; ++i) {
        ob.addOrder(10000+i, Side::SELL, 8000, 1);
        ob.addOrder(20000+i, Side::BUY, 8000, 1);
    }
    assert(ob.queryRange(8000, 8000, Side::SELL) == 0);

    std::cout << "Testing Case 7: Search with Price Gaps..." << std::endl;
    ob.addOrder(30, Side::SELL, 9000, 100);
    ob.addOrder(31, Side::SELL, 9500, 100);
    int rem_gap = ob.addOrder(32, Side::BUY, 9200, 150);
    assert(rem_gap == 50); 
    assert(ob.queryRange(9000, 9000, Side::SELL) == 0);
    assert(ob.queryRange(9500, 9500, Side::SELL) == 100);

    std::cout << "\n>>> ALL LOGIC TESTS PASSED! <<<" << std::endl;
}

void test_price_precision() {
    std::cout << "Running: Price Tests..." << std::endl;
    Price p1 = Price::fromString("0.1");
    Price p2 = Price::fromString("0.2");
    Price p3 = Price::fromString("0.3");
    
    // operator<=> handles ==
    assert(p1 + p2 == p3);
    assert((p1 + p2).toInternal() == 3000); 

    assert(Price::fromString("1").toInternal() == 10000);
    assert(Price::fromString("1.2345").toInternal() == 12345);
    
    Price a = Price::fromString("10.50");
    Price b = Price::fromString("20.25");
    assert(a < b);
    assert(b > a);
    assert((b - a) == Price::fromString("9.75"));
    std::cout << ">>> PRICE TESTS PASSED! <<<" << std::endl;
}

int main() {
    run_comprehensive_tests();
    test_price_precision();
    return 0;
}