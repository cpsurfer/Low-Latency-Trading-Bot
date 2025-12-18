#include<iostream>
#include<sys/socket.h>
#include<cstring>
#include<arpa/inet.h>
#include<unistd.h>
#include<x86intrin.h>

using namespace std;

unsigned long long rdtsc() { return __rdtsc(); }

// --- CUSTOM HFT PARSER ---
// Does not check for errors. Assumes format "123.45"
// This is pure CPU math, no library calls.


// --- CUSTOM HFT PARSER (FIXED) ---
double fast_parse(const char* p) {
    double integer_part = 0.0;
    double divisor = 1.0;
    bool in_fraction = false;

    while (*p != '\0') {
        if (*p == '.') {
            in_fraction = true;
        }
        else if (*p >= '0' && *p <= '9') { // CHECK: Is it actually a digit?
            int digit = *p - '0';
            if (!in_fraction) {
                integer_part = integer_part * 10.0 + digit;
            } else {
                divisor *= 10.0;
                integer_part += digit / divisor;
            }
        }
        else {
            // Found a newline, space, or junk -> STOP PARSING
            break;
        }
        p++;
    }
    return integer_part;
}

int main(){
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) return -1;

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54321);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) return -1;

    char buffer[1024];
    struct sockaddr_in clientAddr;
    socklen_t addr_len = sizeof(clientAddr);

    cout << "Waiting for data..." << endl;

    int bytes_received = recvfrom(sock, buffer, 1024, 0, (struct sockaddr*)&clientAddr, &addr_len);
    if (bytes_received < 0) return -1;

    // ============================================
    //         CRITICAL PATH
    // ============================================
    
    unsigned long long start_tick = rdtsc();

    buffer[bytes_received] = '\0';
    
    // Manual search for space (faster than strchr)
    char* price_ptr = buffer;
    while (*price_ptr != ' ' && *price_ptr != '\0') {
        price_ptr++;
    }

    double price = 0.0;
    if (*price_ptr == ' ') {
        // Pass the address of the character AFTER the space
        price = fast_parse(price_ptr + 1);
    }

    unsigned long long end_tick = rdtsc();
    
    // ============================================

    cout << "Received: " << buffer << endl;
    cout << "Parsed Price: " << price << endl;

    unsigned long long total_cycles = end_tick - start_tick;
    cout << "Latency (Cycles): " << total_cycles << endl;

    const double cpu_frequency_ghz = 2.0; 
    double time_ns = total_cycles / cpu_frequency_ghz;
    
    cout << "Time:   " << time_ns << " nanoseconds" << endl;

    close(sock);
    return 0;
}
