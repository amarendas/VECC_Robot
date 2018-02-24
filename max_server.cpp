#include<iostream>
#include<string.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/errno.h>

#include "Definitions.h"
#include <sstream>
#include <getopt.h>
#include <stdlib.h>
#include <list>
#include <math.h>

using namespace std;

typedef void* HANDLE;
typedef int BOOL;

void* g_pKeyHandle = 0;
unsigned short g_usNodeId = 1;
string g_deviceName;
string g_protocolStackName;
string g_interfaceName;
string g_portName;
int g_baudrate = 0;
char b1[20];
const string g_programName = "HelloEposCmd";

#ifndef MMC_SUCCESS
	#define MMC_SUCCESS 0
#endif

#ifndef MMC_FAILED
	#define MMC_FAILED 1
#endif

#ifndef MMC_MAX_LOG_MSG_SIZE
	#define MMC_MAX_LOG_MSG_SIZE 512
#endif

void LogInfo(string message);
int   OpenDevice(unsigned int* p_pErrorCode);
/*void LogError(string functionName, int p_lResult, unsigned int p_ulErrorCode)
{
	cerr << g_programName << ": " << functionName << " failed (result=" << p_lResult << ", errorCode=0x" << std::hex << p_ulErrorCode << ")"<< endl;
}*/
void  PrintSettings();
int   OpenDevice(unsigned int* p_pErrorCode);
int   CloseDevice(unsigned int* p_pErrorCode);
void  SetDefaultParameters();
int   DemoProfileVelocityMode(int v,HANDLE p_DeviceHandle, unsigned short p_usNodeId, unsigned int & p_rlErrorCode);
int   Demo(int vone,int vtwo,int steer,unsigned int* p_pErrorCode);
int   DemoProfilePositionMode(int p, HANDLE p_DeviceHandle, unsigned short p_usNodeId, unsigned int & p_rlErrorCode);

int divide(int b1[7])
{
    int lResult = MMC_FAILED;
    unsigned int ulErrorCode = 0;
    int light, vone, vtwo, steer;
    if(b1[0]==1)
    {
        light=1;
    }
    else if (b1[0]==0)
    {
        light=0;
       
    }
    else
    {
        cout<<"not valid";
        exit(0);
    }
    
    vone= b1[2];
    if(b1[1]==1)
    {
        vone=-vone;
    }
    
    vtwo= b1[4];
    if(b1[3]==1)
    {
        vtwo=-vtwo;
    }
    atan(vone);
    steer= b1[6];
    if(b1[5]==1)
    {
        steer=-steer;
    }
   
   
cout<<"v1:"<<vone<<endl;
cout<<"v2:"<<vtwo<<endl;
cout<<"steer angle:"<<steer<<endl;

if((lResult = Demo(vone,vtwo,steer,&ulErrorCode))!=MMC_SUCCESS)
	        {
		  cout<<"Error";// LogError("Demo", lResult, ulErrorCode);
		   return lResult;
	        }
}

void PrintSettings()
{      
	stringstream msg; 

	msg << "default settings:" << endl;
	msg << "node id = " << g_usNodeId << endl;
	msg << "device name = '" << g_deviceName << "'" << endl;
	msg << "protocal stack name = '" << g_protocolStackName << "'" << endl;
	msg << "interface name = '" << g_interfaceName << "'" << endl;
	msg << "port name = '" << g_portName << "'"<< endl;
	msg << "baudrate  = " << g_baudrate;

	LogInfo(msg.str());

	
}

void SetDefaultParameters()
{
	//USB
	g_usNodeId = 2;
	g_deviceName = "EPOS2"; //EPOS
	g_protocolStackName = "MAXON SERIAL V2"; //MAXON_RS232
	g_interfaceName = "USB"; //RS232
	g_portName = "USB0"; // /dev/ttyS1
	g_baudrate = 1000000; //115200
        
}

void LogInfo(string message)
{
	cout << message << endl;
}

