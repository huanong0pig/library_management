//
// Created by 22298 on 2023/3/21.
//

#ifndef LIBRARY_MANAGEMENT_BORROWING_H
#define LIBRARY_MANAGEMENT_BORROWING_H

//能产生的最多借阅记录， maxsize = 馆藏所有图书数量/4，即所有图书同时被借走的情况
#define RECORD_MAXSIZE 2000

//设置最大借阅时长为30天, 以秒记(30 x 24 x 60 x 60)
#define LENDING_LIM 2592000

//readerID + isbn + date + 3 = 11 + 13 + 8 + 3 = 35
#define RECORD_LEN 35


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


Records * init_Records();

int addRecord(Records * L, struct readerRecord rd);

int deleteRecord(Records * L, int i);

struct readerRecord checkRecords(Records *L, char *readerID);

//格式化借阅时间
char* formatDate(int opt);

//初始化所借书籍的信息
struct bookInfo initBookInfo(char *isbn);

//新增包含对应书籍信息的借阅记录
int addBookInfo(Records * L, int readerIndex, char *isbn);

//发起借书操作
int borrowingBook(Records * L, char *readerID, char *isbn);

//删去包含对应书籍的借阅记录
int reduceBookInfo(struct bookInfo * book, int returned, int bookcounter);

//发起还书操作
void returningBook(Records *L, char *readerID, char *isbn);

//将内存中的顺序表存在csv文件中
int saveFile(Records *L, char *filename);

#endif //LIBRARY_MANAGEMENT_BORROWING_H
