#ifndef SOCKETQ_H_INCLUDED
#define SOCKETQ_H_INCLUDED


#include <stdlib.h>
#include <stdint.h>


/**
 * @brief SocketQueue structure
 *
 */
typedef struct {
    char  *path;
    int    server;
    int    client;
    size_t msg_size;
} socketq_t;


/**
 * @brief Initialize the provided socket queue
 *
 * @param socketq Pointer to the socketq_t struct
 * @param path filesystem path. It should be unique for every socketq
 * @param msg_size Size of the messages that will be passed
 * @return int -1 in case of error, 0 otherwise
 */
int socketq_init(socketq_t *socketq, char *path, size_t msg_size);

/**
 * @brief Send a message through a socketq. The size of the message is implicitly stored in the socketq struct
 *
 * @param socketq
 * @param message
 */
void socketq_send(socketq_t *socketq, uint8_t *message);

/**
 * @brief Receive a message from a socketq. The size of the message is implicitly stored in the socketq struct
 *
 * @param socketq
 * @param message
 * @return int 1 if a message was received, 0 otherwise
 */
int socketq_receive(socketq_t *socketq, uint8_t *message);

/**
 * @brief Same as socketq_receive but non blocking; the timeout is specified in milliseconds
 *
 * @param socketq
 * @param message
 * @param timeout
 * @return int
 */
int socketq_receive_nonblock(socketq_t *socketq, uint8_t *message, int timeout);


#endif