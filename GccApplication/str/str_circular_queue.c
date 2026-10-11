/**
 * @file    str_circular_queue.c
 * @brief   Array-based circular FIFO queue implementation.
 * @author  eyads
 * @date    2026-05-07
 *
 * @note    Implements the public API declared in str_circular_queue.h.
 *          The queue stores up to QUEUE_SIZE bytes; enqueue writes to the
 *          rear, dequeue reads from the front, and indices wrap around
 *          via modulo QUEUE_SIZE.
 */

/* ============================================
 * Includes
 * ============================================ */
#include "str_circular_queue.h"

/* ============================================
 * Public API Implementation
 * ============================================ */

/* Initialize an empty queue. */
CircularQueue_t CIRQ_zInit(void)
{
    CircularQueue_t q;

    q.front = 0U;
    q.rear  = 0U;
    q.count = 0U;

    for (u8_t i = 0U; i < QUEUE_SIZE; i++)
    {
        q.data[i] = 0U;
    }

    return q;
}

void CIRQ_Init(CircularQueue_t *q)
{
	q->front = 0U;
	q->rear  = 0U;
	q->count = 0U;

	for (u8_t i = 0U; i < QUEUE_SIZE; i++)
	{
		q->data[i] = 0U;
	}
}

/* State queries */
bool_t CIRQ_IsEmpty(const CircularQueue_t *q)
{
    return (q->count == 0U) ? TRUE : FALSE;
}

bool_t CIRQ_IsFull(const CircularQueue_t *q)
{
    return (q->count >= QUEUE_SIZE) ? TRUE : FALSE;
}


/* Core FIFO operations */
bool_t CIRQ_Stack(CircularQueue_t *q, u8_t value)
{
    if (CIRQ_IsFull(q))
    {
        return FALSE;
    }

    q->data[q->rear] = value;
    q->rear = (u8_t)((q->rear + 1U) % QUEUE_SIZE);
    q->count++;

    return TRUE;
}

bool_t CIRQ_Pop(CircularQueue_t *q, u8_t *out_value)
{
    if (CIRQ_IsEmpty(q))
    {
        return FALSE;
    }

    *out_value = q->data[q->front];
    q->front = (u8_t)((q->front + 1U) % QUEUE_SIZE);
    q->count--;

    return TRUE;
}

/* Debug helper */
bool_t CIRQ_Read(const CircularQueue_t *q, u8_t indx, u8_t *out_value)
{
    if (CIRQ_IsEmpty(q) || (indx >= q->count))
    {
        return FALSE;
    }

    /* Translate the logical index into a physical array slot. */
    u8_t physical = (u8_t)((q->front + indx) % QUEUE_SIZE);
    *out_value = q->data[physical];

    return TRUE;
}