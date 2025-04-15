#define MAX_LEN 1500
#define MAX_LEN_PER_LINE 200
#define PORT 12345
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <hal/udp.h>
#include <pthread.h>

#include "hal/deterrents.h"

#define RECORDING "recording"
#define DETERRENTS "deterrents"

static int socketDescriptor;
static bool udp_is_initalized = false;

static pthread_t id;
static bool continueListening = true;

static int recordingStatus = 0;

void udp_send_reply(struct sockaddr_in sinRemote, char* messageTx)
{
    // Send reply
    unsigned int sin_len = sizeof(sinRemote);
    sendto(socketDescriptor, messageTx, strlen(messageTx), 0, (struct sockaddr *)&sinRemote, sin_len);
}

void recording_option_response(struct sockaddr_in sinRemote, int value)
{
    // int currentRecordingStatus = getCurrentRecordingStatus(); REVISIT: implement this

    // value == -1 means server is just checking the value to display it to frontend
    if(value == -1){
        char messageTx[MAX_LEN];
        snprintf(messageTx, MAX_LEN, "%d", recordingStatus);
        udp_send_reply(sinRemote, messageTx);
        return;
    }

    if(value == 0){
        // stopRecording();
        printf("stopRecording\n");
    }
    else if(value == 1){
        // startRecording();
        printf("startRecording\n");
    }
    char messageTx[MAX_LEN];
    snprintf(messageTx, MAX_LEN, "%d",value);
    udp_send_reply(sinRemote, messageTx);
}

void deterrent_option_response(struct sockaddr_in sinRemote, int value)
{
    // int currentDeterrentStatus = getCurrentDeterrentStatus(); REVISIT: implement this

    // value == -1 means server is just checking the value to display it to frontend
    if(value == -1){
        char messageTx[MAX_LEN];
        snprintf(messageTx, MAX_LEN, "%d", Deterrents_check_deterrents_status());
        udp_send_reply(sinRemote, messageTx);
        return;
    }

    if(value == 0){
        printf("stopDeterrent\n");
        Deterrents_cancel_deterrents();
    }

    else if(value == 1){
        printf("startDeterrent\n");
        Deterrents_activate_deterrents();
    }
    char messageTx[MAX_LEN];
    snprintf(messageTx, MAX_LEN, "%d",value);
    udp_send_reply(sinRemote, messageTx);
}

void *udp_receive_data_and_respond()
{
    while(continueListening){
        printf("running\n");
        struct sockaddr_in sinRemote;
        unsigned int sin_len = sizeof(sinRemote);
        char messageRx[MAX_LEN];

        int bytesRx = recvfrom(socketDescriptor, messageRx, MAX_LEN - 1, 0, (struct sockaddr *)&sinRemote, &sin_len);
        messageRx[bytesRx] = 0;
        // printf("Message received: (%d bytes): '%s'\n", bytesRx, messageRx);

        char command[MAX_LEN];
        // fix this 
        int value = -1;
        int words = sscanf(messageRx,"%s %d", command,&value);

        if (words == 0){
            printf("error\n");
        }

        if(strncmp(command, RECORDING, MAX_LEN) == 0){
            recording_option_response(sinRemote,value);
            // printf("recording_option_response\n");
        }
        
        else if(strncmp(command, DETERRENTS, MAX_LEN) == 0){
            deterrent_option_response(sinRemote,value);
            // printf("deterrent_option_response\n");
        }

        else{
            printf("Invalid command\n");
            char messageTx[] = "Invalid command. Type 'help' for a list of commands.\n";
            udp_send_reply(sinRemote, messageTx);
        }
    }
    return NULL;
}

void Udp_init()
{
    assert(!udp_is_initalized);
    udp_is_initalized = true;
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(PORT);

    // Create and bind to socket
    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    bind(socketDescriptor, (struct sockaddr*)&sin, sizeof(sin));

    // Create a thread to receive data and respond
    pthread_create(&id, NULL, &udp_receive_data_and_respond, NULL);

    printf("UDP_init finished, initialized!!\n");
}


void Udp_cleanup()
{
    assert(udp_is_initalized);
    continueListening = false;
    pthread_join(id, NULL);
    close(socketDescriptor);
    // printf("Cleaning up...54321\n");
    udp_is_initalized = false;
}