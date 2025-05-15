#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

/**
 * @brief Structure to hold arguments for port scanning threads.
 *
 * This structure is passed to each scanning thread and contains:
 * - The target server's address information
 * - The port number to scan
 * - The original hostname or IP address as a string for logging
 */
typedef struct port_scan_thread_args_struct
{
    struct sockaddr_in server;
    uint16_t port;
    char hostname[64];
} port_scan_args;

/**
 * @brief Resolves a hostname to an IPv4 address
 *
 * Uses getaddrinfo to resolve the specified hostname into an IPv4 address.
 * If multiple addresses are returned, it selects the first and prints a warning.
 *
 * @param address: Reference to an in_addr struct that will be filled with the resolved IP address
 * @param hostname: The hostname to resolve
 * @return int: 0 on success, or -1 if resolution fails
 */
int get_ip_addr(struct in_addr &address, std::string &hostname);

/**
 * @brief Thread function to check if a specific port is open on a host.
 *
 * @param params_in Pointer to port_scan_args struct with target IP and port.
 * @return void* Always returns NULL. Terminates the thread.
 */
void *check_port(void *params_in);

/**
 * @brief Performs a vanilla TCP scan on a host.
 *
 * @param hostname Target hostname or IP address.
 * @param port Specific port to scan. If negative, scans all ports (1–65535).
 */
void vanilla_scan(std::string &hostname, int port);

/**
 * @brief Thread function used for sweep scan. Checks port 5000 on a single host.
 *
 * @param params_in Pointer to port_scan_args struct containing target host and port.
 * @return void* Always returns NULL. Terminates the thread.
 */
void *check_port_in_sweep(void *params_in);

/**
 * @brief Performs a sweep scan on a list of hostnames/IPs, checking port 5000.
 *
 * @param hostnames Vector of hostnames or IPs to scan.
 */
void sweep_scan(std::vector<std::string> &hostnames);

/**
 * @brief Performs a sweep scan using a wildcard IPv4 address.
 *
 * Wildcard format example: "192.168.1.*", "*.168.*.*", etc.
 * Each wildcard octet expands into the range [0–255].
 *
 * @param hostname A string representing a wildcard IPv4 address.
 */
void sweep_scan_from_wildcard(std::string hostname);