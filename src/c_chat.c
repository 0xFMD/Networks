#include "../include/c_chat.h"


payload_t *create_message(const user_t *user, const char *message){
 
       if(!message || !user) return NULL;

    payload_t * payload = (payload_t*)malloc(sizeof(payload_t));
    if(!payload) return NULL;

    payload->user_metadata = *user;
    payload->type = MESSAGE;
    payload->message_len = strlen(message);
    return payload; 


}

bool send_message(int socket_fd, const payload_t *message){

    if(!message || socket_fd < 0){

        printf("Error: Invalid message or socket_fd\n");
    
        return false;
        }
    
size_t sent = send(socket_fd, message, sizeof(payload_t), 0);


    if(sent != sizeof(payload_t)){
        return false;
    }
    return true;}


bool receive_message(int socket_fd, payload_t *message){
       
    if(!message || !socket_fd  < 0) {
            
            return false;
    }

    int rsev = read(client_fd, message, sizeof(message) - 1);

    if (rsev != sizeof(payload_t)) {
        return false;
    }
    return true;
}
