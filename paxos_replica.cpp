#include "paxos_replica.h"
#include <cassert>

using std::assert;



int paxos_replica::init();
int paxos_replica::run();
int paxos_replica::close();




int paxos_replica::propose(const request_t& req)// on receiving a client request
{ 
	//I am not the king, but I shall be the king, because client(god) want me to.
	if (my_king < id)
		coup();

	if (couping > 0){
		pending_req.push_back(req);
		return 0;
	}

	if (req_exist(req)) //this request has been assigned a seq num
		return 0;
	
	//create a new log record
	order_t order;
	order.seq = log.size()+x;
	order.view = my_king; //my king should be myself
	order.req = req;

	log.push_back(order);

	//propose to every other replica, 
	...

}

bool paxos_replica::req_exist(const request_t& req){
	//go through all the log
	for (auto i = log.begin(); i != log.end(); ++i)
	{
		if (i->req == req)
			return true;
	}
	return false;
}

int paxos_replica::accept(const order_t& ord) //on receiving a PROPOSAL
{
	if (ord.view < my_king)//from a old/dead king
		return 0;//ignore
	if (ord.seq < exe_end)//executed
		return 0;//ignore

	if (ord.view > my_king) //there is new king
		follow(ord.view); //follow the new king
	
	if (ord.seq >= log.size())//a new seq_num
		log[ord.seq] = ord;

	else{//received same seq before,
		if (ord.view < log[ord.seq].view) //expired order
			return 0; //ignore the order

		else if ((ord.view == log[ord.seq].view)){
			assert(ord.req == log[ord.seq].req);// ?? NEED TO DEF operater==() ?? 
			return 0;
		}
		else //(ord.view > log[ord.seq].view) //order from a newer king
		{ 
			
			log[ord.seq] = ord;
			cerf[ord.seq].clear();
			cerf[ord.seq].insert(id);

			/****OPTIMIZATION****/
			// log[ord.seq].view = ord.view;
			// if (log[ord.seq].req != ord.req)// ?? NEED TO DEF operater!=() ?? 
			// {
			// 	log[ord.seq] = ord;
			// 	cerf[ord.seq].clear();
			// 	cerf[ord.seq].insert(id);
			// }
			/********************/
		}	
	}

	//broadcast to other replicas
		...
}

int paxos_replica::learn(const order_t& ord, int source) //on receiving ACCEPT
{
	if (ord.view < log[ord.seq].view) //expired order
		return 0; //ignore the order
	if (ord.seq < exe_end)//executed
		return 0;//ignore
	accept(ord);
	certf[ord.seq].insert(source);//source should not be self, even it is, doesn't matter
	if (certf[ord.seq].size() == f+1 && ord.seq == exe_end)
		process();
}

int paxos_replica::process()
{
	for (int seq = exe_end; seq < log.size(); ++seq)
	{
		if (certf[seq].size() >= f+1){
			exec(log[seq]);
			reply(log[seq]);
			exe_end++;
		}
		else
			break;
	}
}

//REQUESTDONE:<king>
int paxos_replica::reply()
{

}


int paxos_replica::coup(){
	my_king = id;
	couping = 1;//myself
	// pending_req.clear();
	//broadcast OLDKINGISDEAD
	...
}

//OLDKINGISDEAD:<new_king>
int paxos_replica::follow() //on receiving OLDKINGISDEAD
{
	if (couping > 0){ //there is a newer king than me, I shall stop couping
		couping = 0;
		pending_req.clear();
	}
	string str;
	str += "LONGLIVETHEKING:"+to_string(id);
	for (auto i = log.begin(); i != log.end(); ++i){
		str+=":HIST:"+i->str();
	}
	//send str to new king
	...
}

int paxos_replica::admit() //on receiving LONGLIVETHEKING
{
	//
}



//<view>:<seq>:requset_string
order_t::order_t(const string& str)
{
	int pos0 = str.find(":");
	view = stoi(str.substr(0,pos0));

	int pos1 = str.find(":", pos0+1);
	seq = stoi(str.substr(pos0+1, pos1-pos0-1));

	req = request_t(str.substr(pos1+1))
}
string order_t::str()
{
	return to_string(view) +":"+ to_string(seq) +":"+ req.str();
}

//<client_id>:<client_seq>:<client_ip>:<client_port>:msg
request_t::request_t(const string& str)
{
	int pos0 = str.find(":");
	client_id = stoi(str.substr(0,pos0));

	int pos1 = str.find(":", pos0+1);
	client_seq = stoi(str.substr(pos0+1, pos1-pos0-1));

	int pos2 = str.find(":", pos1+1);
	client_ip_str = str.substr(pos1+1, pos2-pos1-1);

	int pos3 = str.find(":", pos2+1);
	client_port = stoi(str.substr(pos2+1, pos3-pos2-1));

	int pos4 = str.find(":", pos3+1);
	msg = str.substr(pos4+1);
}
string request_t::str()
{
	return to_string(client_id) +":"+ to_string(client_seq) +":"+ 
		client_ip_str +":"+ to_string(client_port) +":"+ msg;
}

