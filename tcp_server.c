#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <math.h>
#include "utils.h"


#define numExternals 4
#define EPS 0.001


int * establishConnectionsFromExternalProcesses()
{

    int socket_desc;
    static int client_socket[numExternals]; 

    unsigned int client_size;
    struct sockaddr_in server_addr, client_addr;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        exit(0);
    }
    printf("Socket created successfully\n");
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        printf("Couldn't bind to the port\n");
        exit(0);
    }
    printf("Done with binding\n");
    
    if(listen(socket_desc, 1) < 0){
        printf("Error while listening\n");
        exit(0);
    }
    printf("\n\nListening for incoming connections.....\n\n");

    printf("-------------------- Initial connections ---------------------------------\n");


    int externalCount = 0; 
    while (externalCount < numExternals){

        client_socket[externalCount] = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);
        
        if (client_socket[externalCount] < 0){
            printf("Can't accept\n");
            exit(0);
        }

        printf("One external process connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        externalCount++; 
    }
    printf("--------------------------------------------------------------------------\n");
    printf("All four external processes are now connected\n");
    printf("--------------------------------------------------------------------------\n\n");

    return client_socket;   
}



int main(void)
{
    struct msg messageFromClient;   
    int * client_socket = establishConnectionsFromExternalProcesses(); 

    float temp[numExternals];
    float prev_temp[numExternals];
    int i;
    for(i=0;i<4;i++){
        prev_temp[i]=0;
    }
    
    float centralTemp=0;
    int is_stable = 0;
    int iteration_count=0;

    while (is_stable==0) {
        iteration_count++;
        printf("==================== Iteration %d ====================\n",iteration_count);

        for (i = 0; i < numExternals; i++) {
            if (recv(client_socket[i], (void *)&messageFromClient, sizeof(messageFromClient), 0) < 0) {
                printf("Couldn't receive\n");
                return -1;
            }
            temp[i] = messageFromClient.T;
            printf("Temperature of External Process (%d) = %f\n", i, temp[i]);
        }

        float sum=0;
        int j;
        for (j = 0; j < numExternals; j++) {
            sum = sum + temp[j];
        }

        centralTemp = (2*centralTemp + sum)/6;
        printf("\nUpdated Central Temperature = %f\n\n",centralTemp);

        int all_stable = 1;
        if (iteration_count > 1) {
            for (i = 0; i < 4; i++) {
                float diff = temp[i] - prev_temp[i];
                if(diff<0) diff = -diff;
                if (diff >= EPS) {
                    all_stable = 0;
                }
            }
        } else {
            all_stable = 0;
        }

        struct msg msg_to_send; 
        msg_to_send.T = centralTemp;
        if(all_stable==1){
            msg_to_send.Index = -1;
        }else{
            msg_to_send.Index = 0;
        }

        for (i = 0; i < numExternals; i++) {
            if (send(client_socket[i], (const void *)&msg_to_send, sizeof(msg_to_send), 0) < 0) {
                printf("Can't send\n");
                return -1;
            }
        }        

        int k;
        for (k = 0; k < numExternals; k++) {
            prev_temp[k] = temp[k];
        }

        if (all_stable==1) {
            printf("==================== System Stabilized! ====================\n");
            printf("Final stabilized temperature: %f\n",centralTemp);
            is_stable=1;
        }
        printf("\n");
    }
 
    for (i = 0; i < numExternals; i++){
        close(client_socket[i]);
    }

    return 0;
}
