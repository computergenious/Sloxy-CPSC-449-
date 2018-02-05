/*
 * Proxy.c
 *  This program is a Basic HTTP proxy which receives the HTTP Requests from the web browser
 *  and forward it to the desired web server. After receiving the response from the web server
 *  it forward it to the web browser of the client.
 *  The proxy listens on port 8001
 *  Created on: Jan 23, 2018
 */

/* Standard libraries */
#include <stdio.h>
#include <stdlib.h>

/* Libraries for socket programming */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* Library for parsing strings */
#include <string.h>
#include <strings.h>

/* h_addr?! */
#include <netdb.h>

/* Clean exit! */
#include <signal.h>

int lstn_sock;

/* The function will run after catching Ctrl+c in terminal */
void catcher(int sig) {
    close(lstn_sock);
    printf("catcher with signal  %d\n", sig);
    exit(0);
    
}

int main() {
    
    /* For catching Crtl+c in terminal */
    signal(SIGINT, catcher);
    int lstn_port = 8001;
    
    /* Initializing the Address */
    struct sockaddr_in addr_proxy;
    addr_proxy.sin_family = AF_INET;
    addr_proxy.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_proxy.sin_port = htons(lstn_port);
    printf("----------Address Initialization: done.\n");
    
    /* Creating the listening socket for proxy */
    lstn_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (lstn_sock < 0) {
        printf("Error in socket() call.\n");
        exit(-1);
    } else {
        printf("Listening Socket creation: done.\n");
    }
    
    /* Binding the socket to address and port */
    int bind_status;
    bind_status = bind(lstn_sock, (struct sockaddr *) &addr_proxy,
                       sizeof(struct sockaddr_in));
    if (bind_status < 0) {
        printf("Error in bind() call.\n");
        exit(-1);
    } else {
        printf("Binding: done.\n");
    }
    
    /* Listening on binded port number */
    int lstn_status;
    lstn_status = listen(lstn_sock, 10);
    if (lstn_status < 0) {
        printf("Error in listen() call.\n");
        exit(-1);
    }
    
    /*
     *******************
     *
     * BONUS
     *
     *******************
     */
    int id = fork();
    
    /* Infinite while loop for listening accepting connection requests */
    while (1) {
        /* Accepting connection requests */
        int data_sock;
        data_sock = accept(lstn_sock, NULL, NULL);
        if (data_sock < 0) {
            printf("Error in accept() call");
            exit(-1);
        } else {
            printf("Accepting connection request: done.\n");
        }
        
        /* Receiving HTTP message from the client */
        char c_message_in[1024];
        char s_message_out[1024];
        int c_recv_status;
        c_recv_status = recv(data_sock, c_message_in, sizeof(c_message_in), 0);
        if (c_recv_status < 0) {
            printf("Error in recv() call for client recv message\n");
            exit(-1);
        } else {
            printf("############################################\n");
            printf("## HTTP message received from the client. ##\n ");
            printf("############################################\n");
        }
        
        printf("\nRequest coming in from client is\n%s\n", c_message_in);
        
        /* Preserving the HTTP request for sending it to the web server later */
        strcpy(s_message_out, c_message_in);
        
        /* Parsing the HTTP message to extract the HOST name of the desired web server */
        char host[1024];
        char URL[1024];
        char PATH[1024];
        int i;
        
        // find and parse the GET request, isolating the URL for later use
        char *pathname = strtok(c_message_in, "\r\n");
        printf("Found HTTP request: %s\n", pathname);
        if (sscanf(pathname, "GET http://%s", URL) == 1)
            printf("URL = %s\n", URL);
        
        // seperate the hostname from the pathname
        for (i = 0; i < strlen(URL); i++) {
            if (URL[i] == '/') {
                strncpy(host, URL, i); //copy out the hostname
                host[i] = '\0';
                break;
            }
        }
        
        bzero(PATH, 500); //to clear junk at the beginning of this buffer. don't know why its there
        for (; i < strlen(URL); i++) {
            strcat(PATH, &URL[i]); //copy out the path
            break;
        }
        printf("******************************\n");
        printf("First Half(host): %s\n", host); //firstHalf is the hostname
        printf("Second Half(path): %s\n", PATH); //secondHalf is the path
        printf("******************************\n");
        
        /* Creating the TCP socket for connecting to the desired web server */
        // Address initialization
        struct sockaddr_in addr_server;
        struct hostent *server;
        
        // Getting web server's Address by its host name
        server = gethostbyname(host);
        if (server == NULL)
            printf("Error in gethostbyname() call.\n");
        else
            printf("Web server = %s\n", server->h_name);
        
        // Initialize socket structure
        bzero((char *) &addr_server, sizeof(addr_server));
        addr_server.sin_family = AF_INET;
        bcopy((char *) server->h_addr, (char *) &addr_server.sin_addr.s_addr, server->h_length);
        addr_server.sin_port = htons(80);
        
       
        
        printf("\n\n\n-------------------- CREATE THE HEAD REQUEST------------------\n\n\n");
        
        // Creating the socket
        int web_sock1 = socket(AF_INET, SOCK_STREAM, 0);
        if (web_sock1 < 0) {
            printf(
                   "Error in socket() call for creating --proxy-WebServer-- socket.\n");
        } else {
            printf("###############################################\n");
            printf("## --proxy-WebServer-- socket creation: done ##\n");
            printf("###############################################\n");
        }
        
        // Connecting to the web server's socket
        int connection_status;
        connection_status = connect(web_sock1, (struct sockaddr *) &addr_server,
                                 sizeof(addr_server));
        if (connection_status < 0) {
            printf(
                   "Error in connect() call for connecting to the web server's socket.\n");
            exit(-1);
        } else {
            printf("Web server's socket connection establishment: done\n ");
        }
        
        // ARRAY THAT STORES ALL THE DATA
        char finalResult [50000];
        
        // CREATING THE HEAD REQUEST
        char request[50000];
        snprintf(request, sizeof(request), "HEAD http://%s HTTP/1.1\r\nHost: %s\r\n\r\n",URL,host);
        
        // SENDING THE HEAD REQUEST TO THE SERVER
        int headRequestStatus;
        char headRequestResponse[50000];
        bzero(headRequestResponse, sizeof(headRequestResponse));
        
        headRequestStatus = send(web_sock1, request, sizeof(request),0);
        if (headRequestStatus < 0) {
            printf(
                   " Error in send() call for sending HTTP request to the web server.\n ");
            exit(-1);
        } else {
            printf("##############################################\n");
            printf("## Sending HTTP request to the server: done ##\n");
            printf("##############################################\n");
        }
        
        // RECIEVE HEAD REQUEST FROM THE SERVER
        int headResponseStatus;
        
        headResponseStatus = recv(web_sock1, headRequestResponse, sizeof(headRequestResponse), 0);
        strcpy(finalResult, headRequestResponse);
        
        
        
        /*
         //
         // CHECK IF CONTENT-TYPE IS html OR NAH
         // IF IT IS HTML, THEN SEND A RANGE REQUEST TO
         // RETURN THE DOCUMENT IN 30 BYTE INCREMENTS
         //
         // IF NOT HTML, THEN JUST SEND THE SERVER RESPONSE
         // BACK TO THE CLIENT.
         //
        */
        char *contentType = "NULL";
        char contentTypeText;
        char *strippedHead = "NULL";
        
        contentType = strstr(headRequestResponse, "Content-Type: ");
        sscanf(contentType, "Content-Type: %s", &contentTypeText);

        
        if(strcmp(&contentTypeText, "text/html;") == 0){
            
            printf("\n\n\nThis is an HTML Document.\n\n\n");
            // PUT THE CONTENT-LENGTH INTO AN INTEGER VARIABLE
            
            char *contentLength;
            int contentLengthValue;
            int counter = 0;
            char rangeRequestURL[1024];
            
            contentLength = strstr(headRequestResponse, "Content-Length: ");
            printf("Content Length: %s", contentLength);
            sscanf(contentLength, "Content-Length: %d", &contentLengthValue);
            
            while (counter <= contentLengthValue) {
                
                // Creating the socket
                
                int web_sock2 = socket(AF_INET, SOCK_STREAM, 0);
                if (web_sock2 < 0) {
                    printf(
                           "Error in socket() call for creating --proxy-WebServer-- socket.\n");
                } else {
                    printf("###############################################\n");
                    printf("## --proxy-WebServer-- socket creation: done ##\n");
                    printf("###############################################\n");
                }
                
                // Connecting to the web server's socket
                
                int connect_status;
                connect_status = connect(web_sock2, (struct sockaddr *) &addr_server,
                                         sizeof(addr_server));
                if (connect_status < 0) {
                    printf(
                           "Error in connect() call for connecting to the web server's socket.\n");
                    exit(-1);
                } else {
                    printf("Web server's socket connection establishment: done\n ");
                }
                
                // CREATE THE RANGE REQUEST URL
                
                snprintf(rangeRequestURL, sizeof(rangeRequestURL), "GET http://%s HTTP/1.1\r\nHost: %s\r\nRange: bytes=%d-%d\r\n\r\n",URL,host,counter,(counter+100));
                counter += 101;
                
                // SEND THE RANGE REQUEST URL TO THE SERVER
                
                int rangeRequestResponseStatus;
                char rangeRequestResponse[1024];
                bzero(rangeRequestResponse, sizeof(rangeRequestResponse));
                
                rangeRequestResponseStatus = send(web_sock2, rangeRequestURL, sizeof(rangeRequestURL), 0);
                
                if (rangeRequestResponseStatus < 0) {
                    printf(
                           " Error in send() call for sending HTTP request to the web server.\n ");
                    exit(-1);
                } else {
                    printf("##############################################\n");
                    printf("## Sending HTTP request to the server: done ##\n");
                    printf("##############################################\n");
                }
                
                // RECIEVE THE RANGE REQUEST RESPONSE FROM THE SERVER
                
                int rangeRequestServerResponseStatus;
                
                rangeRequestServerResponseStatus = recv(web_sock2, rangeRequestResponse, sizeof(rangeRequestResponse), 0);
                
                strippedHead = strstr(rangeRequestResponse, "\r\n\r\n");
                strcat(finalResult, strippedHead);
                close(web_sock2);
            }
            printf("\nfinalResult has: \n%s\n",finalResult);
            
            // SEND THE SERVER RESPONSE BACK TO THE CLIENT
            int sendToClientStatus;
            sendToClientStatus = send(data_sock, finalResult, sizeof(finalResult), 0);
            if (sendToClientStatus < 0) {
                printf(
                       "Error in send() call for sending HTTP response to the client.\n");
                exit(-1);
            } else {
                printf("###############################################\n");
                printf("## Sending HTTP response to the client: done ##\n");
                printf("###############################################\n");
            }
        }
        else{
            printf("This is not an HTML document.\n");
            // Creating the socket
            int web_sock = socket(AF_INET, SOCK_STREAM, 0);
            if (web_sock < 0) {
                printf(
                       "Error in socket() call for creating --proxy-WebServer-- socket.\n");
            } else {
                printf("###############################################\n");
                printf("## --proxy-WebServer-- socket creation: done ##\n");
                printf("###############################################\n");
            }
            
            // Connecting to the web server's socket
            int connectionStatus;
            connectionStatus = connect(web_sock, (struct sockaddr *) &addr_server,
                                       sizeof(addr_server));
            if (connectionStatus < 0) {
                printf(
                       "Error in connect() call for connecting to the web server's socket.\n");
                exit(-1);
            } else {
                printf("Web server's socket connection establishment: done\n ");
            }
            
            /* Sending the HTTP request of the client to the web server */
            int web_send_status;
            web_send_status = send(web_sock, s_message_out, sizeof(c_message_in),0);
            if (web_send_status < 0){
                printf(
                       " Error in send() call for sending HTTP request to the web server.\n ");
                exit(-1);
            } else {
                printf("##############################################\n");
                printf("## Sending HTTP request to the server: done ##\n");
                printf("##############################################\n");
            }
            
            /* Receiving the HTTP response from the web server */
            char w_message_in[50000];
            bzero(w_message_in, sizeof(w_message_in));
            
            int web_recv_status;
            web_recv_status = recv(web_sock, w_message_in, sizeof(w_message_in), 0);
            if (web_recv_status < 0) {
                printf(
                       " Error in recv() call for receiving web server's HTTP response.\n ");
                exit(-1);
            } else {
                printf("################################################\n");
                printf("## Receiving web server's HTTP response: done ##\n");
                printf("################################################\n");
            }
            
            printf("\n\nResponse is \n%s\n\n", w_message_in);
            
            
            /* Closing the socket connection with the web server */
            close(web_sock); // SOCKET CONNECTION FOR GET REQUEST
            close(web_sock1); // SOCKET CONNECTION FOR HEAD REQUEST
            
            /* Sending the HTTP response to the client */
            int c_send_status;
            c_send_status = send(data_sock, w_message_in, sizeof(w_message_in), 0);
            if (c_send_status < 0) {
                printf(
                       "Error in send() call for sending HTTP response to the client.\n");
                exit(-1);
            } else {
                printf("###############################################\n");
                printf("## Sending HTTP response to the client: done ##\n");
                printf("###############################################\n");
            }
        }
        /* Closing the socket connection with the client */
        close(data_sock);
        printf("---------- data socket is closed.\n");
    }
    close(lstn_sock);
    printf("lstn_sock is closed.\n");
    return 0;
    
}

