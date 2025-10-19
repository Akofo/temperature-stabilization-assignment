#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include "utils.h"

int main (int argc, char *argv[])
{
    int socket_desc;
    struct sockaddr_in server_addr;
    struct msg the_message; 
    
    int my_index = atoi(argv[1]); 
    float current_temp = atof(argv[2]); 

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    
    if(socket_desc < 0){
        printf("Unable to create socket\n");
        return -1;
    }
    printf("Socket created successfully\n");
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with server successfully\n");
    printf("--------------------------------------------------------\n\n");
    
    int finished = 0;
    int iter=0;

    while (finished==0) {
        iter = iter+1;
        
        the_message = prepare_message(my_index, current_temp); 

        if(send(socket_desc, (const void *)&the_message, sizeof(the_message), 0) < 0){
            printf("Unable to send message\n");
            return -1;
        }

        if(recv(socket_desc, (void *)&the_message, sizeof(the_message), 0) < 0){
            printf("Error while receiving server's msg\n");
            return -1;
        }
        
        float temp_from_central = the_message.T;
        
        printf("Iteration %d:\n",iter);
        printf("  Current external temp: %f\n",current_temp);
        printf("  Received central temp: %f\n",temp_from_central);

        if (the_message.Index == -1) {
            finished = 1;
            printf("  System has stabilized!\n");
        } else {
            float new_temp;
            new_temp = (3*current_temp + 2*temp_from_central)/5;
            current_temp = new_temp;
            printf("  Updated external temp: %f\n",current_temp);
        }
        printf("\n");
    }
    
    printf("--------------------------------------------------------\n");
    printf("Final stabilized temperature = %f\n",current_temp);
    printf("--------------------------------------------------------\n");
    
    close(socket_desc);
    
    return 0;
}
