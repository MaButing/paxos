#include <vector>
#include <map>
#include <set>
#include <string>
#include "communiactor"

using std::vector;
using std::map;
using std::set;
using std::string;


struct request_t
{
	int client_id;
	int client_seq;
	string client_ip_str;
	int client_port;
	string msg;
	request_t():client_id(-1),client_seq(-1),client_port(-1){};
};


struct order_t
{
	int seq;
	int view;
	request_t req;
	order_t():seq(-1),view(-1){};
};



class paxos_replica
{
	int n;
	int id;
	int my_king;
	int exe_end;
	vector<order> log;//seq -> order
	vector<set<int>> certf;//seq -> certificate_set

	communiactor comm;


	int propose();
	int accept();
	int learn();
	int coup();
	int follow();
	int admit();

	order_t order_parse(const string& str);
	request_t req_parse(const string& str);

	//process of view change, the candidate king should not propose client request
	//because not sure about prevous log/state, cannot make decision: whether assign a new seq_num
	int couping; //number of follower in couping process
	std::vector<request_t> pending_req;


public:
	paxos_replica():couping(0){};
	~paxos_replica();
	int init();
	int run();
	int close();
	
};