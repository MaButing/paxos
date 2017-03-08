#include <string>


using std::string;
using std::stoi;
using std::to_string;


struct request_t
{
	int client_id;
	int client_seq;
	string client_ip_str;
	int client_port;
	string msg;

	request_t():client_id(-1),client_seq(-1),client_port(-1){};
	request_t(const string& str);
	string str();
};


struct order_t
{
	int view;
	int seq;
	request_t req;

	order_t():view(-1),seq(-1){};
	order_t(const string& str);
	string str();
};




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




//<view>:<seq>:requset_string
order_t::order_t(const string& str)
{
	int pos0 = str.find(":");
	view = stoi(str.substr(0,pos0));

	int pos1 = str.find(":", pos0+1);
	seq = stoi(str.substr(pos0+1, pos1-pos0-1));

	req = request_t(str.substr(pos1+1));
}
string order_t::str()
{
	return to_string(view) +":"+ to_string(seq) +":"+ req.str();
}
