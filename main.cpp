#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <queue>
#include <cstddef>
#include <stdlib.h>
#include <winbase.h>

using namespace std;

struct student
{
	int flag = 0;      //表示是否坐在座位，0表示站着，1表示坐着
	int ID;
};
int CurrID;  //当前准备考试，或是正在考试的考生
student t[10];

std::queue<student*> q;             //座位队列，共享缓冲区

HANDLE seat, person, mutex, exam_done, check_flag, notify_flag;		//对象定义,句柄

//考生线程
DWORD WINAPI examinee(LPVOID lp)
{
	student* data = (student*)lp;
	WaitForSingleObject(seat, INFINITE);		//P(seat)

	WaitForSingleObject(mutex, INFINITE);		//P(mutex)

	if (data->flag == 0)                        //考生进入座位，即是加入队列
	{
		q.push(data);
		data->flag++;
	}
	cout << data->ID << "号考生坐到座位上" << endl;
    int q_size = q.size();

    //输出当前队列（座位上的考生）
    cout << "当前座位上的考生（由前向后）：" ;
    for(int i = 0; i < q_size; i++)
    {
      cout << q.front()->ID<< "  ";
      q.push(q.front());
      q.pop();
    }
    cout << endl;

	ReleaseMutex(mutex);						//V(mutex)
	ReleaseSemaphore(person, 1, NULL);			//V(person)
	WaitForSingleObject(exam_done, INFINITE);   //P(exam_done),考试结束准备离开
	return 0;
}
//助理线程
DWORD WINAPI assistant(LPVOID lpParameter)
{
	while (1)
	{
		WaitForSingleObject(check_flag, INFINITE);			//P(check_flag),询问主考是否有空
		WaitForSingleObject(person, INFINITE);		        //P(person),检查座位是否有人
		WaitForSingleObject(mutex, INFINITE);		//P(mutex)
		if (!q.empty())                                     //顺序叫起一名考生，腾出一把椅子
		{
			CurrID = q.front()->ID;
			q.pop();
		}
		printf("                                            %d号考生进入考场\n", CurrID);
        ReleaseMutex(mutex);						//V(mutex)
		ReleaseSemaphore(seat, 1, NULL);                     //V(seat),产生一把空椅子
		ReleaseSemaphore(notify_flag, 1, NULL);              //V(notify_flag),通知主考有人进入
	}
	return 0;
}
//主考线程
DWORD WINAPI examiner(LPVOID lpParameter)
{
	while (1)
	{
		WaitForSingleObject(notify_flag, INFINITE);		    //P(notify_flag)，询问是否有考生进入
		//开始考试
		cout <<"                                                             "<<CurrID << "号考生正在考试" << endl;
		//Sleep(1500);                        //模拟考试等待
		cout <<"                                                             "<<CurrID << "号考生考试结束" << endl;
        if (q.empty())
		{
			cout << "口语考试结束！" <<endl;
			return 1;
		}
		ReleaseSemaphore(exam_done, 1, NULL);                 //V(exam_done),通知考生考试结束，可以离开
		ReleaseSemaphore(check_flag, 1, NULL);              //V(check_flag),表示主考空闲
	}
	return 0;
}

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
    cout << "              ************操作系统*************        " << endl;
    cout << endl;

}

int main(int argc, char* argv)
{

    initialize_student();//初始化考生结构体

	visual();

    //创建一个信号量(信号量的属性，初始值，最大值，名字)
	seat = CreateSemaphore(NULL, 5, 5, NULL);		//seat表示空座位的数量
	person = CreateSemaphore(NULL, 0, 5, NULL);       //person表示座位上的人数

	//创建一个互斥量(指向安全属性的指针，初始化互斥对象的所有者*FALSE表示不被任何线程拥有，指向互斥对象名的指针）
	mutex = CreateMutex(NULL, FALSE, NULL);             //互斥信号量来保护共享缓冲区（座位）

	exam_done = CreateSemaphore(NULL, 1, 1, NULL);
	check_flag = CreateSemaphore(NULL, 1, 1, NULL);
	notify_flag = CreateSemaphore(NULL, 0, 1, NULL);

	//考生的线程建立
	HANDLE examinee_Thread[10];
	static int i;           //线程的创建，需要使用静态变量
	for (i = 0; i < 10; i++)
        examinee_Thread[i]=CreateThread(NULL,0,examinee,&t[i],0,NULL);//（指向安全属性的指针，初始化线程堆栈大小，线程的起始地址，给线程传递的参数，线程的标志0是立即运行，线程的ID）
	//助理的线程建立
    HANDLE assistant_Thread=CreateThread(NULL,0,assistant,NULL,0,NULL);
	//主考的线程建立
	HANDLE examiner_Thread=CreateThread(NULL,0,examiner,NULL,0,NULL);

	getchar();

	//所有线程执行完毕后关闭

	//有借有还
	CloseHandle(mutex);
	CloseHandle(seat);
	CloseHandle(person);
	CloseHandle(exam_done);
	CloseHandle(check_flag);
	CloseHandle(notify_flag);
	for (int i = 0; i < 10; i++)
		CloseHandle(examinee_Thread[i]);
	CloseHandle(assistant_Thread);
	CloseHandle(examiner_Thread);

	return 0;
}