int OpenDevice(unsigned int* p_pErrorCode)
{
	int lResult = MMC_FAILED;

	char* pDeviceName = new char[255];
	char* pProtocolStackName = new char[255];
	char* pInterfaceName = new char[255];
	char* pPortName = new char[255];

	strcpy(pDeviceName, g_deviceName.c_str());
	strcpy(pProtocolStackName, g_protocolStackName.c_str());
	strcpy(pInterfaceName, g_interfaceName.c_str());
	strcpy(pPortName, g_portName.c_str());

	LogInfo("Open device...");

	g_pKeyHandle = VCS_OpenDevice(pDeviceName, pProtocolStackName, pInterfaceName, pPortName, p_pErrorCode);

	if(g_pKeyHandle!=0 && *p_pErrorCode == 0)
	{
		unsigned int lBaudrate = 0;
		unsigned int lTimeout = 0;

		if(VCS_GetProtocolStackSettings(g_pKeyHandle, &lBaudrate, &lTimeout, p_pErrorCode)!=0)
		{
			if(VCS_SetProtocolStackSettings(g_pKeyHandle, g_baudrate, lTimeout, p_pErrorCode)!=0)
			{
				if(VCS_GetProtocolStackSettings(g_pKeyHandle, &lBaudrate, &lTimeout, p_pErrorCode)!=0)
				{
					if(g_baudrate==(int)lBaudrate)
					{
						lResult = MMC_SUCCESS;
					}
				}
			}
		}
	}
	else
            
	{
            
		g_pKeyHandle = 0;
	}

	delete []pDeviceName;
	delete []pProtocolStackName;
	delete []pInterfaceName; 
	delete []pPortName;

	return lResult;
}

int CloseDevice(unsigned int* p_pErrorCode)
{
	int lResult = MMC_FAILED;

	*p_pErrorCode = 0;

	LogInfo("Close device");

	if(VCS_CloseDevice(g_pKeyHandle, p_pErrorCode)!=0 && *p_pErrorCode == 0)
	{
		lResult = MMC_SUCCESS;
	}

	return lResult;
}

int PrepareDemo(unsigned int* p_pErrorCode)
{
	int lResult = MMC_SUCCESS;
	BOOL oIsFault = 0;

	if(VCS_GetFaultState(g_pKeyHandle, g_usNodeId, &oIsFault, p_pErrorCode ) == 0)
	{
		cout<<"Error";//LogError("VCS_GetFaultState", lResult, *p_pErrorCode);
		lResult = MMC_FAILED;
	}

        
                
        
	if(lResult==0)
	{
		if(oIsFault)
		{
			stringstream msg;
			msg << "clear fault, node = '" << g_usNodeId << "'";
			LogInfo(msg.str());

			if(VCS_ClearFault(g_pKeyHandle, g_usNodeId, p_pErrorCode) == 0)
			{
				cout<<"Error";//LogError("VCS_ClearFault", lResult, *p_pErrorCode);
				lResult = MMC_FAILED;
			}
		}

		if(lResult==0)
		{
			BOOL oIsEnabled = 0;

			if(VCS_GetEnableState(g_pKeyHandle, g_usNodeId, &oIsEnabled, p_pErrorCode) == 0)
			{
				cout<<"Error";//LogError("VCS_GetEnableState", lResult, *p_pErrorCode);
				lResult = MMC_FAILED;
			}

			if(lResult==0)
			{
				if(!oIsEnabled)
				{
					if(VCS_SetEnableState(g_pKeyHandle, g_usNodeId, p_pErrorCode) == 0)
					{
						cout<<"Error";//LogError("VCS_SetEnableState", lResult, *p_pErrorCode);
						lResult = MMC_FAILED;
					}
				}
			}
		}
           
	}
        if(VCS_ActivateProfileVelocityMode(g_pKeyHandle, g_usNodeId, p_pErrorCode) == 0)
        {
            cout<<"Error";
	    //LogError("VCS_ActivateProfileVelocityMode", lResult, p_rlErrorCode);
            lResult = MMC_FAILED;
	 }
        
         if(VCS_ActivateProfilePositionMode(g_pKeyHandle, g_usNodeId, p_pErrorCode) == 0)
	{
		cout<<"error";//LogError("VCS_ActivateProfilePositionMode", lResult, p_rlErrorCode);
		lResult = MMC_FAILED;
	}
	return lResult;     
        
}

