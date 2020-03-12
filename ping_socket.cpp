#include "ping_socket.h"

#include <netinet/ip_icmp.h>

#include <cstring>
#include <cerrno>

static icmphdr create_icmp_echo_header(uint16_t id, uint16_t seq);
static uint16_t compute_icmp_checksum(const void* buff, int length);
struct icmphdr* get_icmphdr_from_packet(uint8_t* packet);

PingSocket::PingSocket(int pid, std::string ip) : pid(pid), ip(ip) {
	memset(&recipient, 0, sizeof(sockaddr_in));
	recipient.sin_family = AF_INET;
	if (!inet_pton(AF_INET, ip.c_str(), &recipient.sin_addr))
		app_error("%s isn't valid ip address", ip.c_str());

	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0)
		app_error("socket error: %s", strerror(errno));
}

void PingSocket::sendPing(short seq, int ttl) {
    icmphdr header = create_icmp_echo_header(pid, seq);
	setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));
	ssize_t bytes_sent = sendto(sockfd, &header, sizeof(header), 0, (sockaddr*)&recipient, sizeof(sockaddr_in));
    if (bytes_sent < 0)
		app_error("sendto error: %s", strerror(errno));
}

int PingSocket::waitForResponses(timeval* tv) {
	fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
	int retval = select(sockfd + 1, &rfds, nullptr, nullptr, tv);
	if (retval < 0)
		app_error("select error: %s", strerror(errno));
	return retval;
}

bool PingSocket::tryReadResponse(PingResponse* resp) {
	sockaddr_in sender;	
	socklen_t sender_len = sizeof(sender);
	uint8_t buffer[IP_MAXPACKET];

	ssize_t packet_len = recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr*)&sender, &sender_len);
	if (packet_len < 0)
		app_error("recvfrom error: %s", strerror(errno));

	struct icmphdr* icmp_header = get_icmphdr_from_packet(buffer);

	resp->type = icmp_header->type;
	char sender_ip_str[20]; 
	inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_str, sizeof(sender_ip_str));
	resp->ip_str = std::string(sender_ip_str);

	if (resp->type != ICMP_ECHOREPLY && resp->type != ICMP_TIME_EXCEEDED)
		return false;
	if (resp->type == ICMP_TIME_EXCEEDED)
		icmp_header = get_icmphdr_from_packet(((uint8_t*) icmp_header) + 8);

	resp->id = htons(icmp_header->un.echo.id);
	resp->sequence = htons(icmp_header->un.echo.sequence);

	if (resp->id != pid)
		return false;

	resp->receiveTime = std::chrono::system_clock::now();

	return true;
}

std::string PingSocket::getIP() {
	return ip;
}

struct icmphdr* get_icmphdr_from_packet(uint8_t* packet) {
	ip* ip_header = (struct ip*) packet;
	icmphdr* icmp_header = (struct icmphdr*) (packet + 4 * ip_header->ip_hl);
	return icmp_header;
}

static icmphdr create_icmp_echo_header(uint16_t id, uint16_t seq) {
	icmphdr header;
	header.type = ICMP_ECHO;
    header.code = 0;
    header.un.echo.id = ntohs(id);
    header.un.echo.sequence = ntohs(seq);
	header.checksum = 0;
	header.checksum = compute_icmp_checksum((uint16_t *) &header, sizeof(header));
	return header;
}

static uint16_t compute_icmp_checksum(const void* buff, int length) {
	uint32_t sum;
	const uint16_t* ptr = (const uint16_t*) buff;
	for (sum = 0; length > 0; length -= 2)
		sum += *ptr++;
	sum = (sum >> 16) + (sum & 0xffff);
	return (uint16_t)(~(sum + (sum >> 16)));
}