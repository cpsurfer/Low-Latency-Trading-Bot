#include<iostream>      //standard input/output
#include<sys/socket.h>  //main header for network sockets
#include<cstring>       //string manipulation
#include<arpa/inet.h>   //converting IP addresses
#include<unistd.h>      //converting socket(UNIX standard)
#include<x86intrin.h>   //This gives us access to the __rdtsc() intrinsic
using namespace std;

//wrapper function to read cpu cycle counter
unsigned long long rdtsc(){
    return __rdtsc();   //asks the cpu for the cycle count
}


int main(){
    //create a UDP socket
    //AF_INET       =IPv4
    //SOCK_DGRAM    =UDP(datagram)
    //0             =Default protocol(let OS decide)

    int sock=socket(AF_INET,SOCK_DGRAM,0);
    if(sock<0){
        cout<<"Creation of socket faile\n";
        return -1;
    }

    struct sockaddr_in serverAddr;  //like a shipping label(sockaddr)   

    //1.set the adress family same as socket
    serverAddr.sin_family = AF_INET;

    //2.set port number
    //we use htons() to convert number to "Network Byte order" so internet understands it
    serverAddr.sin_port=htons(54321);

    //3.set ip address
    //INADDR_ANY means listen on any available network cards
    serverAddr.sin_addr.s_addr=INADDR_ANY;

    // Bind the socket to the address and port
    // We cast &serverAddr to (struct sockaddr*) to satisfy the function requirements
    if((bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr))<0)){
        cout<<"Binding failed\n";
        return -1;
    }

    //create a buffer to store incoming messages
    char Buffer[1024];
    
    //create space to store sender's details(Ip and port)
    struct sockaddr_in clientAddr;
    socklen_t addr_len = sizeof(clientAddr);
    
    cout<<"waiting for sender data\n";
    
    //receive packet(this pause the program until pakcet arrives)
    //recvfrom returns the number of bytes received
    int bytes_received = recvfrom(sock, Buffer, 1024, 0, (struct sockaddr*)&clientAddr, &addr_len);

    if(bytes_received<0){
        cout << "Receive failed\n";
        return -1;
    }

    //Add a null terminator to make it a valid C++ string
    Buffer[bytes_received] = '\0';

    // --- START TIMER ---
    // Capture the CPU cycle count right now
    unsigned long long start_tick = rdtsc();

    // --- WORK (Parsing & Printing) ---
    // In a real HFT system, you would parse binary data here.
    // For now, we just print the message.
    cout << "Received Packet: " << Buffer << endl;

    // --- END TIMER ---
    // Capture the CPU cycle count immediately after printing
    unsigned long long end_tick = rdtsc();

    // --- REPORT ---
    unsigned long long total_cycles = end_tick - start_tick;
    cout << "Latency (CPU Cycles): " << total_cycles << endl;

    // AMD Ryzen 5 7530U Base Clock = 2.0 GHz
    const double cpu_frequency_ghz = 2.0;

    double time_ns = total_cycles / cpu_frequency_ghz;

    cout << "Time:   " << time_ns << " nanoseconds" << endl;

    return 0;
}

