/**
 * @file    str_circular_queue.h
 * @brief   
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    
 */

#ifndef STR_CIRCULAR_QUEUE_H
#define STR_CIRCULAR_QUEUE_H
/* ============================================
 * Standard Library Includes
 * ============================================ */
#include "../common/std_types.h"			/* Provides u8_t, u16_t, bool_t */

/* ============================================
 * Special Macros
 * ============================================ */
#define QUEUE_SIZE    10  

/* ============================================
 * Structure
 * ============================================ */
typedef struct {
    u8_t  data[QUEUE_SIZE];
    u8_t  front;		/* index of the front (oldest) element */
    u8_t  rear;			/* index where the next element will go */
    u8_t  count;		/* number of elements currently stored */
} CircularQueue_t;

/* ============================================
 * Public API
 * ============================================ */

/* Initialize an empty queue. */

/**
 * @brief   Initialize an empty queue.
 * @param   max  Maximum number of elements (informational; must not exceed QUEUE_SIZE).
 * @return  A zero-initialized CircularQueue_t with front = rear = count = 0.
 *
 * @note    The `max` parameter is accepted for API symmetry but the
 *          capacity is fixed at compile time by QUEUE_SIZE. If a
 *          runtime capacity is ever required, this is the function
 *          to extend.
 */
CircularQueue_t CIRQ_zInit(void);
void CIRQ_Init(CircularQueue_t *q);

/* State queries */

/**
 * @brief   Check whether the queue is empty.
 * @param   q  Pointer to the queue (must not be NULL).
 * @return  TRUE if no elements are stored, FALSE otherwise.
 */
bool_t CIRQ_IsEmpty (const CircularQueue_t *q);

/**
 * @brief   Check whether the queue is full.
 * @param   q  Pointer to the queue (must not be NULL).
 * @return  TRUE if the queue is at capacity, FALSE otherwise.
 */
bool_t CIRQ_IsFull (const CircularQueue_t *q);


/* Core FIFOoperations */

/**
 * @brief   Push a byte onto the rear of the queue.
 * @param   q      Pointer to the queue.
 * @param   value  Byte to enqueue.
 * @return  TRUE on success, FALSE if the queue is full.
 */
bool_t CIRQ_Stack (CircularQueue_t *q, u8_t value);		/* push to rear  */

/**
 * @brief   Pop a byte from the front of the queue.
 * @param   q          Pointer to the queue.
 * @param   out_value  Destination for the removed byte (must not be NULL).
 * @return  TRUE on success, FALSE if the queue is empty.
 */
bool_t CIRQ_Pop (CircularQueue_t *q, u8_t *out_value);	/* pop from front */


/* Debug helper */

/**
 * @brief   Read the byte at logical index `indx` without removing it.
 * @param   q          Pointer to the queue.
 * @param   indx       Logical offset from the front (0 = oldest element).
 * @param   out_value  Destination for the read byte (must not be NULL).
 * @return  TRUE on success, FALSE if `indx` is out of range or the queue is empty.
 *
 * @note    This is a non-destructive peek at an arbitrary position,
 *          unlike a strict "read front" operation. Indices are
 *          translated through the wraparound before access.
 */
bool_t CIRQ_Read(const CircularQueue_t *q, u8_t indx, u8_t *out_value); /* read front */


#endif /* STR_CIRCULAR_QUEUE_H */