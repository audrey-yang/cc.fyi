#include "port-scanner-lib.h"

int get_ip_addr(struct in_addr &address, std::string &hostname)
{
    struct addrinfo hints, *result, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    int status = getaddrinfo(hostname.c_str(), NULL, &hints, &result);
    if (status)
    {
        perror("getaddrinfo");
        return -1;
    }

    for (p = result; p != NULL; p = p->ai_next)
    {
        struct sockaddr_in *ai_addr = (struct sockaddr_in *)p->ai_addr;
        address.s_addr = ai_addr->sin_addr.s_addr;
        break;
    }

    freeaddrinfo(result);

    return 0;
}

void full_scan(std::string &hostname, int port)
{
    std::cout << "Scanning host: " << hostname << " ";
    if (port >= 0)
    {
        std::cout << "port: " << port;
    }
    std::cout << std::endl;

    struct in_addr server_in_addr;
    get_ip_addr(server_in_addr, hostname);

    if (port < 0)
    {
        pthread_t threads[65535];
        port_scan_args *args_list[65536];
        for (uint16_t i = 1; i != 0; i++)
        {
            port_scan_args *params = (port_scan_args *)malloc(sizeof(port_scan_args));
            params->server.sin_family = AF_INET;
            params->server.sin_addr.s_addr = server_in_addr.s_addr;
            params->port = i;
            args_list[i] = params;

            if (pthread_create(&threads[i - 1], NULL, check_port_vanilla, (void *)params))
            {
                std::cerr << "Error creating thread " << i - 1 << std::endl;
            }
        }
        for (uint16_t i = 1; i != 0; i++)
        {
            if (pthread_join(threads[i - 1], NULL))
            {
                std::cerr << "Error joining thread " << i - 1 << std::endl;
            }
            free(args_list[i]);
        }
    }
    else
    {
        port_scan_args params;
        params.server.sin_family = AF_INET;
        params.server.sin_addr.s_addr = server_in_addr.s_addr;
        params.port = port;
        check_port_vanilla(&params);
    }
}

void sweep_scan(std::vector<std::string> &hostnames)
{
    std::vector<pthread_t> threads;
    std::vector<port_scan_args *> args_list;
    for (int i = 0; i < hostnames.size(); i++)
    {
        auto &hostname = hostnames[i];
        port_scan_args *params = (port_scan_args *)malloc(sizeof(port_scan_args));
        params->server.sin_family = AF_INET;
        get_ip_addr(params->server.sin_addr, hostname);
        strcpy(params->hostname, hostname.c_str());
        params->port = 5000;

        pthread_t thread;
        if (pthread_create(&thread, NULL, check_port_in_sweep_vanilla, (void *)params))
        {
            std::cerr << "Error creating thread " << i << std::endl;
        }
        threads.push_back(thread);
        args_list.push_back(params);
    }

    for (int i = 0; i < hostnames.size(); i++)
    {
        if (pthread_join(threads[i], NULL))
        {
            std::cerr << "Error joining thread " << i << std::endl;
        }
        free(args_list[i]);
    }
}

void sweep_scan_from_wildcard(std::string hostname)
{
    // Process wildcard
    std::vector<std::string> octets;
    size_t start = 0;
    size_t end = hostname.find('.');

    while (end != std::string::npos)
    {
        octets.push_back(hostname.substr(start, end - start));
        start = end + 1;
        end = hostname.find('.', start);
    }
    octets.push_back(hostname.substr(start));

    int octet1_start = octets[0] == "*" ? 0 : std::stoi(octets[0]);
    int octet1_end = octets[0] == "*" ? 256 : octet1_start + 1;
    int octet2_start = octets[1] == "*" ? 0 : std::stoi(octets[1]);
    int octet2_end = octets[1] == "*" ? 256 : octet2_start + 1;
    int octet3_start = octets[2] == "*" ? 0 : std::stoi(octets[2]);
    int octet3_end = octets[2] == "*" ? 256 : octet3_start + 1;
    int octet4_start = octets[3] == "*" ? 0 : std::stoi(octets[3]);
    int octet4_end = octets[3] == "*" ? 256 : octet4_start + 1;

    std::vector<pthread_t> threads;
    std::vector<port_scan_args *> args_list;
    int num_threads = 0;
    for (; octet1_start < octet1_end; octet1_start++)
    {
        for (; octet2_start < octet2_end; octet2_start++)
        {
            for (; octet3_start < octet3_end; octet3_start++)
            {
                for (; octet4_start < octet4_end; octet4_start++)
                {
                    std::string ip_addr = std::to_string(octet1_start) + "." + std::to_string(octet2_start) +
                                          "." + std::to_string(octet3_start) + "." + std::to_string(octet4_start);

                    port_scan_args *params = (port_scan_args *)malloc(sizeof(port_scan_args));
                    strcpy(params->hostname, ip_addr.c_str());
                    params->server.sin_family = AF_INET;
                    inet_pton(AF_INET, ip_addr.c_str(), &params->server.sin_addr);
                    params->port = 5000;

                    pthread_t thread;
                    if (pthread_create(&thread, NULL, check_port_in_sweep_vanilla, (void *)params))
                    {
                        std::cerr << "Error creating thread " << num_threads << std::endl;
                    }
                    threads.push_back(thread);
                    args_list.push_back(params);
                    num_threads++;
                }
            }
        }
    }

    for (int i = 0; i < num_threads; i++)
    {
        if (pthread_join(threads[i], NULL))
        {
            std::cerr << "Error joining thread " << i << std::endl;
        }
        free(args_list[i]);
    }
}