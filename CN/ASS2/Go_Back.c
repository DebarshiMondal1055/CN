#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int isLost(float loss_probability) {
    return ((float)rand() / RAND_MAX) < loss_probability;
}

void delay(int ms) {
    usleep(ms * 1000);
}

void nonNACK_GBN(int total_frames, int window_size, float loss_probability) {
    printf("\n[1] Go-Back-N: Non-NACK Based Communication\n");
    int base = 0, nextSeq = 0;

    while (base < total_frames) {
        while (nextSeq < base + window_size && nextSeq < total_frames) {
            printf("Sender: Sending Frame %d\n", nextSeq);
            nextSeq++;
        }

        for (int i = base; i < nextSeq; i++) {
            delay(500);
            if (!isLost(loss_probability)) {
                printf("Receiver: Received Frame %d, Sending ACK\n", i);
                base++;
            } else {
                printf("Receiver: Frame %d lost or ACK lost\n", i);
                printf("Sender: Timeout -> Resending from Frame %d\n", base);
                nextSeq = base;
                break;
            }
        }
    }
}

void nack_GBN(int total_frames, int window_size, float loss_probability) {
    printf("\n[2] Go-Back-N: NACK Based Communication\n");
    int base = 0, nextSeq = 0;

    while (base < total_frames) {
        while (nextSeq < base + window_size && nextSeq < total_frames) {
            printf("Sender: Sending Frame %d\n", nextSeq);
            nextSeq++;
        }

        for (int i = base; i < nextSeq; i++) {
            delay(500);
            if (!isLost(loss_probability)) {
                printf("Receiver: Received Frame %d, Sending ACK\n", i);
                base++;
            } else {
                printf("Receiver: Frame %d lost, Sending NACK\n", i);
                printf("Sender: Received NACK -> Resending from Frame %d\n", i);
                base = i;
                nextSeq = base;
                break;
            }
        }
    }
}

void piggybacked_GBN(int total_frames, int window_size, float loss_probability) {
    printf("\n[3] Go-Back-N: Piggybacked Communication (Bi-directional)\n");
    int baseA = 0, nextA = 0;
    int baseB = 0, nextB = 0;

    while (baseA < total_frames || baseB < total_frames) {
        printf("\n-- A to B Transmission --\n");
        while (nextA < baseA + window_size && nextA < total_frames) {
            printf("A: Sending Frame %d", nextA);
            if (baseB < total_frames) {
                printf(" [Piggybacked ACK for B's Frame %d]", baseB);
                baseB++;
            }
            printf("\n");
            nextA++;
        }

        for (int i = baseA; i < nextA; i++) {
            delay(500);
            if (!isLost(loss_probability)) {
                printf("B: Received Frame %d from A, Sending ACK\n", i);
                baseA++;
            } else {
                printf("B: Frame %d from A lost\n", i);
                nextA = baseA;
                break;
            }
        }

        printf("\n-- B to A Transmission --\n");
        while (nextB < baseB + window_size && nextB < total_frames) {
            printf("B: Sending Frame %d", nextB);
            if (baseA < total_frames) {
                printf(" [Piggybacked ACK for A's Frame %d]", baseA);
                baseA++;
            }
            printf("\n");
            nextB++;
        }

        for (int i = baseB; i < nextB; i++) {
            delay(500);
            if (!isLost(loss_probability)) {
                printf("A: Received Frame %d from B, Sending ACK\n", i);
                baseB++;
            } else {
                printf("A: Frame %d from B lost\n", i);
                nextB = baseB;
                break;
            }
        }
    }
}

int main() {
    srand(time(NULL));
    int choice, total_frames, window_size;
    float loss_probability;

    printf("Go-Back-N Protocol Simulation in C\n");
    printf("Enter total number of frames: ");
    scanf("%d", &total_frames);
    printf("Enter window size: ");
    scanf("%d", &window_size);
    printf("Enter loss probability (0.0 to 1.0): ");
    scanf("%f", &loss_probability);

    if (total_frames <= 0 || window_size <= 0 || loss_probability < 0.0 || loss_probability > 1.0) {
        printf("Invalid input parameters\n");
        return 1;
    }

    printf("1. Non-NACK Based\n2. NACK Based\n3. Piggybacked (Bi-directional)\nEnter your choice: ");
    scanf("%d", &choice);

    switch (choice) {
        case 1:
            nonNACK_GBN(total_frames, window_size, loss_probability);
            break;
        case 2:
            nack_GBN(total_frames, window_size, loss_probability);
            break;
        case 3:
            piggybacked_GBN(total_frames, window_size, loss_probability);
            break;
        default:
            printf("Invalid choice!\n");
    }

    return 0;
}