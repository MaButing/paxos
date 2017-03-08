#include <vector>
#include <map>
#include <set>
#include "communiactor.h"
#include "ReqOrd.h"

using std::vector;
using std::map;
using std::set;





class paxos_replica
{
	int n;
	int f;
	int id;
	int my_king;
	int exe_end;
	vector<order> log;//seq -> order
	vector<set<int>> certf;//seq -> certificate_set
	int x;//number of seq to skip
	int reprop_begin;//OPTIMIZATION

	communiactor comm;

	//process of view change, the candidate king should not propose client request
	//because not sure about prevous log/state, cannot make decision: whether assign a new seq_num
	map<int> couping; //follower in couping process
	std::vector<request_t> pending_req;

	


	int propose();
	int accept();
	int learn();
	int coup();
	int follow();
	int admit();

public:
	paxos_replica():reprop_begin(0),exe_end(0){};
	paxos_replica(int n_in = 1, int f_in = 0, int id_in = 0):
	n(n_in), f(f_in), id(id_in),
	my_king(0),exe_end(0),x(0),reprop_begin(0),
	comm(n_in, id_in)
	{};
	int init(const vector<sockaddr_in>& addr_list_in);
	int run();
	int close();
	
};