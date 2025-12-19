# NanoTrade: Ultra-Low Latency Trading Simulation

**NanoTrade** is a bare-metal C++ High-Frequency Trading (HFT) simulation designed to explore the limits of standard Linux networking. It implements a UDP-based market data feed and an execution engine optimized for microsecond-level latency.

The project demonstrates the progression from a standard socket implementation (~200µs latency) to a highly optimized, busy-spinning, CPU-pinned engine (**~35µs latency**).

## 🚀 Key Features

* **UDP Market Data Feed:** Simulates a NASDAQ-style random walk ticker.
* **Busy Spinning (O_NONBLOCK):** Avoids OS context switching overhead by keeping the CPU awake.
* **CPU Pinning:** Isolates the process to a specific core to prevent cache thrashing.
* **Zero-Copy Parsing:** Custom integer-based parsing replaces slow `strtod`.
* **Branch Prediction:** Uses `likely()` / `unlikely()` macros to optimize the hot path.

## 📊 Performance Benchmarks

All tests run on standard Linux kernel (Loopback UDP).

| Optimization Stage | Latency (ns) | Notes |
| :--- | :--- | :--- |
| **Standard (Blocking)** | ~210,000 ns | High jitter due to OS sleep/wake cycles. |
| **Optimized (This Repo)** | **~35,000 ns** | **6x Improvement.** CPU pinned, no sleeping. |

## 🛠️ How to Run

### 1. The Market (Simulator)
This acts as the exchange, broadcasting prices.
```bash
g++ market.cpp -o market
./market
```

### 2. The Trader (HFT Engine)
In a separate terminal, run the bot. (Sudo required for CPU pinning).
```bash
g++ -O3 -march=native trader.cpp -o trader
sudo ./trader
```
### 3. The faster Trader (HFT Engine)
In a seperate terminal, run the bot, (use sudo for CPU pinning).
```bash
g++ -O3 -march=native update_engine.cpp -o updated_engine
sudo ./updated_engine
```
Use 2. and 3 to compare performance
### Comparison: Basic vs. Optimized HFT Engine

| Feature | Basic Version (Initial Code) | Optimized Version (Current Code) | Performance Impact |
| :--- | :--- | :--- | :--- |
| **I/O Strategy** | **Blocking**<br>`recvfrom` puts the process to sleep if no data is present. Waking up takes ~3–10µs. | **Busy Spinning**<br>`O_NONBLOCK` + `while(true)` keeps the CPU active 100% of the time. Zero wakeup latency. | **Massive**<br>(Eliminates OS context switch overhead). |
| **CPU Scheduling** | **Standard**<br>The OS moves the thread between cores, causing CPU cache misses (cold cache). | **Pinned**<br>`sched_setaffinity` locks the process to Core 1, keeping L1/L2 caches warm. | **High**<br>(Prevents cache thrashing and migration jitter). |
| **Price Parsing** | **Floating Point**<br>Uses `double` division inside the loop for every decimal digit. | **Integer Math**<br>Parses as `long long` and divides only **once** at the very end. | **Medium**<br>(Integer math is ~20x faster than FDIV). |
| **Branching** | **Standard if/else**<br>Compiler guesses which path to take (50/50 chance). | **Branch Prediction**<br>`likely()` / `unlikely()` macros tell the compiler exactly which path is hot. | **Low/Medium**<br>(Optimizes CPU instruction pipeline). |
| **Logging** | **Sync cout**<br>Prints on every loop iteration, blocking execution. | **Conditional**<br>Only prints on trade execution or debug events; separates I/O from logic. | **Critical**<br>(Console I/O is extremely slow and blocks networking). |
| **Latency** | **~200,000 ns** (200µs) | **~35,000 ns** (35µs) | **~6x Speedup** |
