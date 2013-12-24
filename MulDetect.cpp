#include <Windows.h>
#include <string>
#include <list>
#include <process.h>
#include <iostream>
using namespace std;


typedef struct _MY_MSG{
	wchar_t vboxname[100];
	wchar_t filename[200];
	wchar_t resultpath[256];
	DWORD Reserver;
	DWORD Reserver2;
}MY_MSG,pMY_MSG;
typedef struct _VBOX_STATE{
	wchar_t vboxname[100];
	bool state;
	DWORD Reserver;
	DWORD Reserver2;
}VBOX_STATE,pVBOX_STATE;
list<VBOX_STATE> list_vbox_state;   //Ӧ�ó�ʼ��
unsigned nThreadDispatch;
unsigned nThreadvbox;
unsigned nthreadMonitor;
unsigned nthreadHandle;
unsigned nthreadTick;

#define MONITOR_MSG WM_USER+100
#define VBOXFREE_MSG WM_USER+101
#define VBOXSTART_MSG WM_USER+102
#define VBOXSTOP_MSG WM_USER+103
#define STARTCOUNT_MSG WM_USER+104
#define HANDLERESULT_MSG WM_USER+105
#define THREADTERMINATE_MSG WM_USER+106
unsigned _stdcall ControlVboxThread(void * param)
{
	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	while(true)
	{
		if(GetMessage(&msg,0,0,0)) //get msg from message queue
		{	
			MY_MSG mymsg_vbox;
			memset(&mymsg_vbox,0,sizeof(MY_MSG));
			memcpy((void*)&mymsg_vbox,(void*)msg.wParam,sizeof(MY_MSG));
			switch(msg.message)
			{
			case VBOXSTART_MSG:
				{	
					// �˴�Ϊ�������ֵ������                                                                              �������������������� δ���
					break;
				}
			case VBOXSTOP_MSG:
				{
					//�˴�Ϊ�ر����ֵ������                                                                              �������������������� δ���

					PostThreadMessage(nThreadDispatch,VBOXFREE_MSG,(WPARAM)&mymsg_vbox,NULL);
					PostThreadMessage(nthreadHandle,HANDLERESULT_MSG,(WPARAM)&mymsg_vbox,NULL);
					break;
				}
			case THREADTERMINATE_MSG:
				{
					//�˴�Ϊ�ر����������                                                                              �������������������� δ���
				}
			}
		}
	};
	return 1;
}
unsigned _stdcall HandleResultThread(void * param)
{	
	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	while(true)
	{
		if(GetMessage(&msg,0,0,0)) //get msg from message queue
		{	
			MY_MSG mymsg_result;
			memset(&mymsg_result,0,sizeof(MY_MSG));
			memcpy((void*)&mymsg_result,(void*)msg.wParam,sizeof(MY_MSG));
			switch(msg.message)
			{
			case HANDLERESULT_MSG:
				{	
					// ��Ϊ������̨���                                                                                �������������������� δ���
					break;
				}
			case THREADTERMINATE_MSG:
				{

				}
			}
		}
	};
	return 1;
}
#define  NUM_VBOX 3
unsigned _stdcall GetTickThread(void * param)
{
	DWORD num_vbox[NUM_VBOX] = {0};
	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	while(true)
	{
		if(GetMessage(&msg,0,0,0)) //get msg from message queue
		{			
			switch(msg.message)
			{
			case STARTCOUNT_MSG:
				{	
					DWORD dwstart = GetTickCount();
					for(int i =0;i<NUM_VBOX;i++)
					{
						if (num_vbox[i]==0)
						{
							num_vbox[i] = dwstart;
						}
					}
					while(true)
					{
						DWORD dwstop = GetTickCount();
						for (int i =0;i<NUM_VBOX;i++)
						{
							if (dwstop - num_vbox[i] > 60*1000)
							{
								//	PostThreadMessage(nthreadHandle,VBOXSTOP_MSG,)
								num_vbox[i] = 0;
							}
						}
					}
					break;
				}
			case THREADTERMINATE_MSG:
				{

				}
			}
		}
	};
	return 1;
}
unsigned _stdcall DispatchTackThread(void * param)
{
	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	list<MY_MSG> list_task; 
	while(true)
	{
		if(GetMessage(&msg,0,0,0)) //get msg from message queue
		{			
			switch(msg.message)
			{
			case MONITOR_MSG:
				{	
					printf("DispatchTackThread get msg");
					//					FILE_NOTIFY_INFORMATION *tmp = new FILE_NOTIFY_INFORMATION; 
					MY_MSG mymsg_dispatch;
					memset(&mymsg_dispatch,0,sizeof(MY_MSG));
					memcpy((void*)&mymsg_dispatch,(void*)msg.wParam,sizeof(MY_MSG));
					// ��Ҫ�ж��Ƿ������������
					for (list<VBOX_STATE>::iterator it = list_vbox_state.begin(); it != list_vbox_state.end(); it++)
					{
						if (it->state==true)
						{
							wcscpy(mymsg_dispatch.vboxname,it->vboxname);
							PostThreadMessage(nThreadvbox,MONITOR_MSG,(WPARAM)&mymsg_dispatch,0);
							PostThreadMessage(nthreadTick,STARTCOUNT_MSG,(WPARAM)&mymsg_dispatch,0);
						}
						else
							list_task.push_back(mymsg_dispatch);
					}
					break;
				}
			case VBOXFREE_MSG:
				{
					MY_MSG mymsg_dispatch;
					memset(&mymsg_dispatch,0,sizeof(MY_MSG));
					memcpy((void*)&mymsg_dispatch,(void*)msg.wParam,sizeof(MY_MSG));
					if(!list_vbox_state.empty())
					{
						for (list<VBOX_STATE>::iterator it = list_vbox_state.begin(); it != list_vbox_state.end(); it++)
						{
							if (it->state==true)
							{
								PostThreadMessage(nThreadvbox,MONITOR_MSG,(WPARAM)&mymsg_dispatch,0);
								PostThreadMessage(nthreadTick,STARTCOUNT_MSG,(WPARAM)&mymsg_dispatch,0);
								list_vbox_state.erase(it);
								break;
							}
						}
					}
					else
					{
						for (list<VBOX_STATE>::iterator it = list_vbox_state.begin(); it != list_vbox_state.end(); it++)
						{
							if (it->vboxname==mymsg_dispatch.vboxname)
							{
								it->state = false;
								break;
							}
						}
						continue;
					}
				}
			case THREADTERMINATE_MSG:
				{

				}
			}
		}
	};
	return 1;
}
unsigned _stdcall FileMonitorThread(void * param)
{
	wchar_t path[] = L"E:\\testsymbol";
	HANDLE hDir = CreateFile( path, FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | 
		FILE_SHARE_WRITE | 
		FILE_SHARE_DELETE, NULL, 
		OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | 
		FILE_FLAG_OVERLAPPED, NULL); 
	if (hDir == INVALID_HANDLE_VALUE) 
	{ 
		DWORD err = GetLastError();
		return 0;
	} 
	char notify[1024]; 
	DWORD cbBytes; 


	FILE_NOTIFY_INFORMATION *pnotify=(FILE_NOTIFY_INFORMATION *)notify; 
	FILE_NOTIFY_INFORMATION *tmp; 
	while (TRUE) 
	{ 
		memset(notify,0,1024);
		if(ReadDirectoryChangesW(hDir, &notify, sizeof(notify),
			FALSE, FILE_NOTIFY_CHANGE_FILE_NAME| FILE_NOTIFY_CHANGE_LAST_WRITE, 
			&cbBytes, NULL, NULL)) 
		{ 

			tmp = pnotify; 
			MY_MSG mymsg;
			switch(tmp->Action) 
			{ 
			case FILE_ACTION_ADDED: 
				{
					//wprintf(tmp->FileName);

					int len = wcslen(tmp->FileName);
					memset(&mymsg,0,sizeof(MY_MSG));
					wcscpy(mymsg.filename,tmp->FileName);
					PostThreadMessage(nThreadDispatch,MONITOR_MSG,(WPARAM)&mymsg,0);
					break; 
				}

			case FILE_ACTION_REMOVED:
				{
					//printf("Directory/File removed ��ɾ���ļ���- \r\n");
					break; 
				}

			case FILE_ACTION_MODIFIED: 
				{
					//printf("Directory/File modified ���޸��ļ����ݣ�- \r\n");                   ????????????????????????�˴�Ӧ�ÿ����ļ�����ͬ����������
					break; 
				}
			case FILE_ACTION_RENAMED_OLD_NAME: 
				{
					//printf("Directory/File old name ���޸��ļ����֣�- \r\n");                         
					break; 
				}
			case FILE_ACTION_RENAMED_NEW_NAME: 
				{
					//printf("Directory/File new name - \r\n");
					break; 
				}
			case THREADTERMINATE_MSG:
				{

				}
			default: 
				break; 
			}
		}
	}

}

int main(int argc, char* argv[])
{
	// 	nmainThreadId = GetCurrentThreadId();
	// 线程间的通信  用信号量还是消息？

	HANDLE hthreadDispatch = (HANDLE)_beginthreadex(NULL,0,&DispatchTackThread,NULL,0,&nThreadDispatch);

	HANDLE hthreadpp = (HANDLE)_beginthreadex(NULL,0,&FileMonitorThread,NULL,0,&nthreadMonitor);

	HANDLE hvbox = (HANDLE)_beginthreadex(NULL,0,&ControlVboxThread,NULL,0,&nThreadvbox);

	HANDLE hhandle = (HANDLE)_beginthreadex(NULL,0,&HandleResultThread,NULL,0,&nthreadHandle);

	HANDLE hGetTick = (HANDLE)_beginthreadex(NULL,0,&GetTickThread,NULL,0,&nthreadTick);


	//	WaitForSingleObject(threadpp,INFINITE);
	return 0;
}

