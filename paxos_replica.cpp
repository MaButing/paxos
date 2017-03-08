#include "paxos_replica.h"
#include <cassert>
#include <fstream>

using std::ofstream;



int paxos_replica::repl_init(const vector<sockaddr_in>& addr_list_in)
{
	if (comm.comm_init(addr_list_in) != 0)
		return -1;

	if (n >= 3*f+1)
		return 0;
	else
		return -1;
}


int paxos_replica::repl_close()
{
	return comm.comm_close();
}

int paxos_replica::repl_run()
{
	char buff[MAXSENDSIZE];
	while(1){
		int source_id;
		comm.comm_recv(&source_id, buff, MAXSENDSIZE);
		string str(buff);
		size_t pos0 = str.find(":");
		string type = str.substr(0, pos0);
		//CLIENTREQ:request_str
		if (type == "CLIENTREQ"){
			string req_str = str.substr(pos0+1);
			request_t req(req_str);
			assert(source_id == -1*req.client_id);
			recv_req(req);

		}
		//PROPOSAL:<king>:order_str
		else if (type == "PROPOSAL"){
			size_t pos1 = str.find(":", pos0+1);
			int king = stoi(str.substr(pos0+1, pos1-pos0-1));
			string ord_str = str.substr(pos1+1);
			order_t ord(ord_str);
			assert(king == source_id);
			assert(king == ord.view);
			accept(ord, true);
		}
		//ACCEPT:<accepter>:order_str
		else if (type == "ACCEPT"){
			size_t pos1 = str.find(":", pos0+1);
			int accepter = stoi(str.substr(pos0+1, pos1-pos0-1));
			string ord_str = str.substr(pos1+1);
			order_t ord(ord_str);
			learn(ord, accepter);
		}
		//OLDKINGISDEAD:<new_king>:<reprop_begin>
		else if (type == "OLDKINGISDEAD"){
			size_t pos1 = str.find(":", pos0+1);
			int new_king = stoi(str.substr(pos0+1, pos1-pos0-1));
			size_t pos2 = str.find(":", pos1+1);
			int reprop_begin = stoi(str.substr(pos1+1, pos2-pos1-1));
			follow(new_king, reprop_begin);
		}
		//LONGLIVETHEKING:<new_king>:<follower>:{HIST:order_str}
		else if (type == "LONGLIVETHEKING"){
			size_t pos1 = str.find(":", pos0+1);
			int new_king = stoi(str.substr(pos0+1, pos1-pos0-1));
			assert(new_king == id);
			size_t pos2 = str.find(":", pos1+1);
			int follower = stoi(str.substr(pos1+1, pos2-pos1-1));
			assert(follower == source_id);

			vector<order_t> hist;
			size_t hist_end = 0;
			size_t hist_begin = str.find("HIST:", hist_end);
			while(hist_begin != string::npos){
				hist_end = str.find("HIST:", hist_begin+5);
				string ord_str = str.substr(hist_begin+5, hist_end - hist_begin - 5);
				hist.push_back(order_t(ord_str));

				hist_begin = hist_end;
			}

			admit(new_king, follower, hist);
			
		}

		else 
			continue; //ignore
	}
}













//CLIENTREQ:request_str
int paxos_replica::recv_req(const request_t& req)// on receiving a client request
{
	//I am not the king, but I shall be the king, because client(god) want me to.
	if (my_king < id)
		coup();

	if (!couping.empty()){
		pending_req.push_back(req);
		return 0;
	}

	if (req_exist(req)) //this request has been assigned a seq num
		return 0;
	
	//create a new log record
	order_t ord;
	ord.seq = log.size()+x;//assign new seq_num
	ord.view = my_king; //my king should be myself
	ord.req = req;

	propose(ord, true);
	return 0;
}


bool paxos_replica::req_exist(const request_t& req)
{
	//go through all the log
	for (auto i = log.begin(); i != log.end(); ++i)
	{
		if (i->req == req)
			return true;
	}
	return false;
}

