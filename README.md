# paxos
a simple paxos implementation<br>
<br>
USAGE:<br>
$client \<client_id\> \<client_seq\> \<client_ip\> \<client_port\> <br>
	\<client_id\> would be inversed in communicator: comm_id = -client_id;<br>
		when server call comm_recv, source_id = -client_id;<br>
	\<clien_seq\> is where you want to start client_seq number, default should be 0;<br>
	\<client_ip\> is a string of client's ip address (only support ipv4), e.g. "127.0.0.1";<br>
	\<client_port\> is the port number on which the client receives responses.<br>
EXAMPLE: ./client 1 0 "127.0.0.1" 6666<br>
follow the instructions to input the number of replicas, and their addresses.<br>
<br>
$replica \<n\> \<f\> \<id\> \<x\><br>
	\<n\> is total number of replica, should be greater than 2f;<br>
	\<f\> is the number of failure can be tolerated in the system;<br>
	\<id\> is the id of replica, should be 0 ~ n-1, on start, replica 0 is the primary;<br>
	\<x\> is the parameter for skipping x slots in log, <br>
		the primary will forge certifications locally, so it wouldn't be stucked in processing<br>
EXAMPLE: ./replica 3 1 0 0<br>
follow the instructions to input the replicas' addresses.<br>
the log file of replica (execution result) is named as Log_\<id\>.txt<br>
