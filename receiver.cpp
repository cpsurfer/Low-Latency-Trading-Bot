#include<iostream>      // standard input/output
#include<sys/socket.h>  // main header for network sockets
#include<cstring>       // string manipulation (strchr)
#include<cstdlib>       // standard library (strtod for parsing)
#include<arpa/inet.h>   // converting IP addresses
#include<unistd.h>      // converting socket(UNIX standard)
#include<x86intrin.h>   // REQUIRED for rdtsc (CPU time)

using namespace std;

// Wrapper to read the CPU Time-Stamp Counter
unsigned long long rdtsc() {
    return __rdtsc();
}

int main(){
    // 1. Create a UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0){
        cout << "Creation of socket failed\n";
        return -1;
    }

    // 2. Define the Server Address (Port 54321)
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54321);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // 3. Bind the socket
    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cout << "Binding failed\n";
        return -1;
    }

    char buffer[1024];
    struct sockaddr_in clientAddr;
    socklen_t addr_len = sizeof(clientAddr);

    cout << "Waiting for market data (e.g., 'GOOGL 150.25')..." << endl;

    // --- BLOCKING WAIT ---
    int bytes_received = recvfrom(sock, buffer, 1024, 0,
                                  (struct sockaddr*)&clientAddr, &addr_len);

    if (bytes_received < 0) return -1;

    // ============================================
    //         CRITICAL PATH (HOT PATH)
    // ============================================
    
    // --- START TIMER ---
    unsigned long long start_tick = rdtsc();

    // 1. Safe Termination
    buffer[bytes_received] = '\0';

    // 2. Find the separator (Space)
    // strchr scans the memory looking for ' '
    char* price_str = strchr(buffer, ' ');

    double price = 0.0;
    if (price_str != NULL) {
        // 3. Convert string to double
        // price_str points to " ", so price_str + 1 points to "150.25"
        price = strtod(price_str + 1, NULL);
    }

    // --- END TIMER ---
    unsigned long long end_tick = rdtsc();
    
    // ============================================
    //         END CRITICAL PATH
    // ============================================

    // --- NOW PRINT ---
    cout << "Received: " << buffer << endl;
    cout << "Parsed Price: " << price << endl;

    // --- REPORT LATENCY ---
    unsigned long long total_cycles = end_tick - start_tick;
    cout << "Latency (Cycles): " << total_cycles << endl;

    // AMD Ryzen 5 7530U Base Clock = 2.0 GHz
    const double cpu_frequency_ghz = 2.0; 
    double time_ns = total_cycles / cpu_frequency_ghz;
    
    cout << "Time:   " << time_ns << " nanoseconds" << endl;

    close(sock);
    return 0;
}
