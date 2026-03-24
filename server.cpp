#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "employee.pb.h"

#define PORT 9090
#define MAX_BUFFER 65507   // max UDP payload

EmployeeList buildEmployeeList() {
    EmployeeList list;

    auto* e1 = list.add_employees();
    e1->set_id(1);
    e1->set_name("Alice Johnson");
    e1->set_department("Engineering");
    e1->set_salary(95000.0f);

    auto* e2 = list.add_employees();
    e2->set_id(2);
    e2->set_name("Bob Smith");
    e2->set_department("Marketing");
    e2->set_salary(72000.0f);

    auto* e3 = list.add_employees();
    e3->set_id(3);
    e3->set_name("Carol White");
    e3->set_department("HR");
    e3->set_salary(68000.0f);

    return list;
}

int main() {
    // 1. Create UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    // 2. Bind to port
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed\n";
        return 1;
    }

    std::cout << "Server listening on port " << PORT << "...\n";

    while (true) {
        char buffer[MAX_BUFFER];
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);

        // 3. Wait for any request from client
        int bytesReceived = recvfrom(sockfd, buffer, MAX_BUFFER, 0,
                                     (sockaddr*)&clientAddr, &clientLen);
        if (bytesReceived < 0) {
            std::cerr << "recvfrom failed\n";
            continue;
        }

        std::cout << "Request received from client. Sending employee data...\n";

        // 4. Build and serialize protobuf
        EmployeeList empList = buildEmployeeList();
        std::string serialized;
        if (!empList.SerializeToString(&serialized)) {
            std::cerr << "Serialization failed\n";
            continue;
        }

        // 5. Send serialized protobuf back to client
        sendto(sockfd, serialized.c_str(), serialized.size(), 0,
               (sockaddr*)&clientAddr, clientLen);

        std::cout << "Sent " << serialized.size() << " bytes\n";
    }

    close(sockfd);
    return 0;
}
