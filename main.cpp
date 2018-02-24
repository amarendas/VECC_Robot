//============================================================================
// Name        : main.cpp
// Author      : Shishir Singh
// Version     :
// Copyright   : DRHR, BARC
// Description : Robot server program in C++
//============================================================================

#include <iostream>
#include "Definitions.h"
#include <string.h>
#include <sstream>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <list>
#include <math.h>
#include<pthread.h>
#include"RobotServer.h"
typedef void* HANDLE;
typedef int BOOL;

using namespace std;

void* g_pKeyHandle = 0;
unsigned short g_usNodeId = 1;
unsigned short node1 = 1;
unsigned short node2 = 2;
unsigned short node3 = 3;
unsigned short node4 = 4;
string g_deviceName;
string g_protocolStackName;
string g_interfaceName;
string g_portName;
int g_baudrate = 0;

const string g_programName = "RobotServer";

#ifndef MMC_SUCCESS
	#define MMC_SUCCESS 0
#endif

#ifndef MMC_FAILED
	#define MMC_FAILED 1
#endif

#ifndef MMC_MAX_LOG_MSG_SIZE
	#define MMC_MAX_LOG_MSG_SIZE 512
#endif

void  LogError(string functionName, int p_lResult, unsigned int p_ulErrorCode);
void  LogInfo(string message);
void  PrintUsage();
void  PrintHeader();
void  PrintSettings();
int   OpenDevice(unsigned int* p_pErrorCode);
int   CloseDevice(unsigned int* p_pErrorCode);
void  SetDefaultParameters();
int   ParseArguments(int argc, char** argv);
int   DemoProfilePositionMode(HANDLE p_DeviceHandle, unsigned short p_usNodeId, unsigned int & p_rlErrorCode);
int   Demo(unsigned int* p_pErrorCode);
int   SetMotorModes(unsigned int & p_rlErrorCode); 
int   PrepareMotors(unsigned int* p_pErrorCode);

void PrintUsage()
{
	cout << "Usage: HelloEposCmd -h -n 1 -d deviceName -s protocolStackName -i interfaceName -p portName -b baudrate" << endl;
}

void LogError(string functionName, int p_lResult, unsigned int p_ulErrorCode)
{
	cerr << g_programName << ": " << functionName << " failed (result=" << p_lResult << ", errorCode=0x" << std::hex << p_ulErrorCode << ")"<< endl;
}

void LogInfo(string message)
{
	cout << message << endl;
}

