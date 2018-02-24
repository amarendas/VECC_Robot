#include "RobotServer.h"


RobotServer::RobotServer()
{
        running = false;
        lWheelRadius = 50;
        rWheelRadius =50;
        rearWhlBase = 210;
        rightWhlVel = 0;
        leftWhlVel = 0;
        chasisDistFromRearWhl = 63;
        distanceBtwFAndRWhl = 515;
        gearRatioMtrRL = 60.66;
        gearRatioMtrRR = 60.66;
        gearRatioMtrFrnt = 21;
        gearRatioMtrPlt = 21;
        velocity = 0;
        angle = 0;
        plength = 0;
        light1 = false;
        light2 = false;
        odoX = 0;
        odoY = 0;
        odoTheta = 0;
        tempLeftWhlVel = 0;
        tempRightWhlVel = 0;
        steerAng = 0;
        timeStamp = 0;
        thetaCovFactor = 0.005493; //for two byte data 360/65536
        recvBuffer = NULL;
        sendBuffer = NULL;
        serverSocket.sin_family=AF_INET;
	serverSocket.sin_port=htons(8888);
	serverSocket.sin_addr.s_addr=inet_addr("31.30.3.173");

}

void RobotServer::conn()
{
	sockHandle=socket(AF_INET,SOCK_STREAM,0);
	cout<<"socket created"<<endl;
	
	if(bind(sockHandle,(struct sockaddr*)&serverSocket,sizeof(serverSocket))>=0)
	{
		cout<<"binding"<<endl;
                //perror("Binding done");
	}
	else
	{
		cout<<"error in binding"<<endl;
	}
	initListen();
}
	
void RobotServer::initListen()
{
	if(listen(sockHandle,3)==0)
	{
		cout<<endl<<"listening";
		cout<<endl<<"server ready,waiting for client"<<endl;
	}
	else
	{
		cout<<endl<<"error in listening";
		cout<<endl<<"errno"<<errno;
	}
        //cout<<endl<<"Waiting for client"<<endl;
	initAccept();
	
}
void RobotServer::initAccept()
{
    int n=sizeof(client);
    clientHandle=accept(sockHandle,(struct sockaddr*)&client,(socklen_t*)&n);
    if(clientHandle<0)
    {
        cout<<"error in accept"<<endl;
        close(clientHandle);
    }

}

int RobotServer:: receiveData()
{
        
	int noOfBytesReceived;	
	//cout<<"errno"<<errno;
        noOfBytesReceived = recv(clientHandle,recvBuffer,9,MSG_WAITALL);
        if(noOfBytesReceived<0)
        {
            perror("Error in receiving");
            return -1;
        }
        if(noOfBytesReceived==0)
        {
            perror("Client is shutdown");
            return -1;
        }
        cout<<"Bytes received : "<<noOfBytesReceived<<endl;
        //cout<<"First byte value: "<<recvBuffer[0]<<endl;
        for(int i=0;i<noOfBytesReceived;i++)
            printf("%hhx  ", recvBuffer[i]);
        printf("\n");
	return noOfBytesReceived;
	
}

int RobotServer::sendData()
{
    int noOfBytesSent;
    noOfBytesSent = send(clientHandle,sendBuffer,26,0);
    if(noOfBytesSent==-1)
    {
        perror("Error in Sending");
        return -1;
    }
    return noOfBytesSent;
}