int paxos_replica::propose(order_t& ord, bool bcast)
{
	
	assert(ord.view >= log[ord.seq].view);//should be a newer older
	assert(my_king >= ord.view);// before propose ord, I should updated my view
	assert(my_king == id);// I can propose iff I am king

	ord.view = my_king;
	log[ord.seq] = ord;
	
	if (bcast){//broadcast to other replicas
		string str = "PROPOSAL:"+to_string(id)+":"+ord.str();
		for (int i = 0; i < n; ++i){
			comm.comm_send(i, (void*)str.c_str(), str.size()+1);
		}
	}

	return 0;
}

//PROPOSAL:<king>:order_str
int paxos_replica::accept(const order_t& ord, bool bcast) //on receiving a PROPOSAL
{
	if (ord.view < my_king)//from a old/dead king
		return 0;//ignore
	if (ord.seq < exe_end)//executed
		return 0;//ignore

	if (ord.view > my_king) //there is new king
		follow(ord.view, reprop_begin); //follow the new king
	
	if (ord.seq >= (int)log.size())//a new seq_num
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
			certf[ord.seq].clear();
			certf[ord.seq].insert(id);

			/****OPTIMIZATION****/
			// log[ord.seq].view = ord.view;
			// if (log[ord.seq].req != ord.req)// ?? NEED TO DEF operater!=() ?? 
			// {
			// 	log[ord.seq] = ord;
			// 	certf[ord.seq].clear();
			// 	certf[ord.seq].insert(id);
			// }
			/********************/
		}	
	}

	if (bcast){//broadcast ACCEPT to other replicas
		string str = "ACCEPT:"+to_string(id)+":"+ord.str();
		for (int i = 0; i < n; ++i){
			comm.comm_send(i, (void*)str.c_str(), str.size()+1);
		}
	}

	return 0;
}

//ACCEPT:<accepter>:order_str
int paxos_replica::learn(const order_t& ord, int accepter) //on receiving ACCEPT
{
	if (ord.view < log[ord.seq].view) //expired order
		return 0; //ignore the order
	if (ord.seq < exe_end)//executed
		return 0;//ignore
	accept(ord, true);
	certf[ord.seq].insert(accepter);//accepter should not be self, even it is, doesn't matter
	if (certf[ord.seq].size() == (size_t)f+1 && ord.seq == exe_end)
		process();

	return 0;
}

int paxos_replica::process()
{
	for (int seq = exe_end; seq < (int)log.size(); ++seq)
	{
		if (certf[seq].size() >= (size_t)f+1){
			exec(log[seq]);
			reply(log[seq].req);//if I am the king
			exe_end++;
		}
		else
			break;
	}
	return 0;
}

int paxos_replica::exec(const order_t& ord)
{
	string log_name = "Log_"+to_string(id)+".txt";
	ofstream ofs(log_name.c_str(), std::ofstream::out);
	ofs << ord.req.str() << endl;
	ofs.close();
	return 0;
}

//REQUESTDONE:<king>:<client_id>:<client_seq>
int paxos_replica::reply(const request_t& req)
{
	sockaddr_in clientaddr;
	memset(&clientaddr, 0, sizeof(sockaddr_in));
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_port = htons(req.client_port);
	inet_pton(AF_INET, req.client_ip_str.c_str(), &(clientaddr.sin_addr));

	int sock_send = socket(AF_INET, SOCK_STREAM, 0);
    if( sock_send < 0){
        cerr << "[Error, create sending socket]: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
        return -1;
    }
    if( connect(sock_send, (sockaddr*)&clientaddr, sizeof(sockaddr)) < 0){
        cerr << "[Error, connect to"<<req.client_id<<"]: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
        close(sock_send);
        return -1;
    }
	//send
    //REQUESTDONE:<king>:<client_id>:<client_seq>
    string str = "REQUESTDONE:"+to_string(my_king)
    	+":"+to_string(req.client_id)+":"+to_string(req.client_seq);
    //send msg_len
    size_t msg_len = str.size()+1+sizeof(size_t)+sizeof(int);
    int send_len = send(sock_send, &msg_len, sizeof(size_t), MSG_NOSIGNAL);
    if (send_len < 0){
        cerr << "[send, send msg_size error]: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
        return -1;
    }
    //send source_id
    send_len = send(sock_send, &id, sizeof(int), MSG_NOSIGNAL);
    if (send_len < 0){
        cerr << "[send, send source_id error]: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
        return -1;
    }

    send_len = send(sock_send, str.c_str(), str.size()+1, MSG_NOSIGNAL);
    if (send_len < 0){
        cerr << "[send, send msg error]: "<<strerror(errno)<<"(errno: "<<errno<<")"<<endl;
        return -1;
    }
    
	close(sock_send);
	return 0;
}


