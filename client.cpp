#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "employee.pb.h"

#define SERVER_IP "127.0.0.1"
#define PORT 9090
#define MAX_BUFFER 65507

int main() {
    // 1. Create UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    // 2. Set server address
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    // 3. Send a request trigger to the server
    const char* request = "GET_EMPLOYEES";
    sendto(sockfd, request, strlen(request), 0,
           (sockaddr*)&serverAddr, sizeof(serverAddr));

    std::cout << "Request sent to server. Waiting for response...\n";

    // 4. Receive serialized protobuf response
    char buffer[MAX_BUFFER];
    sockaddr_in fromAddr{};
    socklen_t fromLen = sizeof(fromAddr);

    int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER, 0,
                                 (sockaddr*)&fromAddr, &fromLen);
    if (bytesReceived < 0) {
        std::cerr << "recvfrom failed\n";
        return 1;
    }

    std::cout << "Received " << bytesReceived << " bytes from server\n\n";

    // 5. Deserialize protobuf
    EmployeeList empList;
    if (!empList.ParseFromArray(buffer, bytesReceived)) {
        std::cerr << "Failed to parse protobuf\n";
        return 1;
    }

    // 6. Print employees
    std::cout << "===== Employee List =====\n";
    for (const auto& emp : empList.employees()) {
        std::cout << "ID         : " << emp.id() << "\n";
        std::cout << "Name       : " << emp.name() << "\n";
        std::cout << "Department : " << emp.department() << "\n";
        std::cout << "Salary     : $" << emp.salary() << "\n";
        std::cout << "-------------------------\n";
    }

    close(sockfd);
    return 0;
}
