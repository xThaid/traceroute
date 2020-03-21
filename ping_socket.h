#include "traceroute.h"

#include <string>
#include <chrono>
#include <arpa/inet.h>

struct PingResponse {
	int type;
	std::string ip_str;
	int id;
	int sequence;
	std::chrono::system_clock::time_point receiveTime;
};

class PingSocket
{
public:
	PingSocket(int pid, std::string ip);

	void sendPing(short seq, int ttl);
	bool tryReadResponse(PingResponse* resp);
	int waitForResponses(timeval* tv);

	std::string getIP();

private:
	int pid;

	int sockfd;
	std::string ip;
	sockaddr_in recipient;
};