int Demo(int vone,int vtwo,int steer,unsigned int* p_pErrorCode)
{
      //nets t;
      
    int lResult = MMC_SUCCESS;
	unsigned int lErrorCode = 0;

	if(g_usNodeId==1)
        {
        lResult = DemoProfileVelocityMode(vone, g_pKeyHandle, g_usNodeId, lErrorCode);
        }
        else
          if(g_usNodeId==2)
        {
        lResult = DemoProfileVelocityMode(vtwo, g_pKeyHandle, g_usNodeId, lErrorCode);
        }
        else  
            if(g_usNodeId==2)
            {
                lResult = DemoProfilePositionMode(steer,g_pKeyHandle, g_usNodeId, lErrorCode);
            }
	if(lResult != MMC_SUCCESS)
	{
            cout<<"Error";
		//LogError("DemoProfileVelocityMode", lResult, lErrorCode);
	}
        
	/*else
	{
		lResult = DemoProfilePositionMode(g_pKeyHandle, g_usNodeId, lErrorCode);

		if(lResult != MMC_SUCCESS)
		{
			LogError("DemoProfilePositionMode", lResult, lErrorCode);
		}
		else
		{
			if(VCS_SetDisableState(g_pKeyHandle, g_usNodeId, &lErrorCode) == 0)
			{
				LogError("VCS_SetDisableState", lResult, lErrorCode);
				lResult = MMC_FAILED;
			}
		}
	}*/
	return lResult;
}

int DemoProfileVelocityMode(int v, HANDLE p_DeviceHandle, unsigned short p_usNodeId, unsigned int & p_rlErrorCode)
{
	int lResult = MMC_SUCCESS;
	stringstream msg;
        
	msg << "set profile velocity mode, node = " << p_usNodeId;

	LogInfo(msg.str());

	/*if(VCS_ActivateProfileVelocityMode(p_DeviceHandle, p_usNodeId, &p_rlErrorCode) == 0)
	{
            cout<<"Error";
		//LogError("VCS_ActivateProfileVelocityMode", lResult, p_rlErrorCode);
		lResult = MMC_FAILED;
	}*/
	
	 
            
                
            long targetvelocity = v;

            //stringstream msg;
			msg << "move with target velocity = " << targetvelocity << " rpm, node = " << p_usNodeId;
			LogInfo(msg.str());

			if(VCS_MoveWithVelocity(p_DeviceHandle, p_usNodeId, targetvelocity, &p_rlErrorCode) == 0)
			{
				lResult = MMC_FAILED;
                                cout<<"Error";
				//LogError("VCS_MoveWithVelocity", lResult, p_rlErrorCode);
				//break;
			}

			sleep(3);
		

		if(lResult == MMC_SUCCESS)
		{
			LogInfo("halt velocity movement");

			if(VCS_HaltVelocityMovement(p_DeviceHandle, p_usNodeId, &p_rlErrorCode) == 0)
			{
				lResult = MMC_FAILED;
                                cout<<"Error";
				//LogError("VCS_HaltVelocityMovement", lResult, p_rlErrorCode);
			}
		}
                
            
	

	return lResult;

}

