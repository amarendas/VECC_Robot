/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   RobotServer.h
 * Author: Shishir Singh
 *
 * Created on 14 April 2017, 19:13
 */

#ifndef ROBOTSERVER_H
#define ROBOTSERVER_H



#endif /* ROBOTSERVER_H */

#include<iostream>
#include<string.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/errno.h>
#include<math.h>
#define PI 3.14159265
using namespace std;

class RobotServer
{
	public: 
               
                int velocity;
                double angle;
                short int plength;
                bool light1;
                bool light2;
                int odoX;
                int odoY;
                int odoTheta;
                double leftWhlVel;
                double rightWhlVel;
                int steerAng;
                float thetaCovFactor;
                unsigned short int timeStamp;
                int sockHandle;
                int clientHandle;
                sockaddr_in serverSocket;
                sockaddr_in client;
                unsigned char* recvBuffer;
                unsigned char* sendBuffer;
                double distanceBtwFAndRWhl;
                double chasisDistFromRearWhl;
                int tempLeftWhlVel; //in radian/sec
                int tempRightWhlVel; //in radian/sec
                double rearWhlBase;
                double lWheelRadius;
                double rWheelRadius;
                double gearRatioMtrRL;
                double gearRatioMtrRR;
                double gearRatioMtrFrnt;
                double gearRatioMtrPlt;
                bool running;
                RobotServer();                
                void conn();
                int receiveData();
                int sendData();
                void initListen();
                void initAccept();
                static void* run(void* object);
                
};