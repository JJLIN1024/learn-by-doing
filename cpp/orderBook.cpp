 #include <sys/mman.h>

#include <iostream>
#include <cstdint>
#include <cassert>
#include <array>
#include <cmath>
#include <atomic>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <compare> 
#include <stdexcept>

constexpr int MAX_POOL_SIZE = 20000000;
constexpr int MAX_PRICE = 10000000;

using namespace std;

// template<typename T, size_t Size>
// class SPSCRingBuffer {
// 	static_assert((Size & (Size - 1)) == 0, "Size must be power of 2");

// private:
// 	alignas(64) std::atomic<size_t> head{0}; // 寫入位置
// 	alignas(64) std::atomic<size_t> tail{0}; // 讀取位置
// 	T buffer[Size];

// public:
// 	// 寫入者（Producer）調用
// 	bool enqueue(const T& data) {
// 		const size_t h = head.load(std::memory_order_relaxed);
// 		const size_t t = tail.load(std::memory_order_acquire);
		
// 		if (((h + 1) & (Size - 1)) == t) {
// 			return false; // 滿了
// 		}
		
// 		buffer[h] = data;
// 		head.store((h + 1) & (Size - 1), std::memory_order_release);
// 		return true;
// 	}

// 	// 讀取者（Consumer）調用
// 	bool dequeue(T& result) {
// 		const size_t t = tail.load(std::memory_order_relaxed);
// 		const size_t h = head.load(std::memory_order_acquire);
		
// 		if (h == t) {
// 			return false; // 空的
// 		}
		
// 		result = buffer[t];
// 		tail.store((t + 1) & (Size - 1), std::memory_order_release);
// 		return true;
// 	}
// };


class Price {
private:
	int64_t raw_value; 
	static constexpr int64_t SCALE = 10000;
	static constexpr int64_t POW10[] = {1, 10, 100, 1000, 10000};
	explicit constexpr Price(int64_t raw) noexcept : raw_value(raw) {}
public:
	constexpr Price() noexcept : raw_value(0) {}
	[[nodiscard]] static constexpr Price fromInteger(int64_t v) noexcept {
		return Price(v * SCALE);
	}
	[[nodiscard]] static constexpr Price fromRaw(int64_t v) noexcept {
		return Price(v);
	}
	[[nodiscard]] static constexpr Price fromString(const char* s) {
		int64_t int_part = 0;
		int64_t frac_part = 0;
		bool negative = false;
		int digits = 0;

		if (*s == '-') {
			negative = true;
			s++;
		}

		while (*s >= '0' && *s <= '9') {
			int_part = int_part * 10 + (*s - '0');
			s++;
		}

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
		int64_t total = int_part * SCALE + frac_part;
		return Price(negative ? -total : total);
	}
	[[nodiscard]] constexpr auto operator<=>(const Price&) const noexcept = default;
	[[nodiscard]] constexpr Price operator+(const Price& other) const noexcept {
		return Price(raw_value + other.raw_value);
	}
	[[nodiscard]] constexpr Price operator-(const Price& other) const noexcept {
		return Price(raw_value - other.raw_value);
	}
	constexpr Price& operator+=(const Price& other) noexcept {
		raw_value += other.raw_value;
		return *this;
	}
	constexpr Price& operator-=(const Price& other) noexcept {
		raw_value -= other.raw_value;
		return *this;
	}
	[[nodiscard]] constexpr int64_t toInternal() const noexcept {
		return raw_value;
	}
};

constexpr int64_t Price::POW10[];

// side
enum class Side : uint8_t {
	UNKNOWN,
	BUY,
	SELL
};

// BIT
template<size_t SIZE>
class FenwickTree {
	array<int64_t, SIZE + 1> tree;
public:
	void update(int price, int delta) {
		for(price++; price < SIZE + 1; price += price & -price) {
			tree[price] += delta;
		}
	}
	int64_t query(int price) {
		int64_t sum = 0;
		for(price++; price > 0; price -= price & -price) {
			sum += tree[price];
		}
		return sum;
	}
	int64_t queryRange(int low, int high) {
		return query(high) - query(low - 1);
	}
};

