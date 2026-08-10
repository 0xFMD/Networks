#include "../include/c_chat.h"


payload_t *create_message(const user_t *user, const char *message){
 if(!message || !user) return NULL;

    payload_t * payload = (payload_t)malloc(sizeof(payload_t));
    if(!message) return NULL;

    message->user_metadata = *user;
    message->type = MESSAGE;
    message->message_len = size;
    return message ;


}

bool send_message(int socket_fd, const payload_t *message){
    if(!message || !socket_fd) {

        printf("Error: Invalid message or socket_fd\n");
    
        return false;
        }
    
size_t sent = send (socket_fd, message, sizof(payload_t),0);


    if(sent != sizeof(payload_t)){
        return false;
    }
    return true;
}