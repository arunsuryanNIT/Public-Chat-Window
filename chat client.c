#include<dos.h>

#include<stdio.h>
#include<winsock2.h>
#include <stdlib.h>
#include<strings.h>
#include<windows.h>

#define MSG_SIZE 512

int choice, menu, rcvd_size, activity;
char user[20]={'\0'};
SOCKET sock;//Network Socket to communicate with the server
struct sockaddr_in server;
char send_msg[MSG_SIZE], *rcvd_msg, ip_server[30], msg[MSG_SIZE];




DWORD WINAPI ReadingThread(LPVOID param);
int show_menu();
void home_page();
void exit_user();
int register_user();
int verify_entry();
int login_user();
void dashboard();
void public_chat();
void private_chat();
void retext();


int main()
{
    WSADATA wsa;//Windows Socket Application Data variable
    rcvd_msg =  (char*) malloc((MSG_SIZE + 1) * sizeof(char));

    printf("WELCOME TO Hoxchat by Arun\n");

    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)//Winsock Initialization
    {
        printf("\nWinsock Initialization Failed. Error Code : %d ",WSAGetLastError());
        printf("\nPress any key to exit.");
        getch();
        return 1;
    }

    //Create a network socket
    if((sock = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
    {
        printf("\nCould not create socket : %d" , WSAGetLastError());
        printf("\nPress any key to exit.");
        getch();
        return 1;
    }

    server.sin_family = AF_INET;//IPv4 Protocol
    server.sin_port = htons( 7777 );//Port Number to be connected

    ip_reconnect://goto label to reconnect to ip address
    printf("\nReady to Connect. Enter the IP Address of the server to connect.\n");
    scanf("%s",&ip_server);//Input IP of the server


    server.sin_addr.s_addr = inet_addr(ip_server);//IP of the server to be connected

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        printf("\nCould not connect to the IP given : %d" , WSAGetLastError());
        printf("\nPress 1 to enter IP of the server again");
        printf("\nPress any other key to exit.");
        scanf("%d",&choice);
        if(choice==1)
            goto ip_reconnect;
        getch();
        return 1;
    }

    printf("\nConnected to the Server");

    system("cls");
    home_page();

}





int show_menu()
{
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
            printf("\t\t\t|    2--->Register      |\n");
            printf("\t\t\t|    3--->Exit          |\n");
            printf("\t\t\t-------------------------\n\n\n");
            printf("\t\t\t    Enter a Choice\n");
            break;
        case 2:
            printf("\t\t\t-------------------------\n");
            printf("\t\t\t|    1--->Public Chat   |\n");
            printf("\t\t\t|    2--->Log Out       |\n");
            printf("\t\t\t-------------------------\n\n\n");
            printf("\t\t\t    Enter a Choice\n");
            break;
    }
    if(menu==0)
        choice=100;
    else
        scanf("%d",&choice);
    return choice;
}

void home_page()
{
    menu=0;
    show_menu();
    menu=1;
    choice=show_menu();
    switch(choice)
    {
    case 1:
        if(!login_user())
            dashboard();
        else
        break;
    case 2:
        if(!register_user())
            printf("Registration Successful");
        else
            printf("Registration Unsuccessful");
            home_page();
        break;
    case 3:
        exit_user();
        break;
    default:
        home_page();
    }
}


void exit_user()
{
    exit(0);
}