void* RobotServer::run(void* object)
{
    int rvalue = 0;
    unsigned char temp;
    unsigned char temp2[2];
    RobotServer* obj= (RobotServer*)object;
    obj->conn();
    double radius = 0;
   
    
    obj->running = true;
    while(1)
    {
        
        obj->recvBuffer = new unsigned char[9];
        obj->sendBuffer = new unsigned char[26];
        
        rvalue = obj->receiveData();
        //printf("Received data: %d bytes\n",rvalue);
        cout<< "Received data: "<<rvalue<<endl;
        if(rvalue==-1)
        {
            obj->velocity = 0;
            obj->angle = 0;
            obj->plength = 0;
            obj->light1 = false;
            obj->light2 = false;
            obj->timeStamp = 0;
            break;
        }
        if(rvalue>0)
        {
            temp2[0]=obj->recvBuffer[1];
            temp2[1]=obj->recvBuffer[0];
            //obj->velocity = *((short int*)&obj->recvBuffer[0]);
            obj->velocity = *((short int*)&temp2[0]);
            temp2[0]=obj->recvBuffer[3];
            temp2[1]=obj->recvBuffer[2];
            //obj->angle = *((short int*)&obj->recvBuffer[2]) * 180/65535;
            obj->angle = *((short int*)&temp2[0]) * 180/65535;
            temp2[0]=obj->recvBuffer[5];
            temp2[1]=obj->recvBuffer[4];
            //obj->plength = *((short int*)&obj->recvBuffer[4]);
            obj->plength = *((short int*)&temp2[0]);
            temp = *((unsigned char*)&obj->recvBuffer[6]);
            if((temp & 0x01)==0x01)
                obj->light1 = true;
            else
                obj->light1 = false;
            
            if((temp & 0x02)==0x02)
                obj->light2 = true;
            else
                obj->light2 = false;
            
            temp2[0]=obj->recvBuffer[8];
            temp2[1]=obj->recvBuffer[7];
            //obj->timeStamp = *((unsigned short int*)&obj->recvBuffer[7]);
            obj->timeStamp = *((unsigned short int*)&temp2[0]);
            if(obj->angle>50)
                obj->angle = 50;
            if(obj->angle<-50)
                obj->angle = -50;
            if(obj->angle!=0)
            {
                radius = (obj->distanceBtwFAndRWhl/tan(obj->angle*PI/180)) - obj->chasisDistFromRearWhl;
                obj->leftWhlVel = (obj->velocity*radius/obj->lWheelRadius)/(radius+obj->chasisDistFromRearWhl+(obj->rearWhlBase/2));
                obj->rightWhlVel = (obj->velocity*(radius+2*obj->chasisDistFromRearWhl+obj->rearWhlBase)/obj->rWheelRadius)/(radius+obj->chasisDistFromRearWhl+(obj->rearWhlBase/2));
                obj->steerAng = (int)obj->angle;
            }
            else
            {
                obj->leftWhlVel =  (obj->velocity/obj->lWheelRadius);
                obj->rightWhlVel = (obj->velocity/obj->rWheelRadius);
                obj->steerAng = (int)obj->angle;
            }
            cout<<"Velocity: "<<obj->velocity<<endl;
            cout<<"Angle: "<<obj->angle<<endl;
            cout<<"Platform length: "<<obj->plength<<endl;
            cout<<"Left wheel velocity: "<<obj->leftWhlVel<<endl;
            cout<<"Right wheel velocity: "<<obj->rightWhlVel<<endl;
            cout<<"Radius: "<<radius<<endl;
            //int vel = *((short int*)&temp2[0]);
            //cout<<"Velocity: "<<vel<<endl;

        }
        memcpy(obj->sendBuffer+0,&obj->odoX,4);
        memcpy(obj->sendBuffer+4,&obj->odoY,4);
        memcpy(obj->sendBuffer+8,&obj->odoTheta,4);
        memcpy(obj->sendBuffer+12,&obj->tempLeftWhlVel,4);
        memcpy(obj->sendBuffer+16,&obj->tempRightWhlVel,4);
        memcpy(obj->sendBuffer+20,&obj->steerAng,4);
        memcpy(obj->sendBuffer+24,&obj->timeStamp,2);
        
        rvalue = obj->sendData();
        //printf("Sent data: %d bytes\n",rvalue);
        cout<<"Sent data: "<<rvalue<<endl;
        
        if(rvalue==-1)
        {
            obj->velocity = 0;
            obj->angle = 0;
            obj->plength = 0;
            obj->light1 = false;
            obj->light2 = false;
            obj->timeStamp = 0;
            break;
        }
        delete(obj->recvBuffer);
        delete(obj->sendBuffer);
        //sleep(10);
    }
    close(obj->clientHandle);
    close(obj->sockHandle);
    RobotServer::run(obj);
    

}

                
