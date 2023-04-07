//
// Created by 22298 on 2023/3/21.
//

#include "borrowing.h"
#include<stdio.h>
#include <malloc.h>
#include <string.h>
#include "time.h"

//能产生的最多借阅记录， maxsize = 馆藏所有图书数量/4，即所有图书同时被借走的情况
#define RECORD_MAXSIZE 2000

//设置最大借阅时长为30天, 以秒记(30 x 24 x 60 x 60)
#define LENDING_LIM 2592000
#define RECORD_LEN 34

#define BOOK_INFO "data/books.csv"

typedef struct bookInfo{
    char ISBN[13];
    char borrowingDate [8];

    //借阅时间默认为30天，所以归还日期不需要从csv中读取，只要程序运行的时候生成就行了
    char returningDate [8];
};

//一个人可以借多本书，所以书籍信息要以 数组[]形式 存储


typedef struct readerRecord{
    char readerID[11];

    //需要在结构体中单独添加成员来计算已借书数量吗?
    int bookCounter;

    //一个人最多借四本书
    struct bookInfo bookInfo[4];
};

typedef struct RecordList{
    struct readerRecord readerRecord[RECORD_MAXSIZE];
    int last;
}Records;

//硬盘中的csv文件需要先初始化到内存当中，即，将csv中的数据转化为内存中的链表来操作，所以我们需要一个类似 init()函数

//返回一个链表的头指针,方便我们操作链表
//record.csv初始时没有数据，应该在程序运行时创建？

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

//用户归还所有书时消除记录, returning all Book
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

//修改records.csv文件
//1表示增, 2表示删, 不需要设置查,查询直接从内存查寻就行, 文件内容始终和内存保持同步

int editFile(int opt, char *readerID, char *isbn, char *borrowingDate){
    FILE *fp;
    fp = fopen("records.csv", "w");

    if(opt == 1){
        char record[RECORD_LEN] = "";
        sprintf(record, "%s,%s,%s\n", readerID, isbn, borrowingDate);
        fputs(record, "records.csv");


    } else if(opt == 2){


    }

    fclose("records.csv");
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

//  初始化借阅的书籍信息
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

    //先判断是否超出借阅上限
    if(*bookAmount >= 3){
        printf("Over the borrowing limit!");
        return 0;
    }

    // 如果amount = 2说明用户已经接了两本书,那么新书信息应该更新在bookInfo[2]中, 即

    *(book + *bookAmount) = initBookInfo(isbn);
    *bookAmount += 1;

//    修改csv文件
    char *readerID = &(L ->readerRecord[readerIndex].readerID);
    char *borrowingDate = (book + *bookAmount - 1) ->borrowingDate;
    editFile(1,readerID, isbn, borrowingDate);

    return 1;
}

//借书行为发起时:
//  1.查询馆藏数量是否 > 0
//  2.检索顺序表
//  3.调用addBook
int borrowingBook(Records * L, char *readerID, char *isbn){
//    FILE *fp;
//    fp = fopen("books.csv", "r");
    //// -> 调用查询接口


//        首先查看用户是否已经有借阅记录, result值为该用户借阅记录在顺序表中的索引值
    int readerindex = checkReaderIndex(L, readerID);

    //0表示当前无借阅记录, 顺序表中无对应记录
    if(readerindex == -1){
        struct readerRecord rd;
//        *rd .readerID = *readerid;
        strcpy(rd.readerID, readerID);

        readerindex = addRecord(L, rd);
    }

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



