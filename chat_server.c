#include<stdio.h>
#include<winsock2.h>
#include <stdlib.h>
#include<strings.h>
#include<windows.h>

#define MSG_SIZE 512
#define MAX_CLIENT 30

FILE *fp;
fd_set readfds;//set of socket descriptors
char *rcvd_msg, send_msg[MSG_SIZE], msg[MSG_SIZE], users[MAX_CLIENT][20], user[20]={'\0'}, ach[20];
SOCKET master_socket , new_socket , client_socket[MAX_CLIENT] , sock;
struct sockaddr_in client, address;
int activity, addrlen, rcvd_size, public_chatters[MAX_CLIENT], tracker, menu;

int show_menu();
DWORD WINAPI ReadingThread(LPVOID param);
int getHash(char *hash);
char callHash(int hash);
void openData();
int verify_entry();
int login_admin();




int main()
{
    WSADATA wsa;
    char feed[1];
    int choice;
char uname[20], pass[20];
    int i;
    rcvd_msg =  (char*) malloc((MSG_SIZE + 1) * sizeof(char));




     menu=0;
     show_menu();
     menu=1;
     choice=show_menu();

    do
    {
    switch(choice)
    {
        case 1:
            login_admin();
            goto cont;
            break;

        case 2:
            exit(0);
            break;

        default:
            printf("\t\t\t    WRONG CHOICE. PLEASE TRY AGAIN.\n");
    }
    }while(1);


cont:

    system("cls");
     menu=0;
     show_menu();
     menu=2;
     choice=show_menu();


    do
    {
    switch(choice)
    {
        case 1:
            goto init;
            break;

        case 2:
            exit(0);
            break;

        default:
            printf("\t\t\t    WRONG CHOICE. PLEASE TRY AGAIN.\n");
    }
    }while(1);


    init:
    openData();
    for(tracker = 0 ; tracker < MAX_CLIENT;tracker++)
    {
        client_socket[tracker] = 0;
        public_chatters[tracker]=0;

    }


    printf("WELCOME TO U-CHAT\n");

    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)//Winsock Initialization
    {
        printf("\nWinsock Initialization Failed. Error Code : %d ",WSAGetLastError());
        printf("\nPress any key to exit.");
        getch();
        return 1;
    }

    printf("Initialised.\n");

    //Create a socket
    if((master_socket = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
    {
        printf("\nCould not create socket : %d" , WSAGetLastError());
        printf("\nPress any key to exit.");
        getch();
        return 1;
    }

    //Prepare the sockaddr_in structure
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = INADDR_ANY;
    client.sin_port = htons( 7777 );

    //Bind
    if( bind(master_socket ,(struct sockaddr *)&client , sizeof(client)) == SOCKET_ERROR)
    {
        printf("Bind failed with error code : %d" , WSAGetLastError());
        printf("\nPress any key to exit.");
        getch();
        return 1;
    }
    printf("Server Initiallised.\nWaiting for Clients.");

    //Listen to incoming connections
    listen(master_socket , 3);

    addrlen = sizeof(struct sockaddr_in);


    while(TRUE)
    {
        //clear the socket fd set
        FD_ZERO(&readfds);

        //add master socket to fd set
        FD_SET(master_socket, &readfds);

        //add child sockets to fd set
        for (  tracker = 0 ; tracker < MAX_CLIENT ; tracker++)
        {
            sock = client_socket[tracker];
            if(sock > 0)
            {
                FD_SET( sock , &readfds);
            }
        }

        //wait for an activity on any of the sockets, timeout is NULL , so wait indefinitely
        activity = select( 0 , &readfds , NULL , NULL , NULL);

        if ( activity == SOCKET_ERROR )
        {
            printf("select call failed with error code : %d" , WSAGetLastError());
            printf("\nPress any key to exit.");
            getch();
            return 1;
        }

        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(master_socket , &readfds))
        {
            if ((new_socket = accept(master_socket , (struct sockaddr *)&address, (int *)&addrlen))<0)
            {
                perror("accept");
                printf("\nPress any key to exit.");
                getch();
                return 1;
            }
            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
            //add new socket to array of sockets
            for (tracker = 0; tracker <MAX_CLIENT; tracker++)
            {
                if (client_socket[tracker] == 0)
                {
                    client_socket[tracker] = new_socket;
                    printf("Adding client to list of clients at index %d \n" , tracker);
                    break;
                }
            }
        }

        //else its some IO operation on some other socket :)
        for (tracker = 0; tracker < MAX_CLIENT; tracker++)
        {
            sock = client_socket[tracker];
            //if client presend in read sockets
            if (FD_ISSET( sock , &readfds))
            {
                //get details of the client
                getpeername(sock , (struct sockaddr*)&address , (int*)&addrlen);
                //Check if it was for closing , and also read the incoming message
                //recv does not place a null terminator at the end of the string (whilst printf %s assumes there is one).
                rcvd_size = recv( sock , rcvd_msg, MSG_SIZE, 0);


                if( rcvd_size == SOCKET_ERROR)
                {
                    int error_code = WSAGetLastError();
                    if(error_code == WSAECONNRESET)
                    {
                        //Somebody disconnected , get his details and print
                        printf("Host disconnected unexpectedly , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                        //Close the socket and mark as 0 in list for reuse
                        closesocket( sock );
                        client_socket[tracker] = 0;
                    }
                    else
                    {
                        printf("recv failed with error code : %d" , error_code);
                    }
                }
                else if ( rcvd_size == 0)
                {
                    //Somebody disconnected , get his details and print
                    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    closesocket( sock );
                    client_socket[tracker] = 0;
                }

                //Echo back the message that came in
                else
                {
                    //add null character, if you want to use with printf/puts or other string handling functions
                    rcvd_msg[rcvd_size] = '\0';
                    feed[0]='\0';
                    feed[0]=callHash(getHash(rcvd_msg));

                    if(feed[0]=='0'||feed[0]=='1')
                        send( sock , feed , 1 , 0 );
                }
            }
        }
    }
}






void openData()
{
    fp=fopen("Data/user_data.txt", "r+");

    if(fp==NULL)
    {
        fp=fopen("Data/user_data.txt", "w+");
        if(fp==NULL)
        {
            printf("\nDatabase Creation Failed.");
            printf("\nPress any key to exit.");
            getch();
            exit(0);
        }
    }
}




int getHash(char *hash)
{
    int i,x=0;
    char p[4];
    for(i=1;i<5;i++)
        p[i-1]=*(hash+i);

    x=atoi(p);
    return x;
}


char callHash(int hash)
{
    char ch, ucheck[30],n_msg[MSG_SIZE], u_store[20];
    SOCKET ns;
    int count=0,i,j;
    switch(hash)
    {
        case 1011:
            fseek( fp, 0, SEEK_SET );
            count=0;
            for(i=0;(ch=fgetc(fp))!=EOF;i++)
            {
                ucheck[i]=ch;
                if(ch=='#')
                {
                    count++;
                    if(count==3)
                    {
                        ucheck[i]='\0';
                        if(strcmp(rcvd_msg,ucheck)==0)
                        {
                            return '1';
                        }
                        count=0;
                        strcpy(ucheck,"\0");
                        i=-1;
                    }
                }
                else if(ch=='\n')
                {
                    count=0;
                    strcpy(ucheck,"\0");
                    i=-1;
                }
            }
            strcpy(msg,rcvd_msg);
            return '0';
            break;

        case 1012:
            fseek( fp, 0, SEEK_END );
            strcat(msg, rcvd_msg);
            strcat(msg, "\n");
            fputs(msg,fp);
            fclose(fp);
            openData();
            printf("%s",msg);
            return '0';
            break;

        case 1013:
            fseek( fp, 0, SEEK_SET );
            rcvd_msg[4]='1';
            for(i=0;(ch=fgetc(fp))!=EOF;i++)
            {
                ucheck[i]=ch;
                if(ch=='\n')
                {
                    ucheck[i]='\0';
                    if(strcmp(rcvd_msg,ucheck)==0)
                    {

                        for(j=6;j<strlen(rcvd_msg);j++)
                        {
                            if(rcvd_msg[j]=='#')
                            {

                                u_store[j-6]='\0';
                                strcpy(users[tracker],u_store);
                                goto go1;
                            }
                            u_store[j-6]=rcvd_msg[j];
                        }

                        go1:
                            return '0';
                    }
                    strcpy(ucheck,"\0");
                    i=-1;
                }
            }
            return '1';
            break;

        case 1021:
            public_chatters[tracker]=1;
            strcpy(n_msg,users[tracker]);
            strcat(n_msg," has joined public chat.");
            for(i=0;i<MAX_CLIENT;i++)
            {
                ns=client_socket[i];
                if(i!=tracker&&public_chatters[i]==1)
                    send( ns , n_msg , strlen(n_msg) , 0 );
            }
            break;
        case 1022:
            strcpy(n_msg,users[tracker]);
            strcat(n_msg," : ");
            strcat(n_msg,&rcvd_msg[6]);
            for(i=0;i<MAX_CLIENT;i++)
            {
                ns=client_socket[i];
                if(i!=tracker&&public_chatters[i]==1)
                    send( ns , n_msg , strlen(n_msg) , 0 );
            }
            break;

        case 1023:
            public_chatters[tracker]=0;
            strcpy(n_msg,users[tracker]);
            strcat(n_msg," has left public chat.");
            for(i=0;i<MAX_CLIENT;i++)
            {
                ns=client_socket[i];
                if(i!=tracker&&public_chatters[i]==1)
                    send( ns , n_msg , strlen(n_msg) , 0 );
            }
            break;
    }
}





int verify_entry(char entry[20])
{
    int i,x;
    if(strlen(entry)>20)
        return 1;
    for(i=0;i<strlen(entry);i++)
    {
        x=entry[i];
        if((x>=64&&x<=90)||(x>=97&&x<=122)||(x>=48&&x<=57)||(x==46)||(x==95))
            continue;
        return 1;
    }
    return 0;
}




int show_menu()
{
    int choice;
    switch(menu)
    {
        case 0:

            printf("\t\t\t********************************\n");
            printf("\t\t\t    WELCOME TO Hoxchat by Arun %s\n",user);
            printf("\t\t\t********************************\n\n\n");
            break;
        case 1:
            printf("\t\t\t       \n");
            printf("\t\t\t-------------------------\n");
            printf("\t\t\t|    1--->Login         |\n");
            printf("\t\t\t|    2--->Exit          |\n");
            printf("\t\t\t-------------------------\n\n\n");
            printf("\t\t\t    Enter a Choice\n");
            break;
        case 2:
            printf("\t\t\t-----------------------------\n");
            printf("\t\t\t|    1--->Initialize Server |\n");
            printf("\t\t\t|    2--->Exit              |\n");
            printf("\t\t\t-----------------------------\n\n\n");
            printf("\t\t\t    Enter a Choice\n");
            break;
    }
    if(menu==0)
        choice=100;
    else
        scanf("%d",&choice);
    return choice;
}




int login_admin()
{
    char uname[20], pass[20];
    int i=0;
    char ch;
    int k;


    re_uname:
        printf("\t\t\t    Enter admin Username ---> ");
        fflush(stdin);
        gets(uname);
        if(verify_entry(uname))
        {
            printf("\t\t\t    INVALID USERNAME. PLEASE TRY AGAIN.\n");
            goto re_uname;
        }

    re_pass:
        printf("\t\t\t    Enter admin Password ---> ");
        fflush(stdin);
        i=0;
        while (1)
        {
            if (i < 0)
            {
                i = 0;
            }
            ch = getch();
            if (ch == 13)
                break;

            if (ch == 8) /*ASCII value of BACKSPACE*/
            {
                printf("\b \b");
                i--;
                pass[i] = '\0';
                continue;
            }
            pass[i++] = ch;
            ch = '*';
            printf("%c",ch);
        }
        pass[i]='\0';
        if(verify_entry(pass))
        {
            printf("\t\t\t    INVALID PASSWORD. PLEASE TRY AGAIN.\n");
            goto re_pass;
        }

    if(strcmp(uname,"admin")==0||strcmp(pass,"admin")==0)
    {
        strcpy(user,uname);
    return 0;

    }
    printf("\t\t\t    USERNAME OR PASSWORD INCORRECT. PLEASE TRY AGAIN.\n");
        goto re_uname;

}
