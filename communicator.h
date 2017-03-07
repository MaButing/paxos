#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


#include <vector>
#include <iostream>

#define MAXBUFFSIZE 4096
#define MAXSENDSIZE MAXBUFFSIZE-sizeof(uint)-sizeof(size_t)

using std::vector;
using std::cerr;
using std::endl;
using std::strerror;

// a new session for every msg



typedef char byte;

class communicator
{
private:
    uint n;
    uint id;
    int sock;//listening socket
    vector<sockaddr_in> addr_list;

public:
    
    int comm_init(uint np, uint pid, const vector<sockaddr_in>& addr_list_in);
    int comm_send(uint dest_id, void* buff, size_t buff_size);
    int comm_recv(uint* source_id, void* buff, size_t buff_size);
    // int bcast();
    
};