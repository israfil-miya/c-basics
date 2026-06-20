#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <span>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

using namespace std;

void handleClient(int clientSocket, struct sockaddr_in clientAddress) {
  // Convert client's binary IP address to readable format
  char clientIp[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &clientAddress.sin_addr, clientIp, INET_ADDRSTRLEN);

  cout << "Success: Client connected from " << clientIp << ":"
       << ntohs(clientAddress.sin_port) << endl;

  std::this_thread::sleep_for(5s);

  // Receive data from client
  char streamBuffer[4096];
  span<char> bufferSpan(streamBuffer);
  ssize_t bytesReceived = 0;

  string requestData;
  requestData.reserve(2 * 4096);

  while (true) {
    bytesReceived =
        recv(clientSocket, bufferSpan.data(), bufferSpan.size() - 1, 0);

    if (bytesReceived <= 0) {
      break; // Error or client disconnect
    }

    span<char> receivedSpan = bufferSpan.subspan(0, bytesReceived);

    // Null-terminate the buffer
    streamBuffer[bytesReceived] = '\0';

    // cout << "Received buffer: " << streamBuffer << endl; // Not necessarily
    // the complete message
    requestData.append(receivedSpan.data(), receivedSpan.size());

    // For handling HTTP protocol communication specifically
    if (requestData.find("\r\n\r\n") != std::string::npos) {
      break; // We got the full request headers, so we stop reading
    }
  }

  if (bytesReceived == 0) {
    cout << "Client disconnected gracefully before request was complete."
         << endl;
  } else if (bytesReceived == -1) {
    cerr << "Error: recv() failed: " << strerror(errno) << endl;
  } else {
    cout << "Received request:\n" << requestData << endl;

    // Send HTTP response back to the client so the client doesn't wait forever
    string response = "HTTP/1.0 200 OK\r\n"
                      "Content-Type: text/html\r\n"
                      "Content-Length: 22\r\n"
                      "Connection: close\r\n"
                      "\r\n"
                      "<h1>Hello, World!</h1>";

    /*
     * HTTP doesn't necessarily need data/content in HTML, it could be just
     * plain text also. And, in TCP we can send any kind of data, here we are
     * just following the HTTP standard.
     * Content-Type: text/html  -> Signals browser to interpret the
     * content as HTML.
     * Content-Type: text/plain -> Signals browser to interpret the
     * content as plain text; don't interpret it like HTML, render as is.
     */

    if (send(clientSocket, response.c_str(), response.length(), 0) == -1) {
      cerr << "Error: Failed to send response to client!" << endl;
    } else {
      cout << "Success: Sent response to client." << endl;
    }
  }

  // Close the client socket when done (following HTTP/1.0 standard)
  close(clientSocket);
  cout << "Success: Client connection socket " << clientSocket << " is closed!"
       << endl;
}

int main() {
  // 1. Create a TCP socket
  int tcpSocket = socket(AF_INET, SOCK_STREAM, 0);

  if (tcpSocket == -1) {
    cerr << "Error: Failed to create socket!" << endl;
    return 1;
  }

  cout << "Success: TCP socket created with file descriptor: " << tcpSocket
       << endl;

  // Allow reuse of the port immediately after server exits, helpful for dev
  int opt = 1;
  if (setsockopt(tcpSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ==
      -1) {
    cerr << "Error: setsockopt(SO_REUSEADDR) failed!" << endl;
    close(tcpSocket);
    return 1;
  }

  // 2. Bind socket to an address and port
  struct sockaddr_in serverAddress;
  memset(&serverAddress, 0, sizeof(serverAddress));

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(5000);
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(tcpSocket, (struct sockaddr *)&serverAddress,
           sizeof(serverAddress)) == -1) {
    cerr << "Error: Failed to bind socket!" << endl;
    close(tcpSocket);
    return 1;
  }

  if (listen(tcpSocket, SOMAXCONN) == -1) {
    cerr << "Error: Failed to listen on socket!" << endl;
    close(tcpSocket);
    return 1;
  }

  cout << "Success: Socket is now listening for connections..." << endl;

  // Main loop, always keeps the listener open for any new client
  while (true) {
    struct sockaddr_in clientAddress;
    memset(&clientAddress, 0, sizeof(clientAddress));

    socklen_t clientAddressLength = sizeof(clientAddress);

    int clientSocket = accept(tcpSocket, (struct sockaddr *)&clientAddress,
                              &clientAddressLength);

    if (clientSocket == -1) {
      cerr << "Error: Failed to accept connection!" << endl;
      continue;
    }

    // handleClient(clientSocket, clientAddress);
    // we run each client in it's own thread, so the main listening thread
    // isn't blcoked when serving a client, and can always take new client
    // requests
    thread clientThread(handleClient, clientSocket, clientAddress);
    clientThread.detach();
  }

  // Just for good practice (cleanup) purpose,
  // though the program wouldn't reach here
  close(tcpSocket);
  return 0;
}
