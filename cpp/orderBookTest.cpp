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

constexpr int MAX_POOL_SIZE = 1000000;
constexpr int MAX_PRICE = 2000000;

using namespace std;

class Price {
	int64_t raw_value;
public:
};

// side
enum class Side : uint8_t {
	UNKNOWN,
	BUY,
	SELL
};

// BIT
template<size_t SIZE>
class FenwickTree {
	
public:
	
};

// bit mask
template<size_t SIZE>
class BitMask {
	
};
// order
struct alignas(64) Order {
	
};
// order list
struct OrderList {
	
};
// order pool
template<size_t SIZE>
class OrderPool {

};
// order book
class OrderBook {

};

void run_comprehensive_tests() {
	OrderBook ob;

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

	std::cout << "\n>>> ALL TESTS PASSED SUCCESSFULLY! <<<" << std::endl;
		
	return 0;
}