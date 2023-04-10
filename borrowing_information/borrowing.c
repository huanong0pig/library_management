//
// Created by 22298 on 2023/3/21.
//

#include "borrowing.h"
#include<stdio.h>
#include <malloc.h>
#include <string.h>
#include "time.h"

Records * init_Records(){
    Records * L;
    L = (Records *)malloc(sizeof(Records));

    if(L != NULL){
        L -> last = -1;
    }
    return L;
}

//传入读者ID,书籍ID
//借书时产生的借阅信息存储在record.csv中，由于一个人可以借用多本书，所以其中的userid信息会重复
//程序启动时时，我们需要将csv中的信息用链表形式存到内存中使用
//链表中的userid信息必须是唯一的，所以多本书需要以列表形式存储 info[4]

// borrowing book
int addRecord(Records * L, struct readerRecord rd){

    if(L -> last >= RECORD_MAXSIZE - 1){
        return 0;
    }
//    初始化借书数量为0
    rd.bookCounter = 0;

    L -> readerRecord[L -> last] = rd;
    L -> last = L -> last + 1;

    return L ->last;
}

//用户归还所有书时消除记录
//i 是数组索引，而不是真实位置（就是从0开始数）
int deleteRecord(Records * L, int i){
    if(i < 0|| i > L -> last){
        return 0;
    }
    else
    {
        //从顺序表中的i开始,直接按顺序将后一个元素覆盖前一个元素,最终只有i消失了
        for (int j = i; j < L -> last; j++)
            L -> readerRecord[j] = L -> readerRecord[j + 1];
        L ->last = L ->last - 1;
    }
    return 1;
}

//查找用户借阅记录,返回顺序表的下标值
int checkReaderIndex(Records * L, char * readerID){
    int i, result = -1;
    for(i = 0; i < L -> last; i++){
        result = strcmp(L -> readerRecord[i].readerID, readerID);

        if(result == 0)
            return i;
    }
    return -1;
}

//给外部调用的接口，查看借阅信息
struct readerRecord checkRecords(Records *L, char *readerID){
    int readerindex = checkReaderIndex(L, readerID);
    struct readerRecord record = L ->readerRecord[readerindex];
    return record;
}

//修改records.csv文件,作用是对顺序表的修改同步到文件中
//1表示增, 2表示删, 不需要设置查,查询直接从内存查寻就行, 文件内容始终和内存保持同步

int saveFile(Records *L, char *filename){

    FILE *fp = fopen(filename, "w");

    char record[RECORD_LEN] = "";

    char readerID[11] = "";
    char isbn[13] = "";
    char borrowingDate[8] = "";

    for(int i = 0; i < L ->last + 1; i++ )
        for(int j = 0; j < L ->readerRecord[i].bookCounter; j++){
            strcpy(readerID, L ->readerRecord[i].readerID);
            strcpy(isbn, L ->readerRecord[i].bookInfo[j].ISBN);
            strcpy(borrowingDate, L ->readerRecord[i].bookInfo[j].borrowingDate);

            sprintf(record, "%s,%s,%s\n", readerID, isbn, borrowingDate);
            fputs(record, fp);
        }

    fclose(fp);
}

//专门用于初始化结构体 bookInfo中的成员 borrowingDate
char* formatDate(int opt){
    time_t seconds;

    if(opt == 1){
        seconds = seconds + LENDING_LIM;
    }

    time(&seconds);

    struct tm *now;

//    int year = now ->tm_year;
    char date[8], year[4], month[2], day[2];

    sprintf(year, "%4d", now ->tm_year);
    sprintf(month, "%02d", now ->tm_mon);
    sprintf(day, "%02d", now ->tm_mday);

    strcat(date, year);
    strcat(date, month);
    strcat(date, day);

    return date;
}

//  初始化借阅的书籍信息, 借书时需要调用
struct bookInfo initBookInfo(char *isbn){

    struct bookInfo bookinfo;
    strcpy(bookinfo.ISBN, isbn);
    strcpy(bookinfo.borrowingDate, formatDate(0));
    strcpy(bookinfo.returningDate, formatDate(1));

    return bookinfo;
}

//添加书籍, readerIndex是用户在顺序表中的索引值, 我们用它来指示添加书籍信息的位置
int addBookInfo(Records * L, int readerIndex, char *isbn){
    //amount用来表示用户已经借的书籍数量
    int * bookAmount = &(L ->readerRecord[readerIndex].bookCounter);
    struct bookInfo *book = (L ->readerRecord[readerIndex].bookInfo);

    // 如果amount = 2说明用户已经接了两本书,那么新书信息应该更新在bookInfo[2]中, 即
    *(book + *bookAmount) = initBookInfo(isbn);
    *bookAmount += 1;

    return 1;
}

//借书行为发起时:
//  1.查询馆藏数量是否 > 0
//  2.检索顺序表
//  3.调用addBook
int borrowingBook(Records * L, char *readerID, char *isbn){
//        首先查看用户是否已经有借阅记录, readerindex值为该用户借阅记录在顺序表中的索引值
    int readerindex = checkReaderIndex(L, readerID);

    //0表示当前无借阅记录, 顺序表中无对应记录
    if(readerindex == -1){
        struct readerRecord rd;
//        *rd .readerID = *readerid;
        strcpy(rd.readerID, readerID);

        readerindex = addRecord(L, rd);
    }

    //// -> 检查用户借阅数量是否合法，调用查询接口看看馆藏

    //录入书籍信息
    //status表示添加图书是否成功
    int status;
    status = addBookInfo(L, readerindex, isbn);

    if(status == 0){
        //借阅失败,可能是超出借阅数量的上限4
        return 0;
    }
    return 1;
}

// 现在要归还第 returned本书( >= 1)
int reduceBookInfo(struct bookInfo * book, int returned, int bookcounter){
    //i表示要循环覆盖的次数
    int i;

    //把后面的书籍覆盖前面的书籍信息
    for(i = 0; i < bookcounter - returned; i++){
        *(book + returned + i) = *(book + returned + i + 1);
    }

//    editFile(2);

}

void returningBook(Records *L, char *readerID, char *isbn){
    int readerindex = checkReaderIndex(L, readerID);

//    指针*book指向顺序表某用户书籍信息array的第一本书
    struct bookInfo *book = L ->readerRecord[readerindex].bookInfo;
    int * amount = &L ->readerRecord[readerindex].bookCounter;
    int bookindex;
    int result = -1;

    //查找归还的是哪本书?
    for(bookindex = 0; bookindex < 4; bookindex++){
        result = strcmp((book + bookindex) ->ISBN, isbn);

        if(result == 0){
            reduceBookInfo(book, bookindex, *amount);
            *amount -= 1;

            //这个也不写了，交给main
            //// -> 调用函数,增加馆藏数量
            break;
        }
//        result != 0的情况不考虑,因为一定能找到,没借过的书不设置还书选项,不会有找不到的书
    }

    //如果用户归还了所有书籍,那么从顺序表中删除该用户记录
    if( L ->readerRecord[readerindex].bookCounter == 0){
        deleteRecord(L, readerindex);
    }
}

