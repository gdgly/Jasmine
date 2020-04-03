#ifndef _APP_H_
#define _APP_H_

#include "includes.h"

extern OS_EVENT *FramMutex;
extern OS_EVENT *SFlashMutex;
extern OS_EVENT *DubugSem;

//����ջ
#define MAINTASKSTKLEN    	3172	//������
#define CHARGETASKSTKLEN		512		//�������
#define PROTOCOLTASKSTKLEN	1536	//ͨ��Э������
#define NETTASKSTKLEN				512		//��������
#define METERTASKSTKLEN			256		//�������
#define CARDTASKSTKLEN			256		//������



//���ȼ�
#define MUTEX_FRAMPRIO			3
#define MUTEX_SFLASHPRIO		4
#define CHARGETASKPRIO			5		//�������
#define METERTASKPRIO				6		//�������
#define MAINTASKPRIO				8		//������
#define CARDTASKPRIO				7		//������
#define NETTASKPRIO					9		//��������
#define PROTOCOLTASKPRIO		10	//ͨ��Э������



#endif