int register_user()
{
    char uname[20];
    char pass[20];
    char ch;
    int i;

    re_uname:
        printf("\t\t    Only alphabets, digits, '@', '_', '.' are allowed maximum 20 characters\n");
        printf("\t\t\t    Enter a Username ---> ");

        fflush(stdin);
        gets(uname);
        if(verify_entry(uname))
        {
            printf("\t\t\t    INVALID USERNAME. PLEASE TRY AGAIN.\n");
            goto re_uname;
        }



    re_pass:
        printf("\t\t    Only alphabets, digits, '@', '_', '.' are allowed maximum 20 characters\n");
        printf("\t\t\t    Enter your Password ---> ");

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

        strcpy(send_msg,"#1011#");
        strcat(send_msg,uname);
        puts(send_msg);

    if( (send(sock , send_msg , strlen(send_msg) , 0) < 0) || ((rcvd_size = recv(sock , rcvd_msg , 1 , 0)) == SOCKET_ERROR))
    {
        printf("Could not connect to server : %d" , WSAGetLastError());
        getch();
        printf("\nPress any key to exit.");
        return 1;
    }

    if(rcvd_msg[0]=='1')
    {
        printf("\t\t\t    USERNAME ALREADY EXISTS. PLEASE TRY AGAIN.\n");
        goto re_uname;
    }

        strcpy(send_msg,"#1012#");
        strcat(send_msg,pass);

    if( (send(sock , send_msg , strlen(send_msg) , 0) < 0) || ((rcvd_size = recv(sock , rcvd_msg , 1 , 0)) == SOCKET_ERROR))
    {
        printf("Could not connect to server : %d" , WSAGetLastError());
        getch();
        printf("\nPress any key to exit.");
        return 1;
    }
    return 0;
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

int login_user()
{
    char uname[20], pass[20];
    int len,i=0;
    char ch;
    int k;


    re_uname:
        printf("\t\t\t    Enter a Username ---> ");
        fflush(stdin);
        gets(uname);
        if(verify_entry(uname))
        {
            printf("\t\t\t    INVALID USERNAME. PLEASE TRY AGAIN.\n");
            goto re_uname;
        }

    re_pass:
        printf("\t\t\t    Enter your Password ---> ");
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
            //putchar(ch)
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
    strcpy(send_msg,"#1013#");
    strcat(send_msg,uname);
    strcat(send_msg,"#1012#");
    strcat(send_msg,pass);

    if( (send(sock , send_msg , strlen(send_msg) , 0) < 0) || ((rcvd_size = recv(sock , rcvd_msg , 1 , 0)) == SOCKET_ERROR))
    {
        printf("Could not connect to server : %d" , WSAGetLastError());
        getch();
        printf("\nPress any key to exit.");
        return 1;
    }
    if(rcvd_msg[0]=='1')
    {
        printf("\t\t\t    USERNAME OR PASSWORD INCORRECT. PLEASE TRY AGAIN.\n");
        goto re_uname;
    }
    strcpy(user,uname);
    return 0;
}

void dashboard()
{
    system("cls");

    menu=0;
    show_menu();
    menu=2;
    show_menu();
    switch(choice)
    {
        case 1:

            strcpy(send_msg,"#1021#");

            if( send(sock , send_msg , strlen(send_msg) , 0) < 0)
            {
                printf("Could not connect to server : %d" , WSAGetLastError());
                getch();
                printf("\nPress any key to exit.");
                exit(0);
            }

            else
                public_chat();
            break;
        case 2:
            strcpy(user,"U-CHAT");
            system("cls");
            printf("LOG OUT SUCCESSFULL\n");
            home_page();
            break;
        default:
            printf("\t\t\t    WRONG CHOICE. PLEASE TRY AGAIN.\n");

    }

}

void public_chat()
{
    HANDLE hThread;
    DWORD dwThreadID;

    system("cls");

    menu=0;
    show_menu();

    hThread = CreateThread(NULL, 0, &ReadingThread, (void*)sock, 0, &dwThreadID);
    if (!hThread)
    {
        printf("Could not create thread : %d" , WSAGetLastError());
        printf("\nPress any key to exit.");
        getch();
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        getch();
        retext();
    printf("YOU : ");
    //Send some data
    fflush(stdin);
    gets(msg);
    strcpy(send_msg,"#1022#");
    if(strcmp(msg,"#1001#")==0)
        strcpy(send_msg,"#1023#");
    else
        strcat(send_msg,msg);

    if( send(sock , send_msg , strlen(send_msg) , 0) < 0)
    {
        printf("Could not connect to server : %d" , WSAGetLastError());
        getch();
        printf("\nPress any key to exit.");
    }

    if(strcmp(msg,"#1001#")==0)
        break;
    }
    CloseHandle(hThread);
    dashboard();
}


DWORD WINAPI ReadingThread(LPVOID param)
{
    SOCKET socks = (SOCKET) param;
    int error_code, i;
    fd_set rcvfds;

    while(1)
    {

        //clear the socket fd set
        FD_ZERO(&rcvfds);

        //add master socket to fd set
        FD_SET(socks, &rcvfds);
         activity = select( 0 , &rcvfds , NULL , NULL , NULL);

        if ( activity == SOCKET_ERROR )
        {
            printf("select call failed with error code : %d" , WSAGetLastError());
            printf("\nPress any key to exit.");
            getch();
            exit(EXIT_FAILURE);
        }
          if (FD_ISSET( socks , &rcvfds))
            {
            rcvd_size = recv( socks , rcvd_msg, 512, 0);
         if( rcvd_size == SOCKET_ERROR)
                {
                    error_code = WSAGetLastError();
                    if(error_code == WSAECONNRESET)
                    {
                        //Somebody disconnected , get his details and print
                        printf("\nConnection Interrupted Unexpectedly %d", error_code);
                        printf("\nPress any key to exit.");
                        getch();
                        exit(EXIT_FAILURE);
                        closesocket( socks );

                    }
                    else
                    {
                        printf("recv failed with error code : %d" , error_code);
                        printf("\nPress any key to exit.");
                        getch();
                        exit(EXIT_FAILURE);
                        closesocket( socks );

                    }
                }
                else if ( rcvd_size == 0)
                {
                    //Somebody disconnected , get his details and print
                    printf("\nServer Disconnected %d", error_code);
                    printf("\nPress any key to exit.");
                    getch();
                    exit(EXIT_FAILURE);
                    closesocket( socks );

                }

                else
                {
                    //adding null character at the end for usage
                    rcvd_msg[rcvd_size] = '\0';
                    retext();
                    printf("%s\a\n",rcvd_msg);
                    printf("YOU : ");
                    //printf("YOU : %s",send_msg);
                    //strcpy(send_msg,"\0");
                }
        }
    }
}


void retext()
{
    int i;
    printf("\r");
    for(i=0;i<50;i++)
        printf(" ");
    for(i=0;i<50;i++)
        printf("\b");
    strcpy(send_msg,"\0");
    fflush(stdin);
}
