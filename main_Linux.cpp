#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <queue>
#include <unistd.h>
#include <cstddef>
#include <stdlib.h>

using namespace std;

struct student
{
	int flag = 0;      //表示是否坐在座位，0表示站着，1表示坐着
	int ID;
};
int CurrID;  //当前准备考试，或是正在考试的考生
int mynumber;
student t[10];

std::queue<student*> q;             //座位队列，共享缓冲区
sem_t seat,person,mutex,exam_done,check,notice;

void *examinee(void *lp)
{
    student* data = (student*)lp;
    sem_wait(&seat);
    sem_wait(&mutex);
    if (data->flag == 0)                        //考生进入座位，即是加入队列
	{
		q.push(data);
		data->flag++;
	}
	cout << data->ID << "号考生坐到座位上" << endl;


    sem_post(&mutex);
    sem_post(&person);
    sem_wait(&exam_done);
}

void *assistant(void *lp)
{
    while(1)
    {
        sem_wait(&check);
        sem_wait(&person);
        sem_wait(&mutex);
        if (!q.empty())                                     //顺序叫起一名考生，腾出一把椅子
		{
			CurrID = q.front()->ID;
			q.pop();
		}
		printf("                         %d号考生进入考场\n", CurrID);
        sem_post(&mutex);
		sem_post(&seat);
        sem_post(&notice);
    }

}

void *examiner(void *lp)
{
    while(1)
    {
        sem_wait(&notice);
        //开始考试
		cout <<"                                             "<<CurrID << "号考生正在考试" << endl;
		sys_mysleep(mynumber);                      //模拟考试等待
		cout <<"                                             "<<CurrID << "号考生考试结束" << endl;

        int q_size = q.size();
        if (!q.empty())
		{
            //输出当前队列（座位上的考生）
            cout << "当前座位：" ;
            for(int i = 0; i < q_size; i++)
            {
                cout << q.front()->ID<< "  ";
                q.push(q.front());
                q.pop();
            }
            cout << endl;
		}


		sem_post(&exam_done);
        sem_post(&check);

    }
}

//考生编号初始化
void initialize_student()
{
    for (int i = 0; i < 10; i++)
	{
		t[i].ID = i + 1;
		t[i].flag = 0;
	}
}

void visual()
{

    cout << "     **************************************************" << endl;
    cout << "        *********第一题 ：线程的同步与互斥**********   " << endl;
    cout << "              ************操作 系统*************        " << endl;
    cout << "              ************Linux版本************        " << endl;
    cout << endl;
}

int main(int argc,char*argv[])
{
    initialize_student();//初始化考生结构体
    cout << "请输入你的学号： " ;
    cin >> mynumber;
    visual();

	//信号量初始化
	sem_init(&seat,0,5);
	sem_init(&person,0,0);
    sem_init(&mutex,0,1);
    sem_init(&exam_done,0,1);
    sem_init(&check,0,1);
    sem_init(&notice,0,0);
    //创建线程

    pthread_t tid0;
    pthread_t tid1;
    pthread_t tid2;
    static int i;           //线程的创建，需要使用静态变量
    for (i = 0; i < 10; i++)
        pthread_create(&tid0,NULL,examinee,&t[i]);

    pthread_create(&tid1,NULL,assistant,NULL);

    pthread_create(&tid2,NULL,examiner,NULL);

    pthread_exit(0);
    return 0;
}
