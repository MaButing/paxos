# paxos
A simple paxos implementation

Coder: Buting Ma

### USAGE:

```$client <client_id> <client_seq> <client_ip> <client_port>```

* \<client_id\> would be inversed in communicator: comm_id = -client_id; <br> when server call comm_recv, source_id = -client_id;
* \<clien_seq\> is where you want to start client_seq number, default should be 0;
* \<client_ip\> is a string of client's ip address (only support ipv4), e.g. "127.0.0.1";
* \<client_port\> is the port number on which the client receives responses.

EXAMPLE: ```$client 1 0 "127.0.0.1" 6666```
<br>
Follow the instructions to input the number of replicas, and their addresses.

```$replica <n> <f> <id> <x>```

* \<n\> is total number of replica, should be greater than 2f;
* \<f\> is the number of failure can be tolerated in the system;
* \<id\> is the id of replica, should be 0 ~ n-1, on start, replica 0 is the primary;
* \<x\> is the parameter for skipping x slots in log, the primary will forge certifications locally, <br>so it wouldn't be stucked in processing

EXAMPLE: ```./replica 3 1 0 0```
<br>
Follow the instructions to input the replicas' addresses.
The log file of replica (execution result) is named as ChatLog_\<id\>.txt


### Comment:

The only inconsistentency in ChatLog could be the postfix NOOPs. <BR>
Beacause of the skipping mechanism, other replicas don't know their existence.<br>
e.g.<br>
<br>
========replica 0 log=======<br>
REQ:1:0:127.0.0.1:6666:abc<br>
NOOP<br>
NOOP<br>
REQ:1:1:127.0.0.1:6666:def<br>
NOOP```*not in other logs*```<br>
NOOP```*not in other logs*```<br>
 ``` *!!!CRASH!!!* ```<br>
<br>
========replica 1 log=======<br>
REQ:1:0:127.0.0.1:6666:abc<br>
NOOP<br>
NOOP<br>
REQ:1:1:127.0.0.1:6666:def<br>
```*VIEW CHANGE(new primary)*```<br>
REQ:1:2:127.0.0.1:6666:ijk<br>
REQ:1:3:127.0.0.1:6666:xyz<br>
REQ:1:4:127.0.0.1:6666:hhh<br>
<br>
========replica 2 log=======<br>
REQ:1:0:127.0.0.1:6666:abc<br>
NOOP<br>
NOOP<br>
REQ:1:1:127.0.0.1:6666:def<br>
```*VIEW CHANGE*```<br>
REQ:1:2:127.0.0.1:6666:ijk<br>
REQ:1:3:127.0.0.1:6666:xyz<br>
REQ:1:4:127.0.0.1:6666:hhh<br>
