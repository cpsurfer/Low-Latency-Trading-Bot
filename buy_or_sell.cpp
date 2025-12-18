#include<iostream>
#include<sys/socket.h>
#include<cstring>
#include<arpa/inet.h>
#include<unistd.h>
#include<x86intrin.h>

using namespace std;

unsigned long long rdtsc() { return __rdtsc(); }

// --- FAST PARSER ---
double fast_parse(const char* p) {
    double integer_part = 0.0;
    double divisor = 1.0;
    bool in_fraction = false;

    while (*p != '\0') {
        if (*p == '.') {
            in_fraction = true;
        } 
        else if (*p >= '0' && *p <= '9') { 
            int digit = *p - '0'; 
            if (!in_fraction) integer_part = integer_part * 10.0 + digit;
            else { divisor *= 10.0; integer_part += digit / divisor; }
        } 
        else break; 
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

    cout << "Waiting for market data..." << endl;

    // --- LOOP FOREVER ---
    while (true) {
        // 1. Receive
        int bytes_received = recvfrom(sock, buffer, 1024, 0, (struct sockaddr*)&clientAddr, &addr_len);
        if (bytes_received < 0) continue;

        // ============================================
        //         CRITICAL PATH
        // ============================================
        unsigned long long start_tick = rdtsc();

        buffer[bytes_received] = '\0';
        char* price_ptr = buffer;
        while (*price_ptr != ' ' && *price_ptr != '\0') price_ptr++;

        double price = 0.0;
        bool traded = false;

        if (*price_ptr == ' ') {
            price = fast_parse(price_ptr + 1);
            if (price < 151.00) {
                // SEND BUY ORDER
                sendto(sock, "BUY", 3, 0, (struct sockaddr*)&clientAddr, addr_len);
                traded = true;
            }
        }

        unsigned long long end_tick = rdtsc();
        // ============================================

        // PRINT REPORT ONLY (Don't measure this!)
        unsigned long long total_cycles = end_tick - start_tick;
        const double cpu_frequency_ghz = 2.0;
        double time_ns = total_cycles / cpu_frequency_ghz;

        cout << "Price: " << price << " | Action: " << (traded ? "BUY" : "HOLD")
             << " | Time: " << time_ns << " ns" << endl;
    }
   
    close(sock);
    return 0;
}
