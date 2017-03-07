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
	order.seq = log.size();
	order.view = my_king; //my king should be myself
	order.req = req;

	log.push_back(order);

	//propose to every other replica, 
	...

}

bool paxos_replica::req_exist(const request_t& req){
	//go through all the log

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


int paxos_replica::coup(){
	my_king = id;
	couping = 1;//myself
	// pending_req.clear();
	//broadcast OLDKINGISDEAD
	...
}

int paxos_replica::follow() //on receiving OLDKINGISDEAD
{
	if (couping > 0){ //there is a newer king than me, I shall stop couping
		couping = 0;
		pending_req.clear();
	}
}

int paxos_replica::admit() //on receiving LONGLIVETHEKING
{
	//
}



order_t paxos_replica::order_parse(const string& str)
{

	assert(view < id); //no one after me can be a king if I am alive.
}



request_t paxos_replica::req_parse(const string& str);