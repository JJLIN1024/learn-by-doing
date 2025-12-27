# task_struct

## Roadmap

### CPU Scheduling

* [ ] **vruntime 監測**：對比不同 `nice` 值下 `se.vruntime` 的增加斜率。
* [ ] **CPU 綁定**：修改 `cpus_ptr` 並驗證 Task 在核心間的遷移行為。
* [ ] **Context Switch 統計**：量化 `nvcsw` (自願) 與 `nivcsw` (非自願) 的數值變化。
* [ ] **排程策略切換**：變更 `policy` (SCHED_FIFO/RR/NORMAL) 並觀察對優先級的影響。

---

### 2. Memory Management

* [ ] **mm 指標驗證**：檢查多執行緒間 `mm` 是否共享，以及 Kernel Thread 的 `mm` 是否為 NULL。
* [ ] **Page Fault 追蹤**：記錄 `min_flt` 與 `maj_flt` 的觸發次數。
* [ ] **RSS 狀態觀察**：比對 `task_struct` 內的記憶體計數與實體記憶體佔用。
* [ ] **OOM 分數分析**：探究 `oom_score_adj` 如何影響 Kernel 的殺進程決策。
---

### 3. I/O Subsystem

* [ ] **I/O 計帳驗證**：比對 `ioac` 數據與實體磁碟讀寫流量。
* [ ] **D 狀態捕捉**：監控 `__state` 進入 `TASK_UNINTERRUPTIBLE` 的情境。
* [ ] **FD 表格擴張**：測試 `files_struct` 在大量 `open` 下的資源極限。
* [ ] **I/O 延遲追蹤**：分析 `delays` 欄位以獲取 Task 等待磁碟的精確時間。
