# traceroute
This is a simple version of traceroute tool. It doesn't resolve hostnames and operates only on IPv4.

## What is traceroute?
traceroute tracks the route packets taken from an IP network on their way to a given host. It utilizes the IP
protocol's time to live (TTL) field and attempts to elicit an ICMP TIME_EXCEEDED response from each gateway
along the path to the host.

## Building
Simply run
```
$> make
```
in project root.

## Usage
```
$> sudo ./traceroute [host]
```
This tool uses raw sockets, so it needs to be run with root privileges.

## Example
```
$> sudo ./traceroute 8.8.8.8
1. 192.168.0.1 4ms
2. 10.6.71.254 7ms
3. *
4. *
5. *
6. 108.170.250.209 10ms
7. 216.239.41.133 8ms
8. 8.8.8.8 11ms
```
