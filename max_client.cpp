#include<iostream>
#include<string.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/errno.h>
#include<math.h>
using namespace std;
class trial
{
	private:
		struct sockaddr_in server,client;
		int s,n;
		int b1[7];
		
	public:
		void conn();
		int work(int s);
};

void trial::conn()
{
	s=socket(AF_INET,SOCK_STREAM,0);
	
	server.sin_family=AF_INET;
	server.sin_port=8000;
	server.sin_addr.s_addr=inet_addr("127.0.0.1");
	cout<<endl<<"client ready...";
	work(s);
}

int trial::work(int s)
{
	int c;
	n=sizeof(server);
	c=connect(s,(struct sockaddr*)&server,n);
	cout<<c;
	cout<<"errno"<<errno;
	while(c==0)
	{
		cout<<"enter the message:"<<endl;
		
                
int i,light, vone,vtwo,steer,a,b,c;
   
  
  cout<<endl<<"Enter the light value(1 for off and 0 for on and 6 to disconnect from server) =";
  cin>>b1[0];

  cout<<endl<<"Enter the V1 value =";
  cin>>vone;

  cout<<endl<<"Enter the V2 value =";
  cin>>vtwo;

  cout<<endl<<"Enter the Steer angle=";
  cin>>steer;

  // if negative:1
  // if positive:0
  if(vone<0)
  {
    b1[1]=1;
    b1[2]=-vone;
  }
  else
{
    b1[1]=0;
    b1[2]=vone;
}
  if(vtwo<0)
  {
    b1[3]=1;
    b1[4]=-vtwo;
  }
  else
{
   b1[3]=0;
   b1[4]=vtwo;
}
  if(steer<0)
  {
     b1[5]=1;
     b1[6]=-steer;
  }
  else
{
     b1[5]=0;
     b1[6]=steer;
 }
  
  /*for(i=0;i<7;i++)
  {
    cout<<b1[i]<<endl;
  }*/
              


		send(s,b1,sizeof(b1),0);
		
                if(b1[0]==6) 
		{
                      
			close(s);
			return 0;
                        
		}
}
	
	return 0;
     }
int main()
{
	trial t;
	t.conn();
	return 0;
}

























	
