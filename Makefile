default:
	g++ paxos_client.cpp -o client -Wall -Wextra -std=c++11 -C
	g++ paxos_replica.cpp paxos_main.cpp communicator.cpp -o replica -Wall -Wextra -std=c++11 -C

clean:
	rm client replica