void SeparatorLine()
{
	const int lineLength = 60;
	for(int i=0; i<lineLength; i++)
	{
		cout << "-";
	}
	cout << endl;
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

	SeparatorLine();
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
        //RS232
        //g_usNodeId = 1;
	//g_deviceName = "EPOS2";
	//g_protocolStackName = "MAXON_RS232";
	//g_interfaceName = "RS232";
	//g_portName = "/dev/ttyS0";
	//g_baudrate = 115200;
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

int ParseArguments(int argc, char** argv)
{
	int lOption;
	int lResult = MMC_SUCCESS;

	// Shut GetOpt error messages down (return '?'):
	opterr = 0;
	// Retrieve the options:
	while ( (lOption = getopt(argc, argv, ":h:d:s:i:p:b:n:")) != -1 )
	{
		switch ( lOption ) {
			case 'h':
				PrintUsage();
				lResult = 1;
				break;
			case 'd':
				g_deviceName = optarg;
				break;
			case 's':
				g_protocolStackName = optarg;
				break;
			case 'i':
				g_interfaceName = optarg;
				break;
			case 'p':
				g_portName = optarg;
				break;
			case 'b':
				g_baudrate = atoi(optarg);
				break;
			case 'n':
				g_usNodeId = (unsigned short)atoi(optarg);
				break;
			case '?':  // unknown option...
				stringstream msg;
				msg << "Unknown option: '" << char(optopt) << "'!";
				LogInfo(msg.str());
				PrintUsage();
				lResult = MMC_FAILED;
				break;
		}
	}

	return lResult;
}

//int DemoProfilePositionMode(HANDLE p_DeviceHandle, unsigned short p_usNodeId, unsigned int & p_rlErrorCode)
//{
//	int lResult = MMC_SUCCESS;
//	stringstream msg;
//
//	msg << "set profile position mode, node = " << p_usNodeId;
//	LogInfo(msg.str());
//
//	if(VCS_ActivateProfilePositionMode(p_DeviceHandle, p_usNodeId, &p_rlErrorCode) == 0)
//	{
//		LogError("VCS_ActivateProfilePositionMode", lResult, p_rlErrorCode);
//		lResult = MMC_FAILED;
//	}
//	else
//	{
//		list<long> positionList;
//
//		positionList.push_back(5000);
//		positionList.push_back(-20000);
//		positionList.push_back(20000);
//		positionList.push_back(-10000);
//		positionList.push_back(10000);
//		positionList.push_back(-10000);
//		positionList.push_back(5000);
//
//		for(list<long>::iterator it = positionList.begin(); it !=positionList.end(); it++)
//		{
//			long targetPosition = (*it);
//			stringstream msg;
//			msg << "move to position = " << targetPosition << ", node = " << p_usNodeId;
//			LogInfo(msg.str());
//
//			if(VCS_MoveToPosition(p_DeviceHandle, p_usNodeId, targetPosition, 0, 1, &p_rlErrorCode) == 0)
//			{
//				LogError("VCS_MoveToPosition", lResult, p_rlErrorCode);
//				lResult = MMC_FAILED;
//				break;
//			}
//
//			sleep(1);
//		}
//
//		if(lResult == MMC_SUCCESS)
//		{
//			LogInfo("halt position movement");
//
//			if(VCS_HaltPositionMovement(p_DeviceHandle, p_usNodeId, &p_rlErrorCode) == 0)
//			{
//				LogError("VCS_HaltPositionMovement", lResult, p_rlErrorCode);
//				lResult = MMC_FAILED;
//			}
//		}
//	}
//
//	return lResult;
//}
//
//bool DemoProfileVelocityMode(HANDLE p_DeviceHandle, unsigned short p_usNodeId, unsigned int & p_rlErrorCode)
//{
//	int lResult = MMC_SUCCESS;
//	stringstream msg;
//
//	msg << "set profile velocity mode, node = " << p_usNodeId;
//
//	LogInfo(msg.str());
//
//	if(VCS_ActivateProfileVelocityMode(p_DeviceHandle, p_usNodeId, &p_rlErrorCode) == 0)
//	{
//		LogError("VCS_ActivateProfileVelocityMode", lResult, p_rlErrorCode);
//		lResult = MMC_FAILED;
//	}
//	else
//	{
//		list<long> velocityList;
//
//		velocityList.push_back(1000);
//		velocityList.push_back(300);
//		velocityList.push_back(600);
//		velocityList.push_back(900);
//		velocityList.push_back(200);
//
//		for(list<long>::iterator it = velocityList.begin(); it !=velocityList.end(); it++)
//		{
//			long targetvelocity = (*it);
//
//			stringstream msg;
//			msg << "move with target velocity = " << targetvelocity << " rpm, node = " << p_usNodeId;
//			LogInfo(msg.str());
//
//			if(VCS_MoveWithVelocity(p_DeviceHandle, p_usNodeId, targetvelocity, &p_rlErrorCode) == 0)
//			{
//				lResult = MMC_FAILED;
//				LogError("VCS_MoveWithVelocity", lResult, p_rlErrorCode);
//				break;
//			}
//
//			sleep(3);
//		}
//
//		if(lResult == MMC_SUCCESS)
//		{
//			LogInfo("halt velocity movement");
//
//			if(VCS_HaltVelocityMovement(p_DeviceHandle, p_usNodeId, &p_rlErrorCode) == 0)
//			{
//				lResult = MMC_FAILED;
//				LogError("VCS_HaltVelocityMovement", lResult, p_rlErrorCode);
//			}
//		}
//	}
//
//	return lResult;
//}

int Prepare(unsigned int* p_pErrorCode, unsigned short p_usNodeId)
{
	int lResult = MMC_SUCCESS;//int value is 0, failed int value is 1
    	BOOL oIsFault = 0;
        stringstream msg;
        if(p_usNodeId==3)
        {
            if(VCS_DigitalInputConfiguration(g_pKeyHandle, p_usNodeId, 5,0,1,0,1, p_pErrorCode)==0)
            {
                LogError("DigitalInputConfiguration", lResult, *p_pErrorCode);
		lResult = MMC_FAILED;
            }
            if(VCS_DigitalInputConfiguration(g_pKeyHandle, p_usNodeId, 6,1,1,0,1, p_pErrorCode)==0)
            {
                LogError("DigitalInputConfiguration", lResult, *p_pErrorCode);
		lResult = MMC_FAILED;
            }
        }
	if(VCS_GetFaultState(g_pKeyHandle, p_usNodeId, &oIsFault, p_pErrorCode ) == 0)
	{
		LogError("VCS_GetFaultState", lResult, *p_pErrorCode);
		lResult = MMC_FAILED;
                msg<<p_usNodeId;
                LogInfo(msg.str());
	}

	if(lResult==0)
	{
		if(oIsFault)
		{
			//stringstream msg;
			msg << "clear fault, node = '" << p_usNodeId << "'";
			LogInfo(msg.str());

			if(VCS_ClearFault(g_pKeyHandle, p_usNodeId, p_pErrorCode) == 0)
			{
				LogError("VCS_ClearFault", lResult, *p_pErrorCode);
//                                msg << "Fault Clear failed, node = '" << p_usNodeId << "'";
//                                LogInfo(msg.str());
				lResult = MMC_FAILED;
			}
		}
                //msg << lResult;
                //LogInfo(msg.str());
		if(lResult==0)
		{
			BOOL oIsEnabled = 0;

			if(VCS_GetEnableState(g_pKeyHandle, p_usNodeId, &oIsEnabled, p_pErrorCode) == 0)
			{
				LogError("VCS_GetEnableState", lResult, *p_pErrorCode);
				lResult = MMC_FAILED;
//                                msg << "get enable state error, node = '" << p_usNodeId << "'";
//                                LogInfo(msg.str());
			}

			if(lResult==0)
			{
				if(!oIsEnabled)
				{
					if(VCS_SetEnableState(g_pKeyHandle, p_usNodeId, p_pErrorCode) == 0)
					{
						LogError("VCS_SetEnableState", lResult, *p_pErrorCode);
						lResult = MMC_FAILED;
//                                                 msg << "set enable state error, node = '" << p_usNodeId << "'";
//                                                 LogInfo(msg.str());
					}
				}
			}
		}
	}
//        msg << lResult;
//        LogInfo(msg.str());
        return lResult;
}
int PrepareMotors(unsigned int* p_pErrorCode)
{
    int lResult = MMC_SUCCESS;
    if((lResult = Prepare(p_pErrorCode,node1))!=MMC_SUCCESS)
	{
		
		return lResult;
	}
    if((lResult = Prepare(p_pErrorCode,node2))!=MMC_SUCCESS)
	{
		
		return lResult;
	}
    if((lResult = Prepare(p_pErrorCode,node3))!=MMC_SUCCESS)
	{
		
		return lResult;
	}
//    if((lResult = Prepare(p_pErrorCode,node4))!=MMC_SUCCESS)
//	{
//		
//		return lResult;
//	}
    return lResult;
}

int   SetMotorModes(unsigned int & p_rlErrorCode)
{

        int lResult = MMC_SUCCESS;
	stringstream msg;

	msg << "set profile velocity mode, node = " << node1<<endl;
        msg << "set profile velocity mode, node = " << node2<<endl;
        msg << "set profile position mode, node = " << node3<<endl;
        //msg << "set profile position mode, node = " << node4<<endl;

	LogInfo(msg.str());

	if(VCS_ActivateProfileVelocityMode(g_pKeyHandle, node1, &p_rlErrorCode) == 0)
	{
		LogError("VCS_ActivateProfileVelocityMode", lResult, p_rlErrorCode);
		lResult = MMC_FAILED;
                msg<<"failed to activate"<<endl;
	}
	else
	{
		
	}
        if(VCS_ActivateProfileVelocityMode(g_pKeyHandle, node2, &p_rlErrorCode) == 0)
	{
		LogError("VCS_ActivateProfileVelocityMode", lResult, p_rlErrorCode);
		lResult = MMC_FAILED;
                msg<<"failed to activate"<<endl;
	}
	else
	{
        
        }
        if(VCS_ActivateProfilePositionMode(g_pKeyHandle, node3, &p_rlErrorCode) == 0)
	{
		LogError("VCS_ActivateProfilePositionMode", lResult, p_rlErrorCode);
		lResult = MMC_FAILED;
	}
	else
	{
            
        }
        if(VCS_SetPositionProfile(g_pKeyHandle, node3,(unsigned int)700, (unsigned int)3000, (unsigned int)3000, &p_rlErrorCode) == 0)
	{
		LogError("VCS_ActivateProfilePositionMode", lResult, p_rlErrorCode);
		lResult = MMC_FAILED;
	}
	else
	{
            
        }
//        if(VCS_ActivatePositionMode(g_pKeyHandle, node3, &p_rlErrorCode) == 0)
//	{
//		LogError("VCS_ActivateProfilePositionMode", lResult, p_rlErrorCode);
//		lResult = MMC_FAILED;
//	}
//	else
//	{
//            
//        }
//        if(VCS_ActivateProfilePositionMode(g_pKeyHandle, node4, &p_rlErrorCode) == 0)
//	{
//		LogError("VCS_ActivateProfilePositionMode", lResult, p_rlErrorCode);
//		lResult = MMC_FAILED;
//	}
//	else
//	{
//            
//        }
	return lResult;

}

int SteerHoming(unsigned int& p_pErrorCode)
{
        int lResult = MMC_SUCCESS;
	stringstream msg;
        unsigned int homingAcceleration=5000;
        unsigned int speedSwitch=100;
        unsigned int speedIndex=100;
        long homeOffset=4000;
        unsigned int currentThreshHold=0;
        long homePosition=0;
        char homeMethod=1;
        unsigned int timeout = 30000;
        int homingAttained;
        int homingError;
        if(VCS_ActivateHomingMode(g_pKeyHandle, node3, &p_pErrorCode) == 0) // Activate homing mode 
	{
		LogError("VCS_ActivateHomingMode", lResult, p_pErrorCode);
		lResult = MMC_FAILED;
                //msg<<"failed to activate"<<endl;
	}
	else
	{       // Set Homing Parameters
		if(VCS_SetHomingParameter(g_pKeyHandle, node3,homingAcceleration, speedSwitch, speedIndex, homeOffset, currentThreshHold, homePosition, &p_pErrorCode) == 0)
                {
                    LogError("VCS_SetHomingParameter", lResult, p_pErrorCode);
                    lResult = MMC_FAILED;
                    //msg<<"failed to activate"<<endl;
                }
                else
                {
                    if(VCS_FindHome(g_pKeyHandle, node3, homeMethod, &p_pErrorCode) == 0) // Start Homing
                    {
                        LogError("VCS_FindHome", lResult, p_pErrorCode);
                        lResult = MMC_FAILED;
                    }
                    else
                    {
//                        msg<<"In WaitForHomingAttained  1"<<endl;
//                        LogInfo(msg.str());
                        if(VCS_WaitForHomingAttained(g_pKeyHandle, node3, timeout, &p_pErrorCode) == 0)
                        {
                            LogError("VCS_WaitForHomingAttained", lResult, p_pErrorCode);
                            lResult = MMC_FAILED;
                        }
                        else
                        {
//                            msg<<"In WaitForHomingAttained  2"<<endl;
//                            LogInfo(msg.str());
                            if(VCS_GetHomingState(g_pKeyHandle, node3, &homingAttained, &homingError, &p_pErrorCode) == 0)
                            {
                                LogError("VCS_GetHomingState", lResult, p_pErrorCode);
                                lResult = MMC_FAILED;
                            }
                            else
                            {
//                                msg<<"In WaitForHomingAttained  3"<<endl;
//                                LogInfo(msg.str());
                                if(homingAttained == 0)
                                {
                                    msg<<"In WaitForHomingAttained  4"<<endl;
                                    LogInfo(msg.str());
                                    LogError("Homing Not Completed", lResult, p_pErrorCode);
                                    lResult = MMC_FAILED;
                                }
                                else
                                {
//                                    msg<<"In WaitForHomingAttained  5"<<endl;
//                                    LogInfo(msg.str());
                                    if(homingError == 1)
                                    {
                                        LogError("Homing Error Occurred", lResult, p_pErrorCode);
                                        lResult = MMC_FAILED;
                                    }
                                }
                            
                            }
                        }
                    }
                }
	}
//        msg << lResult;
//        LogInfo(msg.str());
      //  cout<<"result is  "<<lResult<<endl;
    return lResult;
}
//int Demo(unsigned int* p_pErrorCode)
//{
//	int lResult = MMC_SUCCESS;
//	unsigned int lErrorCode = 0;
//
//	lResult = DemoProfileVelocityMode(g_pKeyHandle, g_usNodeId, lErrorCode);
//
//	if(lResult != MMC_SUCCESS)
//	{
//		LogError("DemoProfileVelocityMode", lResult, lErrorCode);
//	}
//	else
//	{
//		lResult = DemoProfilePositionMode(g_pKeyHandle, g_usNodeId, lErrorCode);
//
//		if(lResult != MMC_SUCCESS)
//		{
//			LogError("DemoProfilePositionMode", lResult, lErrorCode);
//		}
//		else
//		{
//			if(VCS_SetDisableState(g_pKeyHandle, g_usNodeId, &lErrorCode) == 0)
//			{
//				LogError("VCS_SetDisableState", lResult, lErrorCode);
//				lResult = MMC_FAILED;
//			}
//		}
//	}
//	return lResult;
//}

void PrintHeader()
{
	SeparatorLine();

	LogInfo("Epos Command Library Example Program, (c) maxonmotor ag 2014");

	SeparatorLine();
}

int main(int argc, char** argv)
{
	int lResult = MMC_FAILED;
	unsigned int ulErrorCode = 0;
        long motor1Command = 0;
        long motor2Command = 0;
        long motor3Command = 0;
        long motor4Command = 0;
        int velFeedback=0;
        int positionFeedback=0;
        unsigned short dInputs;
        stringstream msg;
        RobotServer* robotThread = new RobotServer();
        pthread_t tID;
	pthread_attr_t attr;
        pthread_attr_init(&attr);
        //pthread_create(&tID,&attr,RobotServer::run,robotThread);
	PrintHeader();

	SetDefaultParameters();

	if((lResult = ParseArguments(argc, argv))!=MMC_SUCCESS)
	{
		return lResult;
	}
                            
	PrintSettings();
	if((lResult = OpenDevice(&ulErrorCode))!=MMC_SUCCESS)
	{
		LogError("OpenDevice", lResult, ulErrorCode);
		return lResult;
	}

	if((lResult = PrepareMotors(&ulErrorCode))!=MMC_SUCCESS)
	{
		LogError("PrepareMotors", lResult, ulErrorCode);
		return lResult;
	}
        if((lResult = SteerHoming(ulErrorCode))!=MMC_SUCCESS)
	{
		LogError("SteerHoming", lResult, ulErrorCode);
		return lResult;
	}
        if((lResult = SetMotorModes(ulErrorCode))!=MMC_SUCCESS)
	{
		LogError("Motor mode setting", lResult, ulErrorCode);
		return lResult;
	}
        cout<< "Motors are connected......"<<endl;
        pthread_create(&tID,&attr,RobotServer::run,robotThread);
        cout<< "Connecting robot...."<<endl;
        
        while(1)
        {
            motor3Command = -(long)(robotThread->steerAng*29.87);
            //motor4Command = (long)        
            motor1Command = (long)(robotThread->leftWhlVel*robotThread->gearRatioMtrRL*60/(2*3.14159265));//per revolution 2000 count
            motor2Command = (long)(robotThread->rightWhlVel*robotThread->gearRatioMtrRR*60/(2*3.14159265));//per revolution 2000 count

           //************ Send Command to Motors *****
            if(VCS_MoveToPosition(g_pKeyHandle, node3, motor3Command, 1, 1, &ulErrorCode) == 0)
            {
                    LogError("VCS_MoveToPosition", lResult, ulErrorCode);
                    lResult = MMC_FAILED;
                    break;
            }

            if(VCS_MoveWithVelocity(g_pKeyHandle, node1, -motor1Command, &ulErrorCode) == 0)
            {
                    lResult = MMC_FAILED;
                    LogError("VCS_MoveWithVelocity: Motor 1", lResult, ulErrorCode);
                    
                    break;
            }
            if(VCS_MoveWithVelocity(g_pKeyHandle, node2, motor2Command, &ulErrorCode) == 0)
            {
                    lResult = MMC_FAILED;
                    LogError("VCS_MoveWithVelocity: Motor 2", lResult, ulErrorCode);
                    break;
            }
            
            //****************** Recieve Values from Motors ******************
            if(VCS_GetVelocityIs(g_pKeyHandle, node1, &velFeedback, &ulErrorCode) == 0)
            {
                    lResult = MMC_FAILED;
                    LogError("VCS_GetVelocityls: Motor 1", lResult, ulErrorCode);
                    
                    break;
            }
            robotThread->tempLeftWhlVel = (int)velFeedback;
            if(VCS_GetVelocityIs(g_pKeyHandle, node2, &velFeedback, &ulErrorCode) == 0)
            {
                    lResult = MMC_FAILED;
                    LogError("VCS_GetVelocityls: Motor 2", lResult, ulErrorCode);
                    break;
            }
            robotThread->tempRightWhlVel = (int)velFeedback;
            if(VCS_GetPositionIs(g_pKeyHandle, node3, &positionFeedback, &ulErrorCode) == 0)
            {
                    lResult = MMC_FAILED;
                    LogError("VCS_GetPositionls: Motor 3", lResult, ulErrorCode);
                    break;
            }
            cout<<"Encoder value of Steer Motor : "<<positionFeedback<<endl;
            //cout<<"Left Motor Actual Velocity: "<<robotThread->tempLeftWhlVel<<endl;
            //cout<<"Right Motor Actual Velocity: "<<robotThread->tempRightWhlVel<<endl;
//            if(VCS_GetAllDigitalInputs(g_pKeyHandle, node2, &dInputs, &ulErrorCode) == 0)
//            {
//                    lResult = MMC_FAILED;
//                    LogError("VCS_GetAllDigitalInputs: Node 2", lResult, ulErrorCode);
//                    break;
//            }
            //sleep(1);
        }
        LogInfo("halt velocity movement");
//
//        if(VCS_HaltPositionMovement(g_pKeyHandle, node3, &ulErrorCode) == 0)
//        {
//                lResult = MMC_FAILED;
//                LogError("VCS_HaltPositionMovement", lResult, ulErrorCode);
//        }
//        if(VCS_HaltPositionMovement(g_pKeyHandle, node4, &ulErrorCode) == 0)
//        {
//                lResult = MMC_FAILED;
//                LogError("VCS_HaltPositionMovement", lResult, ulErrorCode);
//        }
        if(VCS_HaltVelocityMovement(g_pKeyHandle, node1, &ulErrorCode) == 0)
        {
                lResult = MMC_FAILED;
                LogError("VCS_HaltVelocityMovement", lResult, ulErrorCode);
        }
        if(VCS_HaltVelocityMovement(g_pKeyHandle, node2, &ulErrorCode) == 0)
        {
                lResult = MMC_FAILED;
                LogError("VCS_HaltVelocityMovement", lResult, ulErrorCode);
        }
//	if((lResult = Demo(&ulErrorCode))!=MMC_SUCCESS)
//	{
//		LogError("Demo", lResult, ulErrorCode);
//		return lResult;
//	}
//
//	if((lResult = CloseDevice(&ulErrorCode))!=MMC_SUCCESS)
//	{
//		LogError("CloseDevice", lResult, ulErrorCode);
//		return lResult;
//	}

	return lResult;
}
