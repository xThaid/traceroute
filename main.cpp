#include "traceroute.h"
#include "ping_socket.h"

#include <cstring>
#include <cstdarg>
#include <vector>
#include <chrono>
#include <unistd.h>
#include <cstdio>
#include <set>

void app_error(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fputc('\n', stderr);
  va_end(ap);
  exit(EXIT_FAILURE);
}

static void printProbe(int ttl, std::chrono::system_clock::time_point sendTime, std::vector<PingResponse>& responses) {
	int timeSum = 0;
	std::set<std::string> hosts;
	for (PingResponse& resp : responses) {
		timeSum += std::chrono::duration_cast<std::chrono::milliseconds>(resp.receiveTime - sendTime).count();
		hosts.insert(resp.ip_str);
	}

	printf("%d. ", ttl);

	if (hosts.size() == 0) {
		printf("*\n");
		return;
	}

	for (std::string host : hosts)
		printf("%s ", host.c_str());

	if (responses.size() != PACKETS_PER_PROBE)
		printf("???\n");
	else
		printf("%dms\n", timeSum / PACKETS_PER_PROBE);
}

static bool sendProbe(PingSocket* sock, int ttl) {
	for (int i = 0; i < PACKETS_PER_PROBE; i++)
		sock->sendPing(PACKETS_PER_PROBE * ttl + i, ttl);
	auto sendTime = std::chrono::system_clock::now();

	timeval tv;
	tv.tv_sec = PING_TIMEOUT_SEC;
	tv.tv_usec = PING_TIMEOUT_USEC;

	bool reachedEnd = false;
	std::vector<PingResponse> responses;
	while(responses.size() != PACKETS_PER_PROBE) {
		int retval = sock->waitForResponses(&tv);
		if (retval == 0)
			break;

		PingResponse resp;
		if (!sock->tryReadResponse(&resp))
			continue;
		if (resp.sequence / 3 == ttl)
			responses.push_back(resp);
		
		if (resp.ip_str == sock->getIP())
			reachedEnd = true;
	}

	printProbe(ttl, sendTime, responses);

	return reachedEnd;
}

static void traceroute(std::string ip) {
	PingSocket pingSocket(getpid(), ip);
	for (int ttl = 1; ttl < MAX_TTL; ttl++) {
		if (sendProbe(&pingSocket, ttl))
			break;
	}
}

int main(int argc, char** argv) {
	if (argc != 2)
		app_error("usage: %s host", argv[0]);

	traceroute(std::string(argv[1]));
    return 0;
}