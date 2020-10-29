#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CHIP_LENGTH 8   // chip길이는 8비트
#define BIT_LENGTH 5    // 통신하는 정보는 4비트
#define FRAME_LENGTH 33 // chip이 8비트 통신을 4비트라고 가정

/* 전송 char -> int */
int send_bit_char_to_int(char bit);
/* 수신 int -> char */
char receive_bit_int_to_char(int bit);

/* int -> char */
char changebit_int_to_char(int num);
/* char -> int */
int changebit_char_to_int(char num);

/* 수신bit */
char bitReceiveCheck(int bit);

/* 파이프 전송 */
void sendPipe(char buff[FRAME_LENGTH], int pipe, char frame[FRAME_LENGTH]);

/* 파이프 수신, 저장*/
void receivePipe(char buff[FRAME_LENGTH], int pipe, char frame[FRAME_LENGTH]);

/* Print Sequence */
void printSequence(char frame[FRAME_LENGTH]);

/* chip이랑 bit전송할거 합쳐서 프레임에 넣음*/
void changeSequence(char frame[FRAME_LENGTH], char sendBit[BIT_LENGTH], int chip[CHIP_LENGTH]);

/* Join 4 frames*/
void joinSequence(char joinFrame[FRAME_LENGTH], char frameA[FRAME_LENGTH], char frameB[FRAME_LENGTH], char frameC[FRAME_LENGTH], char frameD[FRAME_LENGTH]);

/* 전송받은 프레임이랑 자기 chip 내적*/
void extractionBit(char joinFrame[FRAME_LENGTH], char receiveBit[BIT_LENGTH], int chip[CHIP_LENGTH]);

int main()
{
    // -1 = !,    -2 = @,    -3 = #,   -4 = $
    //Chip A : 0001 1011
    int chipA[CHIP_LENGTH] = {-1, -1, -1, +1, +1, -1, +1, +1};

    //Chip B : 0010 1110
    int chipB[CHIP_LENGTH] = {-1, -1, +1, -1, +1, +1, +1, -1};

    //Chip C : 0101 1100
    int chipC[CHIP_LENGTH] = {-1, +1, -1, +1, +1, +1, -1, -1};

    //Chip D : 0100 0010
    int chipD[CHIP_LENGTH] = {-1, +1, -1, -1, -1, -1, +1, -1};

    //각각의 send data , send frame

    char receiveA[BIT_LENGTH], receiveB[BIT_LENGTH], receiveC[BIT_LENGTH], receiveD[BIT_LENGTH];

    int pipes_sender[2];
    int pipes_joiner_1[2];

    char buff[FRAME_LENGTH];
    buff[FRAME_LENGTH] = '\0';
    int counter = 0;
    pid_t pid_sender, pid_receiver;

    //send - join파이프 생성
    if (-1 == pipe(pipes_sender))
        exit(1);
    if (-1 == pipe(pipes_joiner_1))
        exit(1);

    pid_sender = fork();

    if (pid_sender > 0)
    { //send 하는 부모 프로세스

        char sendA[BIT_LENGTH] = "1101", sendB[BIT_LENGTH] = "1 01", sendC[BIT_LENGTH] = "1111", sendD[BIT_LENGTH] = "00 1";
        char frameA[FRAME_LENGTH], frameB[FRAME_LENGTH], frameC[FRAME_LENGTH], frameD[FRAME_LENGTH];

        //전송할 데이터 입력
        printf("전송하는 데이터 ..  \nA : %s, B : %s, C : %s, D : %s\n\n", sendA, sendB, sendC, sendD);

        //chip으로 변환
        changeSequence(frameA, sendA, chipA);
        changeSequence(frameB, sendB, chipB);
        changeSequence(frameC, sendC, chipC);
        changeSequence(frameD, sendD, chipD);

        //변환된거 출력
        printf("A : %s >> ", sendA);
        printSequence(frameA);
        printf("B : %s >> ", sendB);
        printSequence(frameB);
        printf("C : %s >> ", sendC);
        printSequence(frameC);
        printf("D : %s >> ", sendD);
        printSequence(frameD);

        //pipe 전송
        sendPipe(buff, pipes_sender[1], frameA);
        sendPipe(buff, pipes_sender[1], frameB);
        sendPipe(buff, pipes_sender[1], frameC);
        sendPipe(buff, pipes_sender[1], frameD);
    }
    else
    {
        //join - receiver 파이프 생성
        int pipes_joiner_2[2];
        int pipes_receiver[2];
        if (-1 == pipe(pipes_joiner_2))
            exit(1);
        if (-1 == pipe(pipes_receiver))
            exit(1);
        pid_receiver = fork();

        if (pid_receiver > 0)
        {
            printf("\n\n start join ... \n\n");
            char frameA[FRAME_LENGTH], frameB[FRAME_LENGTH], frameC[FRAME_LENGTH], frameD[FRAME_LENGTH];
            char joinFrame[FRAME_LENGTH];
            // int counter = 0;
            // read(pipes_sender[0], buff, BUFF_SIZE);
            // //join하는 자식 프로세스
            int count = 0;
            receivePipe(buff, pipes_sender[0], frameA);
            receivePipe(buff, pipes_sender[0], frameB);
            receivePipe(buff, pipes_sender[0], frameC);
            receivePipe(buff, pipes_sender[0], frameD);

            joinSequence(joinFrame, frameA, frameB, frameC, frameD);
            puts("");
            printSequence(joinFrame);
            sendPipe(buff, pipes_joiner_2[1], joinFrame);
            printf("join Finish..\n\n");
        }
        else
        {
	        char receiveA[BIT_LENGTH], receiveB[BIT_LENGTH], receiveC[BIT_LENGTH], receiveD[BIT_LENGTH];
            char joinFrame[FRAME_LENGTH];
            receivePipe(buff, pipes_joiner_2[0], joinFrame);
            printf("\n\n start receive\n\n");
            extractionBit(joinFrame, receiveA, chipA);
            extractionBit(joinFrame, receiveB, chipB);
            extractionBit(joinFrame, receiveC, chipC);
            extractionBit(joinFrame, receiveD, chipD);

            printf("수신된 데이터 ..  \nA : %s, B : %s, C : %s, D : %s\n\n", receiveA, receiveB, receiveC, receiveD);
            printf("finish receive..\n");
        }
    }

    return 0;
}