int paxos_replica::coup()
{
	my_king = id;
	couping.insert(id);//myself
	// pending_req.clear();

	//OPTIMIZATION
	//reprop_begin = exe_end;

	//broadcast OLDKINGISDEAD
	string str = "OLDKINGISDEAD:"+to_string(my_king)+":"+to_string(reprop_begin);
	for (int i = 0; i < n; ++i){
		comm.comm_send(i, (void*)str.c_str(), str.size()+1);
	}

	return 0;
}

//OLDKINGISDEAD:<new_king>:<reprop_begin>
int paxos_replica::follow(int new_king, int reprop_begin) //on receiving OLDKINGISDEAD
{
	if (new_king < my_king)//from a old/dead king
		return 0;//ignore
	
	if (!couping.empty()){ //there is a newer king than me, I shall abandon couping
		couping.clear();
		pending_req.clear();
	}

	my_king = new_king;
	string str;
	str += "LONGLIVETHEKING:"+to_string(my_king)+to_string(id);
	
	for (auto i = log.begin(); i != log.end(); ++i){
		str+=":HIST:"+i->str();
	}
	/****OPTIMIZATION****/
	// int min = reprop_begin < exe_end? reprop_begin: exe_end;//smaller one
	// for (int i = min; i < log.size(); ++i)
	// {
	// 	str+=":HIST:"+i->str();
	// }
	/*******************/

	//send str to new king
	comm.comm_send(my_king, (void*)str.c_str(), str.size()+1);
	return 0;
}

//LONGLIVETHEKING:<new_king>:<follower>:{HIST:order_str}
int paxos_replica::admit(int new_king, int follower, const vector<order_t>& hist) //on receiving LONGLIVETHEKING
{
	if (couping.empty())//not couping
		return 0;//ignore
	if (new_king < my_king)//not follow me(my_king)
		return 0;
	couping.insert(follower);

	for (auto i = hist.begin(); i != hist.end(); ++i){
		/****OPTIMIZATION****/
		// if (i->seq < reprop_begin) 
		// 	reprop_begin = i->seq;
		/********************/
		accept(*i, false);
	}


	if (couping.size() >= (size_t)f+1){
		//finish couping;
		couping.clear();
		//repropose
		for (int i = 0; i < (int)log.size(); ++i){
			if (log[i].seq == -1){//empty slot
				log[i].seq = i;
				log[i].view = my_king;
			}
			else{
				assert(log[i].seq == i);
				log[i].view = my_king;//should be me
			}
			propose(log[i], true);
		}
		/****OPTIMIZATION****/
		// for (int i = reprop_begin; i < log.size(); ++i){
		// 	if (log[i].seq = -1){//empty slot
		// 		log[i].seq = i;
		// 		log[i].view = my_king;
		// 	}
		// 	else{
		// 		assert(log[i].seq == i);
		// 		log[i].view = my_king;//should be me
		// 	}
		// 	propose(log[i]);
		// }
		/*******************/
		
		//propose pending request
		for (auto i = pending_req.begin(); i != pending_req.end(); ++i){
			recv_req(*i);
		}
		pending_req.clear();

	}
	
	return 0;
}

