Three-Tier client-Server Application

By: Manuel Perez

Coded a simple three-tier application involving a client program,
two server programs, and a gateway program.

The applications use TCP sockets to communicate with each other.
Both the gateway and the servers use multithreading to handle
numerous requests.


Client:

The client program works on the user side by checking for the
appropriate format wanted from the user. The client program then
forwards information and the operation request to the gateway
program. Once it receives a response from the gateway it displays
the results to the user.

Gateway:

This program acts as both a client and a server. It acts as a
server by receiving some type of operation request from the
client. The gateway then analyses the request and sends it to
one of two server programs, acting as a client. When this program
receives a response from the servers it then forwards it back to
the appropriate client.

Servers:

Both server programs receive requests from the gateway and
perform an operation on the received strings. The servers then
forward the modified strings back to the gateway.




