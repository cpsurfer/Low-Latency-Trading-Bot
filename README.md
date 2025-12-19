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

