#include "communicator.h"

int communicator::comm_init(uint np, uint pid, const vector<sockaddr_in>& addr_list_in)
{
    n = np;
    id = pid;
    addr_list = addr_list_in;
    if (n > addr_list.size()){
    	cerr<<"addr_list too short"<<endl;
    	return -1;
    }
    if (id >= n){
    	cerr<<"id out of range"<<endl;
    }

    //open the listening socket
    if( (sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
        cerr << "[create listening socket error]: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
        return -1;
    }

    sockaddr_in servaddr = addr_list[id];
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if( bind(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
        cerr << "[bind socket error]: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
        return -1;
    }

    if( listen(sock, SOMAXCONN) == -1){
        cerr << "[listen socket error]: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
        return -1;
    }

    return 0;
}

int communicator::comm_send(uint dest_id, void* buff, size_t buff_size)
{
	
	if (buff_size > MAXSENDSIZE){
        cerr << "[send, buff_size error], MAXSENDSIZE is "<<MAXSENDSIZE<<endl;
		return -1;
	}

	int sock_send;
    if( (sock_send = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        cerr << "[send, create socket error]: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
        return -1;
    }
    if( connect(sock_send, (sockaddr*)(addr_list.data()+dest_id), sizeof(sockaddr)) < 0){
        cerr << "[send, connect error]: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
        return -1;
    }
    cerr<<"connected"<<endl;

    //send msg_len
    size_t msg_len = buff_size+sizeof(size_t)+sizeof(uint);
    int send_len = send(sock_send, &msg_len, sizeof(size_t), MSG_NOSIGNAL);
    if (send_len < 0){
        cerr << "[send, send msg_size error]: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
        return -1;
    }
    //send source_id
    send_len = send(sock_send, &dest_id, sizeof(uint), MSG_NOSIGNAL);
    if (send_len < 0){
        cerr << "[send, send sourse_id error]: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
        return -1;
    }
    //send buff
    send_len = send(sock_send, buff, buff_size, MSG_NOSIGNAL);
    if (send_len < 0){
        cerr << "[send, send buff error]: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
        return -1;
    }
    
    cerr<<"sent";

    close(sock_send);
    return send_len;
}



int communicator::comm_recv(uint* source_id, void* buff, size_t buff_size)
{
    sockaddr_in clientaddr;
    socklen_t peer_addr_size = sizeof(struct sockaddr_in);

    cerr<<"before accept"<<endl;
    int sock_recv;
    if( (sock_recv = accept(sock, (struct sockaddr*)&clientaddr, &peer_addr_size)) == -1){
        cerr << "accept socket error: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
        return -1;
    }

    cerr<<"accept"<<endl;

    byte buffer[MAXBUFFSIZE];
    byte *buffer_ptr = buffer;
    size_t sum_len = 0;
    size_t msg_len = 0;

    while(1){
        int recv_len = recv(sock_recv, buffer_ptr, MAXBUFFSIZE-sum_len, 0);
        if (recv_len < 0){
            cerr << "recv error: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
            break;
        }
        if (recv_len == 0) break;
        
        sum_len += recv_len;
        buffer_ptr = buffer_ptr + recv_len;

        if (msg_len == 0){
            if ( sum_len > 2*sizeof(uint)){
                msg_len = *((uint*) buffer);
                *source_id = *(((uint*)buffer)+1);
                cerr<<"msg_len: "<<msg_len<<",source_id:"<<*source_id<<endl;
            }
        }
        else{ // msg_len > 0
            if (sum_len >= msg_len)
                break;
        }

    }
    //err checking, sum_len should be smaller than buffsize

    memcpy(buff, buffer+sizeof(size_t)+sizeof(uint), buff_size);

    cerr<<"recv"<<endl;
	close(sock_recv);
    return sum_len;
    
}