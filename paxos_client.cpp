#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


#include <vector>
#include <iostream>

using namespace std;

int main(int argc, char const *argv[])
{
	if (argc != 3){
		cerr<<"usage: "<<argv[0]<<" <client_id> <client_seq(default 0)>"<<endl;
		return 0;
	}

	vector<sockaddr_in> addr_list;

	int client_id = atoi(argv[1]);
	int seq = atoi(argv[2]);
	string client_ip_str;
	int client_port_num;

	cout<<"init client: "<<endl;
	cout<<"input client ip_addr"<<endl;
	cin>>client_ip_str;
	cin>>client_port_num;

	int n;
	cout<<"input number of replicas: "<<endl;
	cin >> n;
	addr_list.resize(n);
	cerr<<"addr_list size = "<<addr_list.size()<<endl;
	memset(addr_list.data(), 0, addr_list.size()*sizeof(sockaddr_in));

	cout<<"input the ip_addr and port_num for all "<<n<<" replicas: "<<endl;
	for (int i = 0; i < n; ++i)
	{
		string ip_str; int port;
		cin >> ip_str >>port;

		addr_list[i].sin_family = AF_INET;
    	addr_list[i].sin_port = htons(port);
    	inet_pton(AF_INET, ip_str, &(addr_list[i].sin_addr));
	}

	int king = 0;

	timeval to_len = {1, 0};//init timeout length to 1 secend;
	string msg_str;
	while(1){
		msg_str.clear();
		cout<<"enter the message (!q to quit):"<<endl;
		getline(cin, msg_str);
		if (msg_str == "!q") break;


	    for(; king < n; king++){

	    	//send message

	    	int sock_send = socket(AF_INET, SOCK_STREAM, 0);
		    if( sock_send < 0){
		        cerr << "[send, create socket error]: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
		        break;
		    }
		    if( connect(sock_send, (sockaddr*)(addr_list.data()+king), sizeof(sockaddr)) < 0){
		        cerr << "[Error, connect to"<<king<<"]: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
		        close(sock_send);
		        continue;// move to next replica
		    }
			//send
		    

			close(sock_send);






			//wait for respond for time out
			int sock_listen = socket(AF_INET, SOCK_STREAM, 0);
			if( sock_listen < 0){
		        cerr << "[listen, create socket error]: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
		        break;
		    }

			if( bind(sock_listen, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
		        cerr << "[bind socket error]: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
		        close(sock_listen);
		        break;
		    }

		    if( listen(sock_listen, SOMAXCONN) == -1){
		        cerr << "[listen socket error]: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
		        close(sock_listen);
		        break;
		    }

		    fd_set rfds;
		    FD_ZERO(&rfds);
            FD_SET(sock_listen, &rfds);

            int retval = select(sock_listen+1, &rfds, NULL, NULL, &to_len);
            if (retval == -1){
                cerr << "[select error]"<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
                close(sock_listen);
                break;
            }
            else if (retval == 0){
            	cerr << "receive timeout, replica "<<king<<"is down."<<endl;
                if (to_len.tv_sec < 10) to_len.tv_sec *= 2;
                close(sock_listen);
                continue; //move to next king
            }

            //accept
            ockaddr_in clientaddr;
    		socklen_t peer_addr_size = sizeof(struct sockaddr_in);
            if( (sock_recv = accept(sock_listen, (struct sockaddr*)&clientaddr, &peer_addr_size)) == -1){
		        cerr << "accept socket error: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
		        return -1;
		    }
            //receive



		    close(sock_recv);
            close(sock_listen);
            

		}


		if (king == n){
			cerr << "all replicas are down, exit!"<<endl;
			break;
		}

		
		

	}


	return 0;
}