// bit mask
template<size_t SIZE>
class BitMask {
	static constexpr size_t wordSize = (SIZE + 63) / 64;
	array<uint64_t, wordSize> words{};
	array<uint64_t, (wordSize + 63) / 64> summary{};
public:
	void set(int price) {
		int wordIndex = price >> 6;
		words[wordIndex] |= (1ULL << (price & 63));
		summary[wordIndex >> 6] |= (1ULL << (wordIndex & 63));
	}
	void unset(int price) {
		int wordIndex = price >> 6;
		words[wordIndex] &= ~(1ULL << (price & 63));
		if (words[wordIndex] == 0)
			summary[wordIndex >> 6] &= ~(1ULL << (wordIndex & 63));
	}
	uint64_t getMaskGE(int bit) { return ~0ULL << (bit & 63); }
	uint64_t getMaskLE(int bit) { return ~(~0ULL << (bit & 63)) | (1ULL << (bit & 63)); }

	int getNextPrice(int price) {
		if (price < 0) price = 0;
		if (price >= (int) SIZE) {
			return -1;
		}

		int wordIndex = price >> 6;
		auto currentWord = words[wordIndex] & getMaskGE(price);
		if (currentWord) {
			return (wordIndex << 6) + __builtin_ctzll(currentWord);
		}
		
		int sumamryIndex = wordIndex >> 6;
		auto nextWord = summary[sumamryIndex] & (~(0ULL) << 1 << (wordIndex & 63));
		while(nextWord == 0 && ++sumamryIndex < (int)summary.size()) {
			nextWord = summary[sumamryIndex];
		}

		if (nextWord) {
			auto nextWordIndex = (sumamryIndex << 6) + __builtin_ctzll(nextWord);
			return (nextWordIndex << 6) + (__builtin_ctzll(words[nextWordIndex]));
		}
		return -1;
	}
	int getPrevPrice(int price) {
		if (price < 0) return -1;
		if (price >= (int)SIZE) price = (int)SIZE - 1;

		int wIdx = price >> 6;
		
		// 1. 檢查當前 Word (找小於等於 price 的最高位元)
		// 遮罩產生：保留從 bit 0 到 (price & 63) 的位元
		uint64_t curWordMask = ~(~0ULL << 1 << (price & 63));
		uint64_t curWord = words[wIdx] & curWordMask;
		
		if (curWord) {
			// __builtin_clzll 是算開頭有幾個 0，所以 63 - clz 就是最高位 1 的位置
			return (wIdx << 6) + (63 - __builtin_clzll(curWord));
		}

		// 2. 搜尋 Summary
		int sIdx = wIdx >> 6;
		// 檢查當前 summary 中，比 wIdx 小的位元
		// 遮罩：保留從位元 0 到 ((wIdx & 63) - 1) 的位元
		uint64_t curSumMask = ~(~0ULL << (wIdx & 63)); 
		uint64_t curSum = summary[sIdx] & curSumMask;

		// 如果當前 summary 沒了，往前面的 summary 找
		while (curSum == 0 && --sIdx >= 0) {
			curSum = summary[sIdx];
		}

		if (curSum) {
			// 找到前一個有資料的 Word 索引
			int prevWIdx = (sIdx << 6) + (63 - __builtin_clzll(curSum));
			// 在該 Word 中找最高位的 1
			return (prevWIdx << 6) + (63 - __builtin_clzll(words[prevWIdx]));
		}

		return -1;
	}
};
// order
struct alignas(64) Order {
	Order* prev = nullptr;
	Order* next = nullptr;
	int price = 0;
	int quantity = 0;
	uint64_t id = 0;
	Side side = Side::UNKNOWN;
	Order() = default;
	Order(uint64_t id, int p, int q, Side side):
		prev(nullptr), next(nullptr), id(id), price(p), quantity(q), side(side) {}
};
// order list
struct OrderList {
	Order* head = nullptr;
	Order* tail = nullptr;
	void push_back(Order* order) {
		if (!tail) {
			head = tail = order;
			order->next = nullptr;
			order->prev = nullptr;
		} else {
			tail->next = order;
			order->prev = tail;
			order->next = nullptr;
			tail = order;
		}
	}
	void remove(Order* order) {
		if(order->prev) order->prev->next = order->next;
		if(order->next) order->next->prev = order->prev;
		if(order == head) head = order->next;
		if(order == tail) tail = order->prev;
	}
};
// order pool
template<size_t SIZE>
class OrderPool {
	array<Order, SIZE> pool;
	Order* freeListHead;
public:
	OrderPool() {
		for(int i = 0; i < SIZE - 1; i++) {
			pool[i].next = &(pool[i + 1]);
		}
		freeListHead = &(pool[0]);
	}
	Order* allocate() {
		if (!freeListHead) return nullptr;
		Order* order = freeListHead;
		freeListHead = order->next;
		order->next = order->prev = nullptr;
		return order;
	}
	void deallocate(Order* order) {
		order->next = freeListHead;
		freeListHead = order;
	}
};
// order book
class OrderBook {
	struct alignas(64) InternalData {
		OrderPool<MAX_POOL_SIZE> pool;
		FenwickTree<MAX_PRICE> bidTree;
		FenwickTree<MAX_PRICE> askTree;
		BitMask<MAX_PRICE> bidMask;
		BitMask<MAX_PRICE> askMask;
		array<OrderList, MAX_PRICE> bidList, askList;
	};
	InternalData* d = nullptr;
	size_t total_size = sizeof(InternalData);
public:
	OrderBook(){
		void* addr = mmap(nullptr, total_size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (addr == MAP_FAILED) {
			throw std::runtime_error("mmap failed");
		}

		d = new (addr) InternalData();

		if (mlock(d, total_size) != 0) {
			perror("mlock failed (check ulimit -l)");
		}

		volatile char* p = reinterpret_cast<char*>(d);
		for(size_t i = 0; i < total_size ; i += 4096) {
			p[i] = 0;
		}
	}

	~OrderBook() {
		if (d) {
			d->~InternalData();
			munlock(d, total_size);
			munmap(d, total_size);
		}
	}

	int addOrder(uint64_t id, Side side, int price, int quantity) {
		int remainingQty = quantity;

		if (side == Side::BUY) {
			while(remainingQty > 0) {
				int bestAsk = d->askMask.getNextPrice(0);
				if (bestAsk == - 1 || bestAsk > price) break;

				auto& list = d->askList[bestAsk];
				while(list.head && remainingQty > 0) {
					auto askOrder = list.head;
					int fillQ = min(remainingQty, askOrder->quantity);

					askOrder->quantity -= fillQ;
					remainingQty -= fillQ;
					d->askTree.update(bestAsk, -fillQ);
					if (askOrder->quantity == 0) {
						list.remove(askOrder);
						d->pool.deallocate(askOrder);
					}
				}
				if (!list.head) {
					d->askMask.unset(bestAsk);
				}
			}

			if (remainingQty > 0) {
				auto order = d->pool.allocate();
				if (order) {
					*order = Order(id, price, remainingQty, side);
					d->bidTree.update(price, remainingQty);
					d->bidMask.set(price);
					d->bidList[price].push_back(order);
				}
				
			}
		} else if (side == Side::SELL) {
			while(remainingQty > 0) {
				int bestBid = d->bidMask.getPrevPrice(MAX_PRICE - 1);
				if (bestBid == - 1 || bestBid < price) break;

				auto& list = d->bidList[bestBid];
				while(list.head && remainingQty > 0) {
					auto bidOrder = list.head;
					int fillQ = min(remainingQty, bidOrder->quantity);

					bidOrder->quantity -= fillQ;
					remainingQty -= fillQ;
					d->bidTree.update(bestBid, -fillQ);
					if (bidOrder->quantity == 0) {
						list.remove(bidOrder);
						d->pool.deallocate(bidOrder);
					}
				}
				if (!list.head) {
					d->bidMask.unset(bestBid);
				}
			}

			if (remainingQty > 0) {
				auto order = d->pool.allocate();
				if (order) {
					*order = Order(id, price, remainingQty, side);
					d->askTree.update(price, remainingQty);
					d->askMask.set(price);
					d->askList[price].push_back(order);
				}
				
			}
		}
		return remainingQty;
	}

	int queryRange(int low, int high, Side side) {
		if (side == Side::BUY) {
			return d->bidTree.queryRange(low, high);
		} else if (side == Side::SELL) {
			return d->askTree.queryRange(low, high);
		} else {
			return 0;
		}
	}
};


// floating point
// lock fre

void run_comprehensive_tests() {
	OrderBook ob;

	// --- CASE 1: 價格邊界測試 ---
	std::cout << "Testing Case 1: Boundary Prices..." << std::endl;
	ob.addOrder(1, Side::BUY, 0, 100);
	ob.addOrder(2, Side::SELL, MAX_PRICE - 1, 100);
	assert(ob.queryRange(0, 0, Side::BUY) == 100);
	assert(ob.queryRange(MAX_PRICE - 1, MAX_PRICE - 1, Side::SELL) == 100);

	// --- CASE 2: BitMask 跨 Word 尋找 ---
	std::cout << "Testing Case 2: BitMask Cross-word jump..." << std::endl;
	ob.addOrder(3, Side::SELL, 63, 10);
	ob.addOrder(4, Side::SELL, 64, 10);
	// 買單在 65 元，應吃掉 63 與 64 元的賣單
	int rem_cross = ob.addOrder(5, Side::BUY, 65, 25);
	assert(rem_cross == 5); 
	assert(ob.queryRange(63, 64, Side::SELL) == 0);
	assert(ob.queryRange(65, 65, Side::BUY) == 5);

	// --- CASE 3: 精準撮合為 0 ---
	std::cout << "Testing Case 3: Perfect Fill..." << std::endl;
	ob.addOrder(6, Side::SELL, 200, 50);
	int rem_perfect = ob.addOrder(7, Side::BUY, 200, 50);
	assert(rem_perfect == 0);
	assert(ob.queryRange(200, 200, Side::SELL) == 0);

	// --- CASE 4: 買高賣低 (Aggressive) ---
	std::cout << "Testing Case 4: Aggressive Matching..." << std::endl;
	ob.addOrder(8, Side::SELL, 110, 10);
	ob.addOrder(9, Side::SELL, 120, 10);
	// 買單出 500 元，吃掉 110(10) 和 120(10)，剩餘 0
	int rem_agg = ob.addOrder(10, Side::BUY, 500, 20); 
	assert(rem_agg == 0);
	assert(ob.queryRange(110, 120, Side::SELL) == 0);

	// --- CASE 5: 區間查詢 (確保無殘留) ---
	// 這裡使用更高價格區間，避免與前面 (0~500) 的訂單重疊
	std::cout << "Testing Case 5: Complex Range Query..." << std::endl;
	ob.addOrder(11, Side::BUY, 2000, 10);
	ob.addOrder(12, Side::BUY, 3000, 20);
	ob.addOrder(13, Side::BUY, 4000, 30);
	// 查 2000~3000 應得 30
	assert(ob.queryRange(2000, 3000, Side::BUY) == 30);
	// 查 2000~4000 應得 60
	assert(ob.queryRange(2000, 4000, Side::BUY) == 60);

	// --- CASE 6: 記憶體池重複利用 ---
	std::cout << "Testing Case 6: Memory Pool Reallocation..." << std::endl;
	for(int i=0; i<1000; ++i) {
		ob.addOrder(10000+i, Side::SELL, 8000, 1);
		ob.addOrder(20000+i, Side::BUY, 8000, 1);
	}
	assert(ob.queryRange(8000, 8000, Side::SELL) == 0);

	// --- CASE 7: 價格間隔 (Price Gaps) ---
	std::cout << "Testing Case 7: Search with Price Gaps..." << std::endl;
	ob.addOrder(30, Side::SELL, 9000, 100);
	ob.addOrder(31, Side::SELL, 9500, 100);
	// 買單價格 9200，只能吃到 9000 的單
	int rem_gap = ob.addOrder(32, Side::BUY, 9200, 150);
	assert(rem_gap == 50); 
	assert(ob.queryRange(9000, 9000, Side::SELL) == 0);
	assert(ob.queryRange(9500, 9500, Side::SELL) == 100);

	std::cout << "\n>>> ALL EDGE CASES PASSED SUCCESSFULLY! <<<" << std::endl;
}

void test_pool_exhaustion() {
	OrderBook ob;
	std::cout << "Testing: Pool Exhaustion..." << std::endl;
	
	// 1. 填滿 Pool
	for(int i = 0; i < MAX_POOL_SIZE; ++i) {
		ob.addOrder(i, Side::BUY, 1000, 1);
	}
	
	// 2. 嘗試加入第 MAX_POOL_SIZE + 1 筆 (應處理 allocate 回傳 nullptr 的情況)
	int rem = ob.addOrder(99999, Side::BUY, 1000, 1);
	// 根據你目前的邏輯，如果 allocate 失敗，order 不會被加入，remainingQty 維持原樣
	assert(rem == 1); 

	// 3. 消耗掉部分訂單以釋放空間
	ob.addOrder(88888, Side::SELL, 1000, 500); 
	
	// 4. 再次嘗試加入，應成功
	// int rem2 = ob.addOrder(77777, Side::BUY, 1000, 1);
	// assert(rem2 == 0);
	std::cout << "Testing: Pool Exhaustion... Done" << std::endl;
}

void test_price_gaps() {
	OrderBook ob;
	std::cout << "Testing: Sparse Price Gaps..." << std::endl;

	// 分別在不同的 Word 區間放置訂單
	ob.addOrder(1, Side::SELL, 10, 1);     // Word 0
	ob.addOrder(2, Side::SELL, 4000, 1);   // Word 62
	ob.addOrder(3, Side::SELL, 8000, 1);   // Word 125
	ob.addOrder(4, Side::SELL, 9999, 1);   // Word 156 (接近 MAX_PRICE)

	// 一次性高價買單，測試 getNextPrice 跨 Word 掃描
	int rem = ob.addOrder(5, Side::BUY, 9999, 10);
	assert(rem == 6); // 10 - 4 = 6
	assert(ob.queryRange(0, 9999, Side::SELL) == 0);
	std::cout << "Testing: Sparse Price Gaps... Done" << std::endl;
}

void test_heavy_single_level() {
	OrderBook ob;
	std::cout << "Testing: Heavy Single Price Level..." << std::endl;

	// 在同一個價格掛上 5000 筆訂單
	for(int i = 0; i < 5000; ++i) {
		ob.addOrder(i, Side::SELL, 5000, 1);
	}
	assert(ob.queryRange(5000, 5000, Side::SELL) == 5000);

	// 用一筆大單一次吃光
	int rem = ob.addOrder(9999, Side::BUY, 5000, 5000);
	assert(rem == 0);
	assert(ob.queryRange(5000, 5000, Side::SELL) == 0);
	std::cout << "Testing: Heavy Single Price Level... Done" << std::endl;
}


void test_numerical_limits() {
	OrderBook ob;
	std::cout << "Testing: Numerical Limits..." << std::endl;

	// 1. 測試極大數量 (2^31 - 1 附近)
	int large_qty = 1000000;
	ob.addOrder(1, Side::BUY, 100, large_qty);
	ob.addOrder(2, Side::BUY, 100, large_qty);
	// 檢查 int64_t 是否正確處理
	assert(ob.queryRange(100, 100, Side::BUY) == 2000000000LL);

	// 2. 測試非法價格 (若未處理，BitMask 可能崩潰)
	// 這些應該在 addOrder 入口被擋掉或安全處理
	ob.addOrder(3, Side::BUY, -1, 100); 
	ob.addOrder(4, Side::BUY, MAX_PRICE + 100, 100);
	std::cout << "Testing: Numerical Limits... Done" << std::endl;
}


void test_price_precision() {
	std::cout << "Running: Precision & Accuracy Tests..." << std::endl;

	// CASE 1: 經典浮點數誤差測試 (0.1 + 0.2)
	// 在 double 中，0.1 + 0.2 != 0.3，但在 Price 中必須相等
	Price p1 = Price::fromString("0.1");
	Price p2 = Price::fromString("0.2");
	Price p3 = Price::fromString("0.3");
	assert((p1 + p2) == p3);
	assert((p1 + p2).toInternal() == 3000); // 0.3 * 10000

	// CASE 2: 不同長度的小數點解析
	assert(Price::fromString("1").toInternal() == 10000);
	assert(Price::fromString("1.2").toInternal() == 12000);
	assert(Price::fromString("1.23").toInternal() == 12300);
	assert(Price::fromString("1.234").toInternal() == 12340);
	assert(Price::fromString("1.2345").toInternal() == 12345);

	// CASE 3: 超過 4 位小數的處理 (預期為直接捨去 Truncation)
	assert(Price::fromString("1.2345678").toInternal() == 12345);
}

void test_price_boundaries() {
	std::cout << "Running: Boundary & Negative Tests..." << std::endl;

	// CASE 4: 負數解析
	Price neg = Price::fromString("-123.45");
	assert(neg.toInternal() == -1234500);

	// CASE 5: 極端大數
	// 922337203685477.5807 是 int64_t 帶 4 位小數的極限
	Price large = Price::fromString("1000000.0000"); 
	assert(large.toInternal() == 10000000000LL);

	// CASE 6: 零與小數點開頭
	assert(Price::fromString("0").toInternal() == 0);
	assert(Price::fromString("0.0001").toInternal() == 1);
	assert(Price::fromString(".5").toInternal() == 5000); // 某些 Parser 會遇到
}

void test_price_operators() {
	std::cout << "Running: Operator Tests..." << std::endl;

	Price a = Price::fromString("10.50");
	Price b = Price::fromString("20.25");
	
	assert(a < b);
	assert(b > a);
	assert((b - a) == Price::fromString("9.75"));
}



int main() {
	run_comprehensive_tests();
	// test_pool_exhaustion();
	test_price_gaps();
	test_heavy_single_level();
	// test_numerical_limits();
		

	test_price_precision();
	test_price_boundaries();
	test_price_operators();
		
	return 0;
}