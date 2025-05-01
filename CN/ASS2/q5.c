#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

void delay(int ms) {
    usleep(ms);
}

typedef struct {
    int ack_no;
    int advertised_window;  // This field tells sender how many frames receiver can currently accept
} ACK;

int isLost(float loss_probability) {
    return ((float)rand() / RAND_MAX) < loss_probability;
}

ACK receiveFrame(int frame, int* receiver_buffer, int max_buffer_size, int* buffer_occupancy) {
    ACK ack;

    if (!isLost(frame)) {
        if (*buffer_occupancy < max_buffer_size) {
            printf("Receiver: Received Frame %d\n", frame);
            receiver_buffer[frame % max_buffer_size] = 1;
            (*buffer_occupancy)++;
        } else {
            printf("Receiver: Buffer full! Cannot accept Frame %d\n", frame);
        }
    } else {
        printf("Receiver: Frame %d lost\n", frame);
    }

    ack.ack_no = frame;
    ack.advertised_window = max_buffer_size - *buffer_occupancy;
    return ack;
}

void optimizedSlidingWindowProtocol(int total_frames, int max_seq, float loss_probability) {
    // Calculate MAX_BUFFER_SIZE: Assume MAX_SEQ = 2^m - 1, so MAX_BUFFER_SIZE = 2^(m-1)
    int m = (int)ceil(log2(max_seq + 1));
    int max_buffer_size = 1 << (m - 1);

    // Dynamically allocate receiver_buffer
    int* receiver_buffer = (int*)calloc(max_buffer_size, sizeof(int));
    if (!receiver_buffer) {
        printf("Memory allocation failed\n");
        return;
    }
    int buffer_occupancy = 0;

    int base = 0;
    int next_frame = 0;
    int sender_window = max_buffer_size;

    while (base < total_frames) {
        int send_limit = base + sender_window;

        while (next_frame < send_limit && next_frame < total_frames) {
            printf("Sender: Sending Frame %d\n", next_frame);
            delay(200);
            ACK ack = receiveFrame(next_frame, receiver_buffer, max_buffer_size, &buffer_occupancy);

            if (!isLost(loss_probability)) {
                if (ack.advertised_window > 0) {
                    printf("Sender: Received ACK for Frame %d | Receiver Window Available: %d\n", ack.ack_no, ack.advertised_window);
                } else {
                    printf("Sender: Received ACK for Frame %d | Receiver Buffer FULL!\n", ack.ack_no);
                }

                base++;
                next_frame++;
                sender_window = ack.advertised_window;

                // Simulate frame delivery at app layer to free buffer
                if (buffer_occupancy > 0 && rand() % 2 == 0) {
                    printf("Receiver: Delivered frame to app, freeing buffer.\n");
                    buffer_occupancy--;
                }
            } else {
                printf("Sender: ACK for Frame %d lost. Retrying...\n", next_frame);
                // Do not increment next_frame, resend
            }
        }

        if (sender_window == 0) {
            printf("Sender: Waiting... Receiver Buffer Full.\n");
            delay(500);  // simulate wait time
            sender_window = 1; // give another try
        }
    }
    printf("\nAll frames transmitted successfully!\n");

    // Free allocated memory
    free(receiver_buffer);
}

int main() {
    srand(time(NULL));
    int total_frames, max_seq;
    float loss_probability;

    printf("Optimized Sliding Window Protocol Simulation (With Buffer Feedback)\n");
    printf("Enter total number of frames: ");
    scanf("%d", &total_frames);
    printf("Enter maximum sequence number (MAX_SEQ): ");
    scanf("%d", &max_seq);
    printf("Enter loss probability (0.0 to 1.0): ");
    scanf("%f", &loss_probability);

    // Validate inputs
    if (total_frames <= 0) {
        printf("Invalid input: Total frames must be positive.\n");
        return 1;
    }
    if (max_seq < 1) {
        printf("Invalid input: MAX_SEQ must be at least 1.\n");
        return 1;
    }
    if (loss_probability < 0.0 || loss_probability > 1.0) {
        printf("Invalid input: Loss probability must be between 0.0 and 1.0.\n");
        return 1;
    }

    optimizedSlidingWindowProtocol(total_frames, max_seq, loss_probability);
    return 0;
}