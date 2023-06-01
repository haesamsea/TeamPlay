//server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>

#define MAX  512
#define MAX_SOCK 1024 
#define BROADCAST_INTERVAL 60 // seconds

char* EXIT_STRING2 = "Exit";
char* START_STRING = "\033[1;35mWelcome to '시프해듀오'!!!\n<대화 시작전 본인의 나이와 거주지 정보를위해 D1을 입력해주세요>\n<대화 요소가 떨어질때는 아래의 Q1 ~ Q6까지 선택하여 입력>\033[0m\n";

//매칭 후 전달하는 포트 번호
//포트 번호를 더 다양하게 할수도 있음 다만 그렇게하면
//현재 상태에서 정확한 매칭 구현 불가 
int chatting_port1[] = {
    9090
};
int chatting_port2[] = {
    9191
};
int chatting_port3[] = {
    9292
};
int new_portnum1 = sizeof(chatting_port1) / sizeof(chatting_port1[0]);
int new_portnum2 = sizeof(chatting_port2) / sizeof(chatting_port2[0]);
int new_portnum3 = sizeof(chatting_port2) / sizeof(chatting_port3[0]);

int maxsocketD;  //D for descriptor
int num_client = 0;         //참가 client수
int chatting_Num = 0;         // 지금까지 오간 대화의 수
int socket_list[MAX_SOCK];
int listen_socket;         // 서버의 리슨 소켓
char ip_list[MAX_SOCK][20];

void ClientIn(int s, struct sockaddr_in* newclient_add);
int compare();
void ClientOut(int s);
int listening(int host, int port, int backlog);
void errquit(char* mesg) { perror(mesg); exit(1); }
//추가
void signalhandler(int signal);

int flag = 0;

time_t current_time;
struct tm tm;

