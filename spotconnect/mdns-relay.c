#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define MDNS_GROUP "224.0.0.251"
#define MDNS_PORT 5353
#define PACKET_SIZE 9000

static volatile sig_atomic_t running = 1;

static void stop(int signal_number) {
    (void)signal_number;
    running = 0;
}

static bool is_local_address(struct in_addr address) {
    struct ifaddrs *interfaces = NULL;
    bool local = false;

    if (getifaddrs(&interfaces) != 0) {
        return false;
    }

    for (struct ifaddrs *item = interfaces; item != NULL; item = item->ifa_next) {
        if (item->ifa_addr == NULL || item->ifa_addr->sa_family != AF_INET) {
            continue;
        }
        const struct sockaddr_in *candidate =
            (const struct sockaddr_in *)item->ifa_addr;
        if (candidate->sin_addr.s_addr == address.s_addr) {
            local = true;
            break;
        }
    }

    freeifaddrs(interfaces);
    return local;
}

static bool contains_raop_question(const uint8_t *packet, size_t size) {
    static const uint8_t raop_name[] = {
        5, '_', 'r', 'a', 'o', 'p',
        4, '_', 't', 'c', 'p',
        5, 'l', 'o', 'c', 'a', 'l', 0,
    };

    if (size < sizeof(raop_name)) {
        return false;
    }
    for (size_t offset = 0; offset <= size - sizeof(raop_name); ++offset) {
        if (memcmp(packet + offset, raop_name, sizeof(raop_name)) == 0) {
            return true;
        }
    }
    return false;
}

static int join_local_interfaces(int socket_fd) {
    struct ifaddrs *interfaces = NULL;
    int memberships = 0;

    if (getifaddrs(&interfaces) != 0) {
        return 0;
    }

    for (struct ifaddrs *item = interfaces; item != NULL; item = item->ifa_next) {
        if (item->ifa_addr == NULL || item->ifa_addr->sa_family != AF_INET ||
            (item->ifa_flags & (IFF_UP | IFF_MULTICAST)) !=
                (IFF_UP | IFF_MULTICAST)) {
            continue;
        }

        struct ip_mreq membership = {0};
        membership.imr_multiaddr.s_addr = inet_addr(MDNS_GROUP);
        membership.imr_interface =
            ((const struct sockaddr_in *)item->ifa_addr)->sin_addr;
        if (setsockopt(socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &membership,
                       sizeof(membership)) == 0) {
            ++memberships;
        }
    }

    freeifaddrs(interfaces);
    return memberships;
}

int main(void) {
    int receive_fd = socket(AF_INET, SOCK_DGRAM, 0);
    int send_fd = socket(AF_INET, SOCK_DGRAM, 0);
    int enabled = 1;
    struct timeval receive_timeout = {.tv_sec = 1, .tv_usec = 0};
    struct sockaddr_in bind_address = {0};
    struct sockaddr_in browser = {0};
    bool browser_known = false;
    bool send_bound = false;
    bool reported_forward = false;
    bool reported_raop = false;
    bool debug = getenv("MDNS_RELAY_DEBUG") != NULL;
    uint8_t packet[PACKET_SIZE];

    if (receive_fd < 0 || send_fd < 0) {
        perror("mdns-relay: socket");
        return EXIT_FAILURE;
    }

    setsockopt(receive_fd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));
#ifdef SO_REUSEPORT
    setsockopt(receive_fd, SOL_SOCKET, SO_REUSEPORT, &enabled, sizeof(enabled));
#endif
    setsockopt(receive_fd, SOL_SOCKET, SO_RCVTIMEO, &receive_timeout,
               sizeof(receive_timeout));

    bind_address.sin_family = AF_INET;
    bind_address.sin_port = htons(MDNS_PORT);
    bind_address.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(receive_fd, (struct sockaddr *)&bind_address,
             sizeof(bind_address)) != 0) {
        perror("mdns-relay: bind");
        return EXIT_FAILURE;
    }

    int memberships = join_local_interfaces(receive_fd);
    if (memberships == 0) {
        fprintf(stderr, "mdns-relay: could not join an mDNS interface\n");
        return EXIT_FAILURE;
    }

    signal(SIGINT, stop);
    signal(SIGTERM, stop);
    fprintf(stderr, "mdns-relay: listening for same-host RAOP discovery\n");

    while (running) {
        struct sockaddr_in source = {0};
        socklen_t source_size = sizeof(source);
        ssize_t received = recvfrom(receive_fd, packet, sizeof(packet), 0,
                                    (struct sockaddr *)&source, &source_size);
        if (received < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            perror("mdns-relay: recvfrom");
            return EXIT_FAILURE;
        }
        if (received < 12) {
            continue;
        }

        bool response = (packet[2] & 0x80U) != 0;
        if (!response && ntohs(source.sin_port) != MDNS_PORT &&
            is_local_address(source.sin_addr) &&
            contains_raop_question(packet, (size_t)received)) {
            if (debug && !browser_known) {
                fprintf(stderr,
                        "mdns-relay: captured the RAOP browser on UDP port "
                        "%u\n",
                        ntohs(source.sin_port));
            }
            if (!send_bound) {
                struct sockaddr_in send_address = {
                    .sin_family = AF_INET,
                    .sin_port = htons(0),
                    .sin_addr = source.sin_addr,
                };
                if (bind(send_fd, (struct sockaddr *)&send_address,
                         sizeof(send_address)) != 0) {
                    perror("mdns-relay: bind forwarding socket");
                    return EXIT_FAILURE;
                }
                send_bound = true;
                fprintf(stderr,
                        "mdns-relay: forwarding from the RAOP browser "
                        "address\n");
            }
            browser = source;
            browser_known = true;
            continue;
        }

        if (response && browser_known) {
            bool raop_response =
                contains_raop_question(packet, (size_t)received);
            if (sendto(send_fd, packet, (size_t)received, 0,
                       (struct sockaddr *)&browser, sizeof(browser)) < 0) {
                perror("mdns-relay: sendto");
                continue;
            }
            if (!reported_forward) {
                fprintf(stderr,
                        "mdns-relay: forwarding multicast responses to "
                        "SpotConnect\n");
                reported_forward = true;
            }
            if (debug && raop_response && received >= 512 && !reported_raop) {
                fprintf(stderr,
                        "mdns-relay: forwarded a complete RAOP response "
                        "(%zd bytes)\n",
                        received);
                reported_raop = true;
            }
        }
    }

    close(send_fd);
    close(receive_fd);
    return EXIT_SUCCESS;
}