int DemoProfilePositionMode(int p, HANDLE p_DeviceHandle, unsigned short p_usNodeId, unsigned int & p_rlErrorCode)
{
	int lResult = MMC_SUCCESS;
	stringstream msg;

	msg << "set profile position mode, node = " << p_usNodeId;
	LogInfo(msg.str());

	/*if(VCS_ActivateProfilePositionMode(p_DeviceHandle, p_usNodeId, &p_rlErrorCode) == 0)
	{
		cout<<"error";//LogError("VCS_ActivateProfilePositionMode", lResult, p_rlErrorCode);
		lResult = MMC_FAILED;
	}*/
	
			long targetPosition = p;
			//stringstream msg;
			msg << "move to position = " << targetPosition << ", node = " << p_usNodeId;
			LogInfo(msg.str());

			if(VCS_MoveToPosition(p_DeviceHandle, p_usNodeId, targetPosition, 0, 1, &p_rlErrorCode) == 0)
			{
				cout<<"error";//LogError("VCS_MoveToPosition", lResult, p_rlErrorCode);
				lResult = MMC_FAILED;
				//break;
			}

			sleep(1);
	

		if(lResult == MMC_SUCCESS)
		{
			LogInfo("halt position movement");

			if(VCS_HaltPositionMovement(p_DeviceHandle, p_usNodeId, &p_rlErrorCode) == 0)
			{
				cout<<"error";//LogError("VCS_HaltPositionMovement", lResult, p_rlErrorCode);
				lResult = MMC_FAILED;
			}
		}
	

	return lResult;
}

class nets
{
	public: 
		static void* conn(void* unnamed);
		int work(int s);
		void lis(int s);
};

void* nets::conn(void* unnamed)
{
	static sockaddr_in server;
	nets n;
	int s;
	 
	s=socket(AF_INET,SOCK_STREAM,0);
	cout<<"socket created"<<endl;
	
	server.sin_family=AF_INET;
	server.sin_port=8000;
	server.sin_addr.s_addr=inet_addr("127.0.0.1");
	
	if(bind(s,(struct sockaddr*)&server,sizeof(server))>=0)
	{
		cout<<"binding"<<endl;
	}
	else
	{
		cout<<"error in binding"<<endl;
	}
	n.lis(s);
}
	
void nets::lis(int s)
{
	if(listen(s,3)==0)
	{
		cout<<endl<<"listening";
		cout<<endl<<"server ready,waiting for client";
	}
	else
	{
		cout<<endl<<"error in listening";
		cout<<endl<<"errno"<<errno;
	}
	work(s);
	
}
int nets:: work(int s)
{
	nets t;
	cout<<endl<<"welcome"<<endl;
	
	sockaddr_in client;
	int b1[7];
	int c,i;
	int n=sizeof(client);
	c=accept(s,(struct sockaddr*)&client,(socklen_t*)&n);
	cout<<"errno"<<errno;
	while(recv(c,b1,sizeof(b1),0))
	{
		
                 cout<<"client:";//<<b1<<endl;
                
                 divide(b1);
                // for(i=0;i<7;i++)
                // {
                  //  cout<<b1[i]<<endl;
                  //}
		//if(strcmp(b1,"end")==0)
		if(b1!=0)
                {
		
                       if(b1[1]!=1)
                      
                       {  
                        cout<<"client no more exit"<<endl;
			lis(s);
		       }
	         }
       }
	close(s);

}

int main()
{
    int lResult = MMC_FAILED;
	unsigned int ulErrorCode = 0;
        
        SetDefaultParameters();

	PrintSettings();
        if((lResult = OpenDevice(&ulErrorCode))!=MMC_SUCCESS)
	{
		cout<<"Error";//LogError("OpenDevice", lResult, ulErrorCode);
		return lResult;
	}

        
        if((lResult = PrepareDemo(&ulErrorCode))!=MMC_SUCCESS)
	{
		cout<<"Error";//LogError("PrepareDemo", lResult, ulErrorCode);
		return lResult;
	}
        
        
	nets n;
	pthread_t tID;
	char *b;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_create(&tID,&attr,&n.conn,NULL);
	pthread_join(tID,(void**)&b);
	pthread_attr_destroy(&attr);
        
        if((lResult = CloseDevice(&ulErrorCode))!=MMC_SUCCESS)
	{
	cout<<"Error";	//LogError("CloseDevice", lResult, ulErrorCode);
		return lResult;
	}

	return 0;
}