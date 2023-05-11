#include "TCPRequestChannel.h"

using namespace std;


TCPRequestChannel::TCPRequestChannel (const std::string _ip_address, const std::string _port_no) {
    // Set up server side
    int portno = atoi(_port_no.c_str()); 
    struct sockaddr_in server_addr;


    if (_ip_address == "") {
        sockfd = socket(AF_INET,SOCK_STREAM,0);
        if (sockfd < 0) {
            cerr << "Error opening socket" << endl;
        }
        bzero((char *)&server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(portno);

        if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            cerr << "Error on binding" << endl;
        }
        listen(sockfd, 5);
        cout << "Server running and listening!!" << endl;
    }
    else {
        sockfd = socket(AF_INET,SOCK_STREAM,0);
        if (sockfd < 0) {
            cerr << "Error OPENING SOCKET" << endl;
        }
        struct hostent *server = gethostbyname(_ip_address.c_str());
        if (server == nullptr) {
            cerr << "ERROR WITH HOST" << endl;
        }

        bzero((char *)&server_addr, sizeof(server_addr));
        struct in_addr ip;
        if (inet_pton(AF_INET,_ip_address.c_str(), &ip) < 0) {
            cerr << "Error setting IP address" << endl;
        }
        //server_addr.sin_addr.s_addr = *server->h_addr_list[0];
        server_addr.sin_addr = ip;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(portno);

        if (connect(sockfd, (struct sockaddr*) &server_addr, sizeof (server_addr)) < 0) {
            cerr << "ERROR CONNECTING" << endl;
        }

    }

}

TCPRequestChannel::TCPRequestChannel (int _sockfd) {
    this->sockfd = _sockfd;
}

TCPRequestChannel::~TCPRequestChannel () {
    close(sockfd);
}

int TCPRequestChannel::accept_conn () {
    struct sockaddr_in client_addr;
    socklen_t clilen = sizeof (client_addr);
    int socketfd = accept(sockfd, (struct sockaddr *)&client_addr, &clilen);
    if (socketfd < 0 ) {
        cerr << "ERROR ON ACCEPT" << endl;
    }
    cout << "Successful connection" << endl;
    return socketfd;
}

int TCPRequestChannel::cread (void* msgbuf, int msgsize) {
    ssize_t num_bytes;
    num_bytes = read(sockfd, msgbuf, msgsize);
    if (num_bytes < 0) {
        cerr << "ERROR READING FROM SOCKET" << endl;
    }

    return num_bytes;
}

int TCPRequestChannel::cwrite (void* msgbuf, int msgsize) {
    ssize_t num_bytes;
    num_bytes = write(sockfd, msgbuf, msgsize);
    if (num_bytes < 0) {
        cerr << "ERROR WRITING FROM SOCKET" << endl;
    }

    return num_bytes;
}