int send_bit_char_to_int(char bit)
{
    //전송용
    if (bit == '1')
        return 1;
    else if (bit == '0')
        return -1;
    else if (bit == ' ')
        return 0;
}

char receive_bit_int_to_char(int bit)
{
    //전송용
    if (bit == 1)
        return '1';
    else if (bit == 0)
        return ' ';
    else if (bit == -1)
        return '!';
}

char change_int_to_char(int num)
{
    //전송할때 chip계산위해서
    switch (num)
    {
    case 0:
        return '0';
    case 1:
        return '1';
    case 2:
        return '2';
    case 3:
        return '3';
    case 4:
        return '4';
    case -1:
        return '!';
    case -2:
        return '@';
    case -3:
        return '#';
    case -4:
        return '$';
    }
}

int change_char_to_int(char num)
{
    //전송할때 chip계산위해서
    switch (num)
    {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '!':
        return -1;
    case '@':
        return -2;
    case '#':
        return -3;
    case '$':
        return -4;
    }
}

char bitReceiveCheck(int bit)
{
    bit = bit / 8;
    if (bit == 1)
        return '1';
    else if (bit == -1)
        return '0';
    else if (bit == 0)
        return ' ';
}

void printSequence(char frame[FRAME_LENGTH])
{
    printf("%32s\n", frame);
}

void changeSequence(char frame[FRAME_LENGTH], char sendBit[BIT_LENGTH], int chip[CHIP_LENGTH])
{
    for (int i = 0; i < BIT_LENGTH - 1; i++)
    {
        int flag = send_bit_char_to_int(sendBit[i]);
        for (int j = 0; j < CHIP_LENGTH; j++)
        {
            frame[i * CHIP_LENGTH + j] = change_int_to_char(chip[j] * flag);
        }
    }
    frame[32] = '\0';
}

void sendPipe(char buff[FRAME_LENGTH], int pipe, char frame[FRAME_LENGTH])
{
    sprintf(buff, "%s", frame);
    write(pipe, buff, strlen(buff));
    memset(buff, 0, FRAME_LENGTH - 1);
}

void receivePipe(char buff[FRAME_LENGTH], int pipe, char frame[FRAME_LENGTH])
{
    read(pipe, buff, FRAME_LENGTH - 1);
    buff[FRAME_LENGTH] = '\0';
    strcpy(frame, buff);
    printf("%s\n",frame);
    memset(buff, 0, FRAME_LENGTH);
    sleep(1);
}

void joinSequence(char joinFrame[FRAME_LENGTH], char frameA[FRAME_LENGTH], char frameB[FRAME_LENGTH], char frameC[FRAME_LENGTH], char frameD[FRAME_LENGTH])
{
    for (int i = 0; i < FRAME_LENGTH; i++)
    {
        int sum = change_char_to_int(frameA[i]) + change_char_to_int(frameB[i]) + change_char_to_int(frameC[i]) + change_char_to_int(frameD[i]);
        joinFrame[i] = change_int_to_char(sum);
    }
}

void extractionBit(char joinFrame[FRAME_LENGTH], char receiveBit[BIT_LENGTH], int chip[CHIP_LENGTH])
{
    for (int i = 0; i < BIT_LENGTH - 1; i++)
    {
        int sum = 0;
        for (int j = 0; j < CHIP_LENGTH; j++)
        {
            sum += change_char_to_int(joinFrame[i * CHIP_LENGTH + j]) * chip[j];
        }
        // 1 -> 1
        // -1 -> 0
        // 0 -> 공백
        receiveBit[i] = bitReceiveCheck(sum);
    }
    receiveBit[4] = '\0';
}