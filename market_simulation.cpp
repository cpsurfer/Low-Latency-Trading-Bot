#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <cstdlib>
#include <ctime>

using namespace std;

int main() {
    srand(time(0)); // Seed random number generator

    // 1. Create Socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("Socket creation failed"); return -1; }

    // 2. Setup Destination (Where is the Trader?)
    struct sockaddr_in traderAddr;
    traderAddr.sin_family = AF_INET;
    traderAddr.sin_port = htons(54321); // Trader is listening here
    traderAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 3. Non-Blocking Mode (So we don't freeze waiting for a Buy order)
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    double price = 152.00; // Starting price
    char buffer[1024];
    struct sockaddr_in fromAddr;
    socklen_t fromLen = sizeof(fromAddr);

    cout << "--- NASDAQ SIMULATOR STARTED ---" << endl;
    cout << "Sending prices to port 54321..." << endl;

    int tick = 0;
    while (true) {
        // A. Simulate Price Movement (Random Walk)
        // Move price by random amount between -0.50 and +0.50
        double change = ((rand() % 100) - 50) / 100.0;
        price += change;

        // Keep price positive
        if (price < 1.0) price = 152.00;

        // B. Format Packet: "GOOGL 150.25"
        string msg = "GOOGL " + to_string(price);
        
        // C. Send to Trader
        sendto(sock, msg.c_str(), msg.length(), 0, (struct sockaddr*)&traderAddr, sizeof(traderAddr));

        // D. Check for "BUY" Order (Did the trader react?)
        int len = recvfrom(sock, buffer, 1024, 0, (struct sockaddr*)&fromAddr, &fromLen);
        if (len > 0) {
            buffer[len] = '\0';
            cout << "[MATCH] Trade Executed at $" << price << " | Msg: " << buffer << endl;
        } else {
           // Optional: Print price feed
           // cout << "Tick " << tick << ": " << price << endl;
        }

        // E. Market Speed (Sleep for 100ms)
        // If you remove this, you will flood the trader with millions of packets!
        usleep(100000); 
        tick++;
    }

    return 0;
}
