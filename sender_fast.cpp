#include<iostream>      // standard input/output
#include<sys/socket.h>  // main header for network sockets
#include<cstring>       // string manipulation
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
    serverAddr.sin_port = htons(54321);       // Port to listen on
    serverAddr.sin_addr.s_addr = INADDR_ANY;  // Listen on any IP

    // 3. Bind the socket to the address
    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cout << "Binding failed\n";
        return -1;
    }

    // 4. Prepare to receive data
    char buffer[1024];
    struct sockaddr_in clientAddr;
    socklen_t addr_len = sizeof(clientAddr);

    cout << "Waiting for sender data..." << endl;

    // --- BLOCKING WAIT ---
    // The program stops here until a packet arrives
    int bytes_received = recvfrom(sock, buffer, 1024, 0,
                                  (struct sockaddr*)&clientAddr, &addr_len);

    if (bytes_received < 0) {
        cout << "Receive failed\n";
        return -1;
    }

    // ============================================
    //         CRITICAL PATH (HOT PATH)
    // ============================================
    
    // --- START TIMER ---
    unsigned long long start_tick = rdtsc();

    // WORK: Only memory access, NO printing!
    buffer[bytes_received] = '\0';
    volatile char first_char = buffer[0]; // "volatile" forces CPU to actually read it

    // --- END TIMER ---
    unsigned long long end_tick = rdtsc();
    
    // ============================================
    //         END CRITICAL PATH
    // ============================================

    // --- NOW PRINT (Slow Path) ---
    cout << "Received Packet: " << buffer << endl;

    // --- REPORT LATENCY ---
    unsigned long long total_cycles = end_tick - start_tick;
    cout << "Latency (CPU Cycles): " << total_cycles << endl;

    // AMD Ryzen 5 7530U Base Clock = 2.0 GHz
    const double cpu_frequency_ghz = 2.0; 

    double time_ns = total_cycles / cpu_frequency_ghz;
    cout << "Time:   " << time_ns << " nanoseconds" << endl;

    close(sock);
    return 0;
}
