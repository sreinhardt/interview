#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>

#if defined(__x86_64)
  #define PAD(sz) char pad[sz];
#else
  #define PAD(sz) do { } while(0);
#endif

#define PORTNO 12346
#define BUFFER_SIZE 512

int h=0, b=0, p=0;

int checksec(FILE *f) {
  FILE *key;
  char buf[1024];

  if (h & b & p) {
    key = fopen("flag.txt", "r");
    fread(buf, 1024, 1, key);
    fprintf(f, "%s", buf);
    fclose(key);
    return 0;
  }
  return 1;
}

void hekers(FILE *f) {
  PAD(12);
  volatile int zeroWeekend;
  char buf[32];

  fprintf(f, "So you want to be an 31337 Hax0r?\n");
  fgets(buf, 40, f);

  switch (strncmp("y3$\n", buf, 3)) {
    case 0:
      fprintf(f, "First you must get power\n");
      break;
    default:
      fprintf(f, "Well then go away\n");
      break;
  }

  if (zeroWeekend == 0xcafebabe)
    h = 1;
}

void batmenss(FILE *f) {
  PAD(12);
  volatile int batsignet;
  char buf[32];

  fprintf(f, "So you want to be the batman?\n");
  fgets(buf, 40, f);

  switch (strncmp("YESSSSSSS\n", buf, 9)) {
    case 0:
      fprintf(f, "First you must get rich\n");
      break;
    default:
      fprintf(f, "Well then go away\n");
      break;
  }

  if (batsignet == 0x12345678)
    b = 1;
}

void pokemans(FILE *f) {
  PAD(12);
  volatile int pikachy;
  char buf[32];

  fprintf(f, "So you want to be the best there ever was?\n");
  fgets(buf, 40, f);

  switch (strncmp("catchemall\n", buf, 10)) {
    case 0:
      fprintf(f, "First you must get respect\n\n");
      break;
    default:
      fprintf(f, "Well then go away\n");
      break;
  }

  if (pikachy == 0xfa75beef)
    p = 1;
}

void readInput(int sock) {
  int msg;
  char choice[4];
  char buffer[BUFFER_SIZE];
  FILE *fptr = fdopen(sock, "r+");
  char *prompt = "Do you want to be a?\n"
                 "1.) Pokemon Master\n"
                 "2.) Elite Hacker\n"
                 "3.) The Batman\n";

  while (checksec(fptr)) {

    fprintf(fptr, "%s", prompt);
    fgets(choice, 4, fptr);

    switch (choice[0]) {
      case '1':
        pokemans(fptr);
        break;
      case '2':
        hekers(fptr);
        break;
      case '3':
        batmenss(fptr);
        break;
      default:
        fprintf(fptr, "\nThat is not one of the choices\n");
    }

    fflush(fptr);
  }

  fprintf(fptr, "%s", buffer);
  fflush(fptr);
  fclose(fptr);
}

int main(int argc, char *argv[]) {
  char buffer[BUFFER_SIZE];
  int sockfd, newsockfd, portno, pid;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("ERROR opening socket");
    exit(1);
  }

  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
  serv_addr.sin_port = htons(PORTNO);

  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR on binding");
    exit(1);
  }
  listen(sockfd, 5);
  clilen = sizeof(cli_addr);

  while (1) {
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      perror("ERROR on accept");
    
    pid = fork();
    if (pid < 0)
      perror("ERROR on fork");
    if (pid == 0) {
      close(sockfd);
      readInput(newsockfd);
      goto exit;
    } else
      close(newsockfd);
    
    waitpid(-1, NULL, WNOHANG);
  }
  close(sockfd);

exit:
  return 0;
}
