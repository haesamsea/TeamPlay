//client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>

#define MAX     1024
#define MAXNAME    20
typedef struct {
    char name[50];
    int age;
    char gender[10];
    char residence[50];
} UserInfo;

//random date course recommendation 
char* date_courses[] = {
    "수성못: Pizza & Cafe",
    "앞산: 등산 & Cafe ",
    "두류공원: 이월드 & 야외음악당",
    "서문시장: 시장구경 & 맛있는 음식",
    "동성로: 다양한 즐길거리 & 먹거리",
    "신세계 백화점: 다양한 음식 & 영화관",
    "강정보: 액티비티",
    "경북대학교: 센트럴파크",
    "경산: Cafe & 이웃집수달 동물원"
};
int num_date_courses = sizeof(date_courses) / sizeof(date_courses[0]);

//ending statement
char* EXIT_STRING1 = "Day1 from today";
char* EXIT_STRING2 = "Exit";

int connection(int af, char* servip, unsigned short port);
//error handling
void errquit(char* mesg) { perror(mesg); exit(1); }
// SIGINT IGNORE SIGNAL HANDLER
void signalhandler(int signal);

int main(int argc, char* argv[]) {
    srand(time(NULL));

    char bufname[MAXNAME];
    char bufmsg[MAX];
    char bufall[MAX + MAXNAME];
    int maxsocketD;
    int sock;

    fd_set read_fds;
    time_t current_time;
    struct tm tm;

    pid_t pid = getpid(); // 현재 프로세스 아이디 받아오기
    printf("프로세스 아이디: %d\n", pid);

    if (argc != 4) {
        printf("사용법 : %s <server_ip>  <Port_num> <nick_name> \n", argv[0]);
        exit(0);
    }

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

    sock = connection(PF_INET, argv[1], atoi(argv[2]));
    if (sock == -1)
        errquit("connection fail");

    printf("---------------------------------------------------\n");
    printf("              채팅을 시작합니다                       \n");
    printf("          여러분의 사랑을 응원합니다                  \n");
    printf("  대화 주제가 부족할 시, question을 입력해주세요!        \n");
    printf("----------------------------------------------------\n");

    //서버 접속
    maxsocketD = sock + 1;
    FD_ZERO(&read_fds);

    while (1) {
        FD_SET(0, &read_fds);
        FD_SET(sock, &read_fds);
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        if (select(maxsocketD, &read_fds, NULL, NULL, &timeout) < 0)
            if (errno == EINTR) {
                continue;
            }
            else {
                errquit("select fail");
            }

        if (FD_ISSET(sock, &read_fds)) {
            int byte;
            if ((byte = read(sock, bufmsg, MAX)) > 0) {
                bufmsg[byte] = 0;
                write(1, "\033[0G", 4);      //커서 이동
                printf("%s", bufmsg);      // Print the message
                fprintf(stderr, "\033[1;32m"); // make GREEN
                fprintf(stderr, "\033[33m%s> \033[0m", argv[3]); // Print my nickname
            }
        }
        if (FD_ISSET(0, &read_fds)) {
            signal(SIGINT, SIG_IGN); // SIGINT 시그널 무시
            signal(SIGTERM, SIG_IGN); // SIGTERM 시그널 무시
            signal(SIGQUIT, SIG_IGN); // SIGQUIT 시그널 무시

            signal(SIGINT, signalhandler);
            signal(SIGQUIT, signalhandler);
            signal(SIGTERM, signalhandler);
            int i, len, found;
            if (fgets(bufmsg, MAX, stdin)) {
                fprintf(stderr, "\033[1;33m"); // YELLOW
                fprintf(stderr, "\033[1A");   // y -1만큼 이동
                current_time = time(NULL);   // current time
                tm = *localtime(&current_time);
                sprintf(bufall, "<<Current_Time: Hour : Min : Sec %02d:%02d:%02d>>%s>%s", tm.tm_hour, tm.tm_min, tm.tm_sec, argv[3], bufmsg); //current time 출력
                /*
                if (strcmp(bufmsg, "question\n") == 0) {  // 사용자가 "question"을 입력한 경우
                    if (write(sock, "question", strlen("question"), 0) < 0)
                        puts("Error: Write error on socket.");
                }*/
                /*  len = strlen(bufmsg);
                  found = 0;
                  for (i = 0; i < len; i++) {
                      if (bufmsg[i] == "2") {
                          found = 1;
                          break;
                      }
                  }
                  if(found)
                  {
                      if (write(sock, "20대", strlen("20대"), 0) < 0)
                          puts("Error: Write error on socket.");
                  }*/

                if (strcmp(bufmsg, "Daegu 24\n") == 0) {  // 클라이언트가 Q1을 선택한 경우
                    if (write(sock, "Daegu 24", strlen("Daegu 24")) < 0)
                        puts("Error: Write error on socket.");
                }
                else if (strcmp(bufmsg, "Busan 31\n") == 0) {  // 클라이언트가 Q1을 선택한 경우
                    if (write(sock, "Busan 31", strlen("Busan 31")) < 0)
                        puts("Error: Write error on socket.");
                }
                else if (strcmp(bufmsg, "Seoul 40\n") == 0) {  // 클라이언트가 Q1을 선택한 경우
                    if (write(sock, "Seoul 40", strlen("Seoul 40")) < 0)
                        puts("Error: Write error on socket.");
                }
                else if (strcmp(bufmsg, "Q1\n") == 0) {  // 클라이언트가 Q1을 선택한 경우
                    if (write(sock, "Q1", strlen("Q1")) < 0)
                        puts("Error: Write error on socket.");
                }
                else if (strcmp(bufmsg, "Q2\n") == 0) {  // 클라이언트가 Q2을 선택한 경우
                    if (write(sock, "Q2", strlen("Q2")) < 0)
                        puts("Error: Write error on socket.");
                }
                else if (strcmp(bufmsg, "Q3\n") == 0) {  // 클라이언트가 Q3을 선택한 경우
                    if (write(sock, "Q3", strlen("Q3")) < 0)
                        puts("Error: Write error on socket.");
                }
                else if (strcmp(bufmsg, "Q4\n") == 0) {  // 클라이언트가 Q4을 선택한 경우
                    if (write(sock, "Q4", strlen("Q4")) < 0)
                        puts("Error: Write error on socket.");
                }
                else if (strcmp(bufmsg, "Q5\n") == 0) {  // 클라이언트가 Q5을 선택한 경우
                    if (write(sock, "Q5", strlen("Q5")) < 0)
                        puts("Error: Write error on socket.");
                }
                else if (strcmp(bufmsg, "Q6\n") == 0) {  // 클라이언트가 Q6을 선택한 경우
                    if (write(sock, "Q6", strlen("Q6")) < 0)
                        puts("Error: Write error on socket.");
                }
                else if (strcmp(bufmsg, "D1\n") == 0) {  // 클라이언트가 Q6을 선택한 경우
                    if (write(sock, "D1", strlen("D1")) < 0)
                        puts("Error: Write error on socket.");
                }
                //else if (strcmp(bufmsg, "Woman\n") == 0) {  // 클라이언트가 Q6을 선택한 경우
                //    if (write(sock, "Woman", strlen("Woman"), 0) < 0)
                //        puts("Error: Write error on socket.");
                //}
                //else if (strcmp(bufmsg, "Man\n") == 0) {  // 클라이언트가 Q6을 선택한 경우
                //    if (write(sock, "Man", strlen("Man"), 0) < 0)
                //        puts("Error: Write error on socket.");
            }
            if (write(sock, bufall, strlen(bufall)) < 0)
                puts("Error: Write error on socket.");


            if (strstr(bufmsg, EXIT_STRING1) != NULL) {
                printf("\n");
                printf("\033[31m        \n");
                printf("\033[31m  ***   ***  \n");
                printf("\033[31m ***** ***** \n");
                printf("\033[31m*************\n");
                printf("\033[31m *********** \n");
                printf("\033[31m  *********  \n");
                printf("\033[31m   *******   \n");
                printf("\033[31m    *****    \n");
                printf("\033[31m     ***     \n");
                printf("\033[31m      *      \033[0m\n");

                int random_index = rand() % num_date_courses;
                printf("Recommended Date course : \n");
                printf("%s\n", date_courses[random_index]);

                close(sock);
                exit(0);
            }
            else if (strstr(bufmsg, EXIT_STRING2) != NULL) {
                char buffer[1024];
                // "Exit"를 입력한 경우
                snprintf(buffer, sizeof(buffer), "%s가 exit 하였습니다\n", argv[3]);
                if (write(sock, buffer, strlen(buffer)) < 0) {
                    puts("Error: Writing error on socket.");
                }
                close(sock);
                exit(1);
            }
        }
    }
}

int connection(int af, char* servip, unsigned short port) {
    struct sockaddr_in server_add;
    int  sockt;

    //create socket
    if ((sockt = socket(af, SOCK_STREAM, 0)) < 0)
        return -1;
    //구조체 초기화
    bzero((char*)&server_add, sizeof(server_add));
    server_add.sin_family = af;
    inet_pton(AF_INET, servip, &server_add.sin_addr);
    server_add.sin_port = htons(port);

    //connecting call
    if (connect(sockt, (struct sockaddr*) & server_add, sizeof(server_add))
        < 0)
        return -1;
    return sockt;
}

void signalhandler(int signal) {
    if (signal == SIGINT) {
        printf("  You cannot leave before the end of chatting...\n");
        sleep(1);
        return;
    }
    else if (signal == SIGQUIT) {

        printf("  Where are you going?...\n");
        sleep(1);
        return;
    }
    else if (signal == SIGTERM) {
        printf("  Please check the opponents chat...\n");
        sleep(1);
        return;
    }
}