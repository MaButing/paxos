client:
	g++ paxos_client.cpp -o client -Wall -Wextra -std=c++11

replica:
	g++ paxos_replica.cpp paxos_main.cpp communicator.cpp -o replica -Wall -Wextra -std=c++11