int main(int argc, char* argv[]) {
    //client address 구조체
    struct sockaddr_in client_add;
    char buf[MAX + 1];
    int i, j, byte, accept_socket, addrlen = sizeof(struct sockaddr_in);
    fd_set read_fds;   //socket의 배열, 변화(client의 요청)감지

    pid_t pid = getpid(); // 현재 프로세스 아이디 받아오기
    printf("프로세스 아이디: %d\n", pid);

    if (argc != 2) {
        printf("사용법 :%s <Port_num>\n", argv[0]);
        exit(0);
    }
    //signal handling//
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signalhandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    signal(SIGINT, signalhandler);
    signal(SIGQUIT, signalhandler);
    signal(SIGTERM, signalhandler);
    //listening(host, port, backlog) 함수 호출
    listen_socket = listening(INADDR_ANY, atoi(argv[1]), 5);
    //thread 활용해서 보다 정확한 알람 서비스의 구현

    time_t last_broadcast = time(NULL);

    while (1) {
        struct timeval tv;

        tv.tv_sec = BROADCAST_INTERVAL;
        tv.tv_usec = 0;
        FD_ZERO(&read_fds); //FD를 0으로 setting
        FD_SET(listen_socket, &read_fds); //listen_socket
        for (i = 0; i < num_client; i++)
            FD_SET(socket_list[i], &read_fds);

        maxsocketD = compare() + 1;
        if (select(maxsocketD, &read_fds, NULL, NULL, &tv) < 0) //변화 ok : 양수
            if (errno == EINTR) {
                continue;
            }
            else {
                errquit("select fail");
            }
        //accept
        if (FD_ISSET(listen_socket, &read_fds)) {
            accept_socket = accept(listen_socket,
                (struct sockaddr*) & client_add, &addrlen);
            if (accept_socket == -1)
                errquit("accept fail");
            //1 client in
            ClientIn(accept_socket, &client_add);
            write(accept_socket, START_STRING, strlen(START_STRING));
            current_time = time(NULL);         //current time
            tm = *localtime(&current_time);

            write(1, "\033[0G", 4);     //커서 이동
            printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
            fprintf(stderr, "\033[33m");//YELLOW

            printf("1 Client has entered. Updated Current Client = %d\n", num_client);
            fprintf(stderr, "\033[32m");//GREEN
            fprintf(stderr, "Server >"); //커서 출력
        }
        for (i = 0; i < num_client; i++) {
            if (FD_ISSET(socket_list[i], &read_fds)) {

                byte = read(socket_list[i], buf, MAX); //receive
                if (byte <= 0) {
                    ClientOut(i);
                    continue;
                }
                buf[byte] = 0;
                /* if (strncmp(buf, "20대", 2) == 0)
                 {
                     for (i = 0; i < num_client; i++)
                         write(socket_list[i], "\033[31m20대에게 추천드리는 선물\n[1.카드지갑 2.기프티콘 3.디퓨저\n4.폰 케이스 5.이어폰 6.폴라로이드 카메라]\n\033[0m", strlen("\033[35m20대에게 추천드리는 선물\n[1.카드지갑 2.기프티콘 3.디퓨저\n4.폰 케이스 5.이어폰 6.폴라로이드 카메라]\n\033[0m"), 0);
                 }*/
                 //Daegu 뿐만 아니라 다른 지역도 생성가능  
                if (strncmp(buf, "Daegu 24", 2) == 0) {
                    int random_index = rand() % new_portnum1;
                    char port_message[100];
                    snprintf(port_message, sizeof(port_message), "\033[31m대구에 사시는 24살 당신, 다음 포트로 재접속해주세요: %d\n\033[0m", chatting_port1[random_index]);

                    for (i = 0; i < num_client; i++) {
                        write(socket_list[i], port_message, strlen(port_message));
                    }
                    //port 번호를 index로 받아옴
                    printf("%d\n", chatting_port1[random_index]);
                }
                else if (strncmp(buf, "Busan 31", 2) == 0) {
                    int random_index = rand() % new_portnum2;
                    char port_message[100];
                    snprintf(port_message, sizeof(port_message), "\033[31m부산에 사시는 31살 당신, 다음 포트로 재접속해주세요: %d\n\033[0m", chatting_port2[random_index]);

                    for (i = 0; i < num_client; i++) {
                        write(socket_list[i], port_message, strlen(port_message));
                    }

                    printf("%d\n", chatting_port2[random_index]);
                }
                else if (strncmp(buf, "Seoul 40", 2) == 0) {
                    int random_index = rand() % new_portnum3;
                    char port_message[100];
                    snprintf(port_message, sizeof(port_message), "\033[31m서울에 사시는 40살 당신, 다음 포트로 재접속해주세요: %d\n\033[0m", chatting_port3[random_index]);

                    for (i = 0; i < num_client; i++) {
                        write(socket_list[i], port_message, strlen(port_message));
                    }

                    printf("%d\n", chatting_port3[random_index]);
                }
                //20대 뿐만 아니라 다른 나이대도 생성가능
                /*else if (strncmp(buf, "24", 2) == 0) {
                    int random_index = rand() % new_portnum;
                    char port_message[100];
                    snprintf(port_message, sizeof(port_message), "\033[31m24살인 당신, 다음 포트로 재접속해주세요: %d\n\033[0m", chatting_port[random_index]);

                    for (i = 0; i < num_client; i++) {
                        write(socket_list[i], port_message, strlen(port_message), 0);
                    }

                    printf("%d\n", chatting_port[random_index]);
                }*/
                else if (strncmp(buf, "Q1", 2) == 0) {
                    for (i = 0; i < num_client; i++)
                        write(socket_list[i], "\033[31mQ1. 당신이 가장 가보고싶은 여행지는 어디인가요?\n\033[0m", strlen("\033[35mQ1. 당신이 가장 가보고싶은 여행지는 어디인가요?\n\033[0m"));
                }
                else if (strncmp(buf, "Q2", 2) == 0) {
                    for (i = 0; i < num_client; i++)
                        write(socket_list[i], "\033[31mQ2. 당신의 이상형을 알려주세요.\n\033[0m", strlen("\033[35mQ2. 당신의 이상형을 알려주세요.\n\033[0m"));
                }
                else if (strncmp(buf, "Q3", 2) == 0) {
                    for (i = 0; i < num_client; i++)
                        write(socket_list[i], "\033[31mQ3. 당신은 연애경험이 많은 사람인가요?\n\033[0m", strlen("\033[35mQ3. 당신은 연애경험이 많은 사람인가요?\n\033[0m"));
                }
                else if (strncmp(buf, "Q4", 2) == 0) {
                    for (i = 0; i < num_client; i++)
                        write(socket_list[i], "\033[31mQ4. 당신은 화가나면 어떻게 푸는 사람인가요?\n\033[0m", strlen("\033[35mQ4. 당신은 화가나면 어떻게 푸는 사람인가요?\n\033[0m"));
                }
                else if (strncmp(buf, "Q5", 2) == 0) {  // 클라이언트가 q5을 선택한 경우
                    for (i = 0; i < num_client; i++)
                        write(socket_list[i], "\033[31mQ5. 당신의 MBTI는 무엇인가요?\n\033[0m", strlen("\033[33mQ5. 당신의 MBTI는 무엇인가요?\n\033[0m\n"));
                }
                else if (strncmp(buf, "Q6", 2) == 0) {  // 클라이언트가 q5을 선택한 경우
                    for (i = 0; i < num_client; i++)
                        write(socket_list[i], "\033[31mQ6. 내일 상대방을 만날 의향이 있나요?\n Yes: Day1 from today / No: Exit\n\033[0m", strlen("\033[33mQ6. 내일 상대방을 만날 의향이 있나요?\n Yes: Day1 from today / No: Exit\033[0m\n"));
                }
                else if (strncmp(buf, "D1", 2) == 0) {  // 클라이언트가 q5을 선택한 경우
                    for (i = 0; i < num_client; i++)
                        write(socket_list[i], "\033[31mD1. 당신의 거주지역과 나이는?\n\033[0m", strlen("\033[33mD1.  당신의 거주지역과 나이는?\n\033[0m\n"));
                }

                else {
                    if (strstr(buf, EXIT_STRING2) != NULL) {
                        ClientOut(i);   // 종료
                        continue;
                    }
                    for (j = 0; j < num_client; j++)
                        write(socket_list[j], buf, byte);
                    printf("\033[0G");      //커서 이동
                    fprintf(stderr, "\033[97m");//WHITE
                    printf("%s", buf);
                    fprintf(stderr, "\033[32m");//GREEN
                    fprintf(stderr, "Server >");

                }
            }
        }
        //keep_talking : time handling
        // Check if it's time to broadcast
        time_t now = time(NULL);
        //마지막으로 write한 시간이 60초가 지나면 문자열 write
        if (difftime(now, last_broadcast) >= BROADCAST_INTERVAL) {
            char* msg = "\033[33mkeep talking\033[0m\n";
            for (i = 0; i < num_client; i++)
                write(socket_list[i], msg, strlen(msg));
            last_broadcast = now;
        }
    }
    return 0;
}

