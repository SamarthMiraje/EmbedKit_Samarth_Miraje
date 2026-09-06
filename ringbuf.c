#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define BUFFER_SIZE         8U
#define BUFFER_MASK         (BUFFER_SIZE - 1U)

#define RINGBUF_OK          0
#define RINGBUF_ERR_FULL   -1
#define RINGBUF_ERR_EMPTY  -2

typedef struct {
    uint8_t buffer[BUFFER_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} RingBuffer_t;

void ringbuf_init(RingBuffer_t *rb) {
    if (rb == NULL) {
        return;
    }
    rb->head = 0U;
    rb->tail = 0U;
    rb->count = 0U;
}

bool ringbuf_is_full(const RingBuffer_t *rb) {
    return (rb != NULL) && (rb->count == BUFFER_SIZE);
}

bool ringbuf_is_empty(const RingBuffer_t *rb) {
    return (rb != NULL) && (rb->count == 0U);
}

uint8_t ringbuf_get_count(const RingBuffer_t *rb) {
    return (rb != NULL) ? rb->count : 0U;
}

int8_t ringbuf_write(RingBuffer_t *rb, uint8_t data) {
    if (rb == NULL || ringbuf_is_full(rb)) {
        return RINGBUF_ERR_FULL;
    }

    rb->buffer[rb->head] = data;
    rb->head = (uint8_t)((rb->head + 1U) & BUFFER_MASK);
    rb->count++;

    return RINGBUF_OK;
}

int8_t ringbuf_read(RingBuffer_t *rb, uint8_t *data) {
    if (rb == NULL || data == NULL || ringbuf_is_empty(rb)) {
        return RINGBUF_ERR_EMPTY;
    }

    *data = rb->buffer[rb->tail];
    rb->tail = (uint8_t)((rb->tail + 1U) & BUFFER_MASK);
    rb->count--;

    return RINGBUF_OK;
}

int main(void) {
    RingBuffer_t rb;
    ringbuf_init(&rb);

    const uint8_t initial_bytes[8] = {0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48};
    for (uint8_t i = 0; i < 8; i++) {
        uint8_t byte = initial_bytes[i];
        if (ringbuf_write(&rb, byte) == RINGBUF_OK) {
            if (ringbuf_is_full(&rb)) {
                printf("[WRITE] 0x%02X -> OK (count=%u) FULL\n", byte, ringbuf_get_count(&rb));
            } else {
                printf("[WRITE] 0x%02X -> OK (count=%u)\n", byte, ringbuf_get_count(&rb));
            }
        }
    }

    uint8_t overflow_byte = 0x99;
    if (ringbuf_write(&rb, overflow_byte) == RINGBUF_ERR_FULL) {
        printf("[WRITE] 0x%02X -> FAIL (buffer full)\n", overflow_byte);
    }

    uint8_t read_val = 0;
    for (uint8_t i = 0; i < 3; i++) {
        if (ringbuf_read(&rb, &read_val) == RINGBUF_OK) {
            printf("[READ]  -> 0x%02X (count=%u)\n", read_val, ringbuf_get_count(&rb));
        }
    }

    const uint8_t refill_bytes[3] = {0x49, 0x4A, 0x4B};
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t byte = refill_bytes[i];
        if (ringbuf_write(&rb, byte) == RINGBUF_OK) {
            if (ringbuf_is_full(&rb)) {
                printf("[WRITE] 0x%02X -> OK (count=%u) FULL\n", byte, ringbuf_get_count(&rb));
            } else {
                printf("[WRITE] 0x%02X -> OK (count=%u)\n", byte, ringbuf_get_count(&rb));
            }
        }
    }

    while (!ringbuf_is_empty(&rb)) {
        if (ringbuf_read(&rb, &read_val) == RINGBUF_OK) {
            printf("[READ]  -> 0x%02X (count=%u)\n", read_val, ringbuf_get_count(&rb));
        }
    }

    if (ringbuf_read(&rb, &read_val) == RINGBUF_ERR_EMPTY) {
        printf("[READ]  (empty) -> FAIL (buffer empty)\n");
    }

    return 0;
}
