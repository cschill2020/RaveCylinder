#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// Client side implementation for a UDP datagram server.
// Note that this is a datagram client that is pushing
// data to the server.  Datagram sockets are connectionless
// and we do not expect a response. For more information:
// https://beej.us/guide/bgnet/html/#datagram
class UDPClient {
public:
  UDPClient() {}

  ~UDPClient() { close(socket_); }

  bool OpenConnection(const char *host, const char *port) {
    struct addrinfo address_hints;
    struct addrinfo *address_result;

    memset(&address_hints, 0, sizeof(address_hints));
    address_hints.ai_family =
        AF_INET; // Since your original code was using sockaddr_in and
                 // PF_INET, I'm using AF_INET here to match.  Use
                 // AF_UNSPEC instead if you want to allow getaddrinfo()
                 // to find both IPv4 and IPv6 addresses for the hostname.
                 // Just make sure the rest of your code is equally family-
                 // agnostic when dealing with the IP addresses associated
                 // with this connection. For instance, make sure any uses
                 // of sockaddr_in are changed to sockaddr_storage,
                 // and pay attention to its ss_family field, etc...
    address_hints.ai_socktype = SOCK_DGRAM;
    address_hints.ai_protocol = IPPROTO_UDP;

    // Resolve the domain name into a list of addresses.
    int error = getaddrinfo(host, port, &address_hints, &address_result);
    if (error != 0) {
      perror("getaddrinfo");
      return false;
    }

    // loop through all the results and connect to the first we can
    for (server_address_ = address_result; server_address_ != NULL;
         server_address_ = server_address_->ai_next) {
      void *address_ptr = nullptr;
      socket_ = socket(server_address_->ai_family, server_address_->ai_socktype,
                       server_address_->ai_protocol);
      if (socket_ < 0) {
        perror("socket");
        continue;
      }
      // if (connect(socket_, server_address_->ai_addr,
      //             server_address_->ai_addrlen) == -1) {
      //   close(socket_);
      //   perror("client: connect");
      //   continue;
      // }
      return true;
    }

    // Is this bad (address_result was copied to private member
    // server_address_)?
    freeaddrinfo(address_result);
    return false;
  }

  void SendTo(const std::vector<uint8_t> &bytes) {
    const char *char_bytes = reinterpret_cast<const char *>(bytes.data());
    int num_bytes =
        sendto(socket_, char_bytes, bytes.size(), 0, server_address_->ai_addr,
               server_address_->ai_addrlen);

    if (num_bytes < 0) {
      perror("sendto");
    }
  }

private:
  int socket_;
  struct addrinfo *server_address_;
};