int  listening(int host, int port, int backlog) {
    int socket_D;
    struct sockaddr_in server_add;
    //socket create
    socket_D = socket(PF_INET, SOCK_STREAM, 0);

    if (socket_D == -1) {
        perror("socket fail");
        exit(1);
    }
    //binding: socket내부에 구조체형식으로 주소정보 넣음(sin~)
    bzero((char*)&server_add, sizeof(server_add));
    server_add.sin_family = AF_INET;
    server_add.sin_addr.s_addr = htonl(host);
    server_add.sin_port = htons(port);
    if (bind(socket_D, (struct sockaddr*) & server_add, sizeof(server_add)) < 0) {
        perror("bind fail");  exit(1);
    }
    //connection 요청 기다림 from client
    //listening
    listen(socket_D, backlog);
    return socket_D;
}

//추가
void signalhandler(int signal) {
    if (signal == SIGINT) {
        printf("  No one left in the chatting Room...\n");
        sleep(1);
        return;
    }
    else if (signal == SIGQUIT) {

        printf("   Do you want to leave the Server?...\n");
        sleep(1);
        return;
    }
    else if (signal == SIGTERM) {
        printf("   Server cannot leave the  chatting Room...\n");
        sleep(1);
        return;
    }
}
// 입장하는 client
void ClientIn(int s, struct sockaddr_in* newclient_add) {
    char buf[20];
    inet_ntop(AF_INET, &newclient_add->sin_addr, buf, sizeof(buf));
    write(1, "\033[0G", 4);      //커서이동
    fprintf(stderr, "\033[33m");   //YELLOW
    printf("new client: %s\n", buf);//print IP address
    //client socket list에 새로운 client 추가
    socket_list[num_client] = s;
    strcpy(ip_list[num_client], buf);

    num_client++; //client in
}

// client 퇴장
void ClientOut(int s) {
    close(socket_list[s]);
    if (s != num_client - 1) {
        socket_list[s] = socket_list[num_client - 1];
        strcpy(ip_list[s], ip_list[num_client - 1]);

    }
    num_client--; //client out
    current_time = time(NULL);         //current time
    tm = *localtime(&current_time);
    write(1, "\033[0G", 4);      //커서 이동
    fprintf(stderr, "\033[34m"); //BLUE
    printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
    printf("1 Client has exited. Updated Current Client = %d\n", num_client);
    fprintf(stderr, "\033[32m");//GREEN
    fprintf(stderr, "Server >");

    fflush(stdout);

}

int compare() {
    int max = listen_socket;
    int i;
    for (i = 0; i < num_client; i++)
        if (socket_list[i] > max)
            max = socket_list[i];
    return max;
}