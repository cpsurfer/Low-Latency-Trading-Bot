#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <x86intrin.h>
#include <fcntl.h>
#include <sched.h>

using namespace std;

// --- CONFIG ---
const int DEBUG_PACKET_COUNT = 10; // Print first 10 packets raw to prove connection
const double CPU_FREQ_GHZ = 2.0;   // ADJUST THIS to your CPU speed (e.g., 3.6)

// --- MACROS ---
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

unsigned long long rdtsc() { return __rdtsc(); }

// --- OPTIMIZED PARSER ---
double fast_parse(const char* p) {
    long long integer_part = 0;
    double divisor = 1.0;
    bool in_fraction = false;

    while (*p != '\0') {
        if (likely(*p >= '0' && *p <= '9')) { 
            integer_part = (integer_part * 10) + (*p - '0');
            if (in_fraction) divisor *= 10.0;
        } 
        else if (unlikely(*p == '.')) {
            in_fraction = true;
        } 
        else if (*p == ' ' || *p == '\n') {
            // Stop parsing on space or newline
            break; 
        }
        p++;
    }
    // If string was empty or invalid, return 0 to avoid crash
    if (divisor == 1.0 && integer_part == 0 && !in_fraction) return 0.0;
    
    return (double)integer_part / divisor;
}

int main(){
    // 1. PIN TO CORE 1
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset);
    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) != 0) {
        perror("WARNING: Could not pin CPU (Run with sudo?)");
    }

    // 2. SETUP SOCKET
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) { perror("Socket failed"); return -1; }

    // Set Non-Blocking (Busy Spin)
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54321);
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Listen on ALL interfaces

    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed (Is port 54321 already in use?)");
        return -1;
    }

    char buffer[1024];
    struct sockaddr_in clientAddr;
    socklen_t addr_len = sizeof(clientAddr);
    int packets_received = 0;

    cout << "=== HIGH FREQUENCY TRADER READY ===" << endl;
    cout << "Listening on Port 54321..." << endl;

    // --- MAIN LOOP ---
    while (true) {
        int bytes = recvfrom(sock, buffer, 1024, 0, (struct sockaddr*)&clientAddr, &addr_len);
        
        // BUSY SPIN: If no data, jump back immediately
        if (likely(bytes < 0)) continue;

        // --- CRITICAL PATH START ---
        unsigned long long start = rdtsc();

        buffer[bytes] = '\0';
        
        // 1. LOCATE PRICE
        // This handles "SYM 100.50" AND just "100.50"
        char* price_ptr = buffer;
        
        // If string starts with a letter (e.g. "S"), move to space
        if (*price_ptr >= 'A' && *price_ptr <= 'z') {
            while (*price_ptr != ' ' && *price_ptr != '\0') price_ptr++;
            if (*price_ptr == ' ') price_ptr++; // Move past the space
        }

        // 2. PARSE
        double price = fast_parse(price_ptr);

        // 3. DECIDE
        bool traded = false;
        // Raise limit slightly for testing purposes
        if (unlikely(price < 151.00 && price > 0.0)) {
            sendto(sock, "BUY", 3, 0, (struct sockaddr*)&clientAddr, addr_len);
            traded = true;
        }

        unsigned long long end = rdtsc();
        // --- CRITICAL PATH END ---

        packets_received++;

        // REPORTING
        // Print if TRADED OR if it's one of the first 10 packets (for debugging)
        if (traded || packets_received <= DEBUG_PACKET_COUNT) {
            double ns = (double)(end - start) / CPU_FREQ_GHZ;
            
            cout << (traded ? "[BUY] " : "[SKIP]") 
                 << " Price: " << price 
                 << " | Latency: " << ns << " ns" 
                 << (packets_received <= DEBUG_PACKET_COUNT ? " (DEBUG MODE)" : "") 
                 << endl;
        }
    }
    
    close(sock);
    return 0;
}
