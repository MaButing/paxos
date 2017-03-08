#include "communicator.h"
#include <iostream>
#include <cstdlib>
#include <string>
using namespace std;

int main(int argc, char const *argv[])
{
	if (argc != 3) return0;
	int n = atoi(argv[1]);
	int id = atoi(argv[2]);

	// sockaddr_in addr;
	// memset(&addr, 0, sizeof(sockaddr_in));
	vector<sockaddr_in> addr_list(n);
	memset(addr_list.data(), 0, addr_list.size()*sizeof(sockaddr_in));
	for (int i = 0; i < n; ++i)
	{
		addr_list[i].sin_family = AF_INET;
    	addr_list[i].sin_port = htons(6666+i*1111);
    	inet_pton(AF_INET, "127.0.0.1", &(addr_list[i].sin_addr));
	}

	communicator comm;
	comm.comm_init(n, atoi(argv[2]), addr_list);

	cerr<<"init"<<endl;
	char buff[MAXSENDSIZE];
	string str;
	
	if (id == 0){
		int source;
		while(1){
			comm.comm_recv(&source, (void*)buff, MAXSENDSIZE);
			cout<<source<<":"<<buff<<endl;
		}
	}
	else{
		while(1){
			cin >> str;
			cerr<<comm.comm_send(0, (void*)str.c_str(), str.length()+1);
		}
	}



	return 0;
}
