#include "deploy.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>

#include <vector>
#include <cmath>
#include <queue>
#include <cstring>
#include <algorithm>
#include <cstdio>

#include <time.h>
#include <sys/timeb.h>
#include <unistd.h>
#include <signal.h>

#define MAXN 1600
#define INF 999999
#define sizeNum 600
#define limit 87	
using namespace std;


// translate the char to int
void  my_atoi(int *output, const char *str)
{
	//assert(NULL !=str);

	int lenth = strlen(str);
	*output = 0;
	while (lenth>0)
	{
		while (isdigit(*str))
		{
			*output = *output * 10 + *str - '0';
			str++;
			lenth--;
		}
		output++;
		str++;
		lenth--;
	}
}

/*整数转化成字符串*/  
void my_itoa(int num, string &str)  
{  
    int i = 0;  
    char temp[100];  
    if(num==0)
    {	
    	str+='0';
    }
    else
    {
    	while(num)  
    	{  
        	temp[i] = num % 10 + '0';   //取模运算得到从后往前的每一个数字变成字符  
        	num = num / 10;  
     	   i++;  
    	}  
    	temp[i] = 0;    //字符串结束标志  
      
    	i = i - 1;     //回到temp最后一个有意义的数字  
    	while(i >= 0)  
    	{  
        	str+= temp[i];   
        	i--;     
    	}
    }     
}  
  
struct Edge
{
	int from, to, cap, cost, flow;
	Edge(int u, int v, int Cap, int Cost, int Flow) :from(u), to(v), cap(Cap), cost(Cost), flow(Flow){}
};

struct Consumer
{
	int start;
	int end;
	int cost;
};

int again=0;

vector<Edge>edges;
vector<Edge> temp;
vector<Edge> last;
vector<Edge> last2;// save the second
vector<Edge> ll;

vector<int> gtemp[MAXN];
vector<int> gtemp2[MAXN];
vector<int> G[MAXN];
vector<int>Server;//存储服务器编号
vector<int>Server_del;//存储服务器编号
vector<int>::iterator ser;
vector<int>finalCost;//存储（1-n）台服务器的费用
vector<int>band;  //存储路径流量

vector<int>path[MAXN];//存储路径
int P = 0;			 //存储路径数目

int inq[MAXN];
int d[MAXN];
int p[MAXN];
int a[MAXN];

int nodeNum = 0;   //节点数目
int lastflow = 0;	
int maxflow = 0;   //消费点需求和
int totalCost;		
int flow;
int outcome = INF;
int del=0;
int source;      // 超级源
int destination; //超级汇
int serverCost;	//每台服务器费用

int saveFlag = 0; //是否保存路径  最后一次才保存

// 最大流量 和 相对低费用
int maxFlow = 0;
int low = INF;
vector<int> Left;

void init(int n){
	for (int i = 0; i <= n; i++)G[i].clear();
	edges.clear();
}

void AddEdge(int from, int to, int cap, int cost)
{
	edges.push_back(Edge(from, to, cap, cost, 0));
	edges.push_back(Edge(to, from, 0, -cost, 0));
	int m = edges.size();
	G[from].push_back(m - 2);
	G[to].push_back(m - 1);
}


bool BellmanFord(int s, int t, int& flow, int& cost){
	for (int i = 0; i <= MAXN; i++)
	{
		d[i] = INF;

	}
	memset(inq, 0, sizeof(inq));
	d[s] = 0; inq[s] = 1; p[s] = 0; a[s] = INF;


	queue<int>Q;
	Q.push(s);
	while (!Q.empty())
	{
		int u = Q.front(); Q.pop();

		inq[u] = 0;
		for (int i = 0; i<(int)G[u].size(); i++)
		{
			Edge& e = edges[G[u][i]];

			if (e.cap>e.flow&&d[e.to]>d[u] + e.cost)
			{
				d[e.to] = d[u] + e.cost;
				p[e.to] = G[u][i];
				a[e.to] = min(a[u], e.cap - e.flow);
				if (!inq[e.to])
				{
					Q.push(e.to);
					inq[e.to] = 1;
				}
			}
		}

	}
	if (d[t] == INF)return false;

	int u = t;
	//增广
	while (u != s){
		edges[p[u]].flow += a[t];
		edges[p[u] ^ 1].flow -= a[t];
		u = edges[p[u]].from;

	}

	if (saveFlag)  //存储路径和带宽
	{
		int flag = 0;
		for (int u = t; u != s; u = edges[p[u]].from)
		{
			if (flag >= 1)
			{
				path[P].push_back(u);
			}
			flag++;

			//temp[index] = u;
		}
		P++;
		band.push_back(a[t]);
	}

	flow += a[t];
	cost += d[t] * a[t];
	return true;
}
int Mincost(int s, int t){
	int flow = 0, cost = 0;
	while (BellmanFord(s, t, flow, cost));
	lastflow=flow;
	return cost;
}


int process(Consumer *consumer)
{
	maxFlow=0;
	low=INF;
	int cosNum=Server.size();
	Server.push_back(0);
	for (int i = 0; i<cosNum; i++)
	{
		AddEdge(source, consumer[i].end, INF, 0);
		totalCost = Mincost(source, destination);
		flow = lastflow;
		
		//edges 和 G 复原
		edges = temp;
		G[nodeNum].pop_back();
		G[i].pop_back();

			if (totalCost<low)
			{
				low = totalCost;
				outcome = totalCost;
				Server[Server.size() - 1] = consumer[i].end;
			}


		//}

	}
	// 循环结束，将确定的一个服务器点 加入图中
	edges = temp;
	AddEdge(source, Server[Server.size() - 1], INF, 0);
	cout<<"add "<<Server[Server.size()-1]<<" ";
	temp = edges; //更新存储临时量
	outcome+=Server.size()*serverCost;
	return outcome; //添加一个服务器后  带宽费用
}
void deleteNode_tuihuo(int n)
{	low=INF;
	del=0;
	edges=last;
	maxFlow=0;
	for(int i=0;i<n;i++)
	{
		for(int k=0;k<n;k++)
		{
			if(k!=i)
			{
				AddEdge(source, Server[k], INF, 0);
			}
		}
		totalCost = Mincost(source, destination);
		if(lastflow>maxFlow)
		{	
				low=totalCost;
				del=i;
				outcome=totalCost+(n-1)*serverCost;
				maxFlow=lastflow;
		}
		
	  else	if(lastflow==maxFlow)
		{
			if(totalCost<=low)
			{
				low=totalCost;
				del=i;
				outcome=totalCost+(n-1)*serverCost;
				maxFlow=lastflow;
			}
		
		}
		ll=last2;
		edges.swap(ll);
		for (int i = 0; i<MAXN; i++)
		{
			G[i] = gtemp[i];
		}
	}
	
	ser=Server.begin()+del;
	del=Server[del];
	Left.push_back(del);
	Server.erase(ser);	
	cout<<"\\"<<n-1<<"  server,the cost is "<<outcome<<" flow "<<maxFlow<<" delete "<<del;
	cout<<endl;
}	
void deleteNode(int n)
{	low=INF;
	del=0;
	edges=last;
	maxFlow=0;
	for(int i=0;i<n;i++)
	{
		for(int k=0;k<n;k++)
		{
			if(k!=i)
			{
				AddEdge(source, Server[k], INF, 0);
			}
		}
		totalCost = Mincost(source, destination);
		if(lastflow>maxFlow)
		{	
				low=totalCost;
				del=i;
				outcome=totalCost+(n-1)*serverCost;
				maxFlow=lastflow;
		}
		
	  else	if(lastflow==maxFlow)
		{
			if(totalCost<=low)
			{
				low=totalCost;
				del=i;
				outcome=totalCost+(n-1)*serverCost;
				maxFlow=lastflow;
			}
		
		}
		ll=last2;
		edges.swap(ll);
		for (int i = 0; i<MAXN; i++)
		{
			G[i] = gtemp[i];
		}
	}
	
	ser=Server.begin()+del;
	del=Server[del];
	Server.erase(ser);	
	cout<<"\\"<<n-1<<"  server,the cost is "<<outcome<<" flow "<<maxFlow<<" delete "<<del;
	cout<<endl;
}	

void deleteNode3(int n,int t,int p)
{	low=INF;
	del=0;
	edges=last;
	maxFlow=0;
	for(int i=t*p-t;i<p*t+p-t;i++)
	{
		for(int k=0;k<n;k++)
		{
			if(k!=i)
			{
				AddEdge(source, Server[k], INF, 0);
			}
		}
		totalCost = Mincost(source, destination);
		if(lastflow>maxFlow)
		{	
				low=totalCost;
				del=i;
				outcome=totalCost+(n-1)*serverCost;
				maxFlow=lastflow;
		}
		
	  else	if(lastflow==maxFlow)
		{
			if(totalCost<=low)
			{
				low=totalCost;
				del=i;
				outcome=totalCost+(n-1)*serverCost;
				maxFlow=lastflow;
			}
		
		}
		ll=last2;
		edges.swap(ll);
		for (int i = 0; i<MAXN; i++)
		{
			G[i] = gtemp[i];
		}
	}
	
	ser=Server.begin()+del;
	del=Server[del];
	Server.erase(ser);	
	cout<<"\\"<<again<<"\\"<<n-1<<"  server,the cost is "<<outcome;
	cout<<endl;
}

int my_print_time()
{
    struct timeb rawtime;
    ftime(&rawtime);
  
    static unsigned long s = rawtime.time;
	unsigned long out_s=rawtime.time-s;
    printf("time %d ",out_s);
    return out_s;
}


vector<vector<int> > ivec_road;//建立一个nodeNum大小的矩阵，用于存储链路的关系，ivec_road[i][j]=20,表示path保存的路径中从i->j方向的总流量为20
vector<int> path_final[MAXN];//保存最终的路径，格式与path相同
vector<int> band_final;//保存最终每条路径的流量
int f_start = -1;//存储ivec_road中第一大于0的起点，s存储终点
int s_end = -1;
vector<int> temp_road;//缓存一次寻找出来的一条链路的所有网络节点
vector<int> temp_flow;//缓存寻找出来的链路的每个节点间的流量
int xun_count = 0;//记录循环的次数
int diedai(int start)
{
	int i = 0;
	for (; i < nodeNum; ++i)
	{
		if (ivec_road[start][i]>0)
		{
			f_start = start;
			s_end = i;
			temp_road.push_back(f_start);
			temp_flow.push_back(ivec_road[f_start][s_end]);
			break;
		}
	}
	if (i == nodeNum)
		return 0;
	return 1;

}

//将BellmanFord中保存的路径和流量转化成正确的格式
void ctrl_s(vector<int> *path1, vector<int> &band1)
{

	//根据path和band初始化ivec_road
	//cout << "ctrls" << endl;
	ivec_road = vector<vector<int>>(nodeNum, vector<int>(nodeNum, 0));
	for (int i = 0; i < P; ++i)
	{
		if (path1[i].size() == 1)
			continue;
		for (int j = path1[i].size() - 1; j>0; --j)
		{
			ivec_road[path1[i][j]][path1[i][j - 1]] += band1[i];

		}
	}

	//方向相反路径的流量相互抵消，取数值较大的为该链路的方向
	for (int i = 0; i < nodeNum; ++i)
	{
		for (int j = 0; j < nodeNum; ++j)
		{
			if (ivec_road[i][j]>ivec_road[j][i])
			{
				ivec_road[i][j] -= ivec_road[j][i];
				ivec_road[j][i] = 0;
			}
			else{
				ivec_road[j][i] -= ivec_road[i][j];
				ivec_road[i][j] = 0;
			}
		}
	}

	int sum_flow = 0;//存储ivec_road中所有数据的和
	for (int i = 0; i < nodeNum; ++i)
	{
		for (int j = 0; j < nodeNum; ++j)
		{
			sum_flow += ivec_road[i][j];
		}
	}

	//一次循环找出一条链路
	while (sum_flow>0)
	{
		f_start = -1;
		s_end = -1;
		temp_road.clear();
		temp_flow.clear();
		//cout << "f1" << endl;

		//找出ivec_road中第一个大于0的链路
		for (auto iter = Server_del.begin(); iter != Server_del.end(); ++iter)
		{
			int flag_first = 0;//用于在找到了第一个大于0的数后，跳出第一层循环
			for (int j = 0; j <nodeNum; ++j)
			{
				if (ivec_road[*iter][j]>0)
				{
					flag_first = 1;
					f_start = *iter;
					s_end = j;
					break;
				}
			}
			if (flag_first == 1)
				break;
		}
	

		temp_road.push_back(f_start);
		temp_flow.push_back(ivec_road[f_start][s_end]);
		
		int xun_huan = 1;
		while (xun_huan)
		{
			xun_huan = diedai(s_end);
		}
		temp_road.push_back(s_end);

		sort(temp_flow.begin(), temp_flow.end());
		int min_flow = temp_flow[0];
		//更新ivec_road
		for (int i = 0; i <(int) temp_road.size() - 1; ++i)
		{
			ivec_road[temp_road[i]][temp_road[i + 1]] -= min_flow;
		}
		//重新求和
		sum_flow = 0;
		for (int i = 0; i < nodeNum; ++i)
		{
			for (int j = 0; j < nodeNum; ++j)
			{
				sum_flow += ivec_road[i][j];
			}
		}

		for (auto iter = temp_road.rbegin(); iter != temp_road.rend(); ++iter)
		{
			path_final[xun_count].push_back(*iter);
		}
		band_final.push_back(min_flow);
		xun_count++;

	}






}

//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num, char * filename)
{
	my_print_time();
	int my_time=0;
	
	
	int serverNum=0;
	
	int minFinalCost = INF;
	saveFlag = 0;
	P = 0;
	int firstline[3] = { 0, 0, 0 };
	my_atoi(firstline, topo[0]);

	int netNum, cosNum;
	nodeNum = firstline[0];
	netNum = firstline[1];
	cosNum = firstline[2];

	my_atoi(firstline, topo[2]);
	serverCost = firstline[0];

	vector<int> dotflow[nodeNum];//存储每个网络节点每条链路的总带宽，dotflow[i]存放也第i个节点的相连的链路的带宽
	vector<int> dotcost[nodeNum];//每个网络节点周围链路的单位费用

	// init the common node  it has two direction
	for (int i = 4; i<netNum + 4; i++)
	{
		int fourline[4] = { 0, 0, 0, 0 };
		my_atoi(fourline, topo[i]);

		dotflow[fourline[0]].push_back(fourline[2]);//将该条链路的带宽存到起始点的vector
		dotflow[fourline[1]].push_back(fourline[2]);//将该条链路的带宽存到终点的vector，下为单位费用
		dotcost[fourline[0]].push_back(fourline[3]);
		dotcost[fourline[1]].push_back(fourline[3]);

		AddEdge(fourline[0], fourline[1], fourline[2], fourline[3]);
		AddEdge(fourline[1], fourline[0], fourline[2], fourline[3]);
	}

	vector<int> dottotalflow;//每个网络节点的总流量，dottotalflow[i]表示i点发出的总带宽，下同
	vector<int> dottotalcost;//每个网络节点的总单位费用
	vector<int> temp_dottotalflow;//缓存的dottotalflow
	vector<int> bignum;//存储dottotalflow中总流量前5大的节点的编号，实际数量是大于等于5.
	for (int i = 0; i < nodeNum; ++i)
	{
		int sumflow = 0, sumcost = 0;
		

		for (auto v : dotflow[i])
		{
			sumflow += v;
		}
		dottotalflow.push_back(sumflow);
		
		for (auto v : dotcost[i])
		{
			sumcost += v;
		}
		dottotalcost.push_back(sumcost);
		
	}

	temp_dottotalflow = dottotalflow;
	sort(temp_dottotalflow.begin(), temp_dottotalflow.end());//排序
	int bigone = temp_dottotalflow[temp_dottotalflow.size() - 1];//bigone表示总流量最大的数值
	int count_five = 0;//用于计数，当其等于4时，表示已经找到第五个大的数值
	int five_big = temp_dottotalflow[temp_dottotalflow.size() - 1];//用于存储第五大的数值
	for (auto iter = temp_dottotalflow.rbegin(); iter != temp_dottotalflow.rend(); ++iter)
	{
		if (*iter < bigone)
		{
			++count_five;
			bigone = *iter;
		}
		if (count_five == 5)
		{
			five_big = *iter;
			break;
		}
	}

	//将所有大于five_big的节点编号存入bignum
	for (int i=0; i< (int )dottotalflow.size(); ++i)
	{
		if (dottotalflow[i] >= five_big)
			bignum.push_back(i);
	}

	Consumer consumer[cosNum];
	int count = 0;

	//cout << "网络节点流量" << endl;
	for (int i = netNum + 5; i<netNum + 5 + cosNum; i++)
	{
		int threeline[3] = { 0, 0, 0 };
		my_atoi(threeline, topo[i]);
		consumer[count].start = threeline[0];
		consumer[count].end = threeline[1];
		consumer[count].cost = threeline[2];
		count++;
	}

	source = nodeNum;
	destination = nodeNum + 1;

	//    add the destination(T）  only in    
	for (int i = 0; i < count; i++)
	{
		AddEdge(consumer[i].end, destination, consumer[i].cost, 0);
		maxflow += consumer[i].cost;

	}
	cout << "maxflow" << maxflow << endl;

	//以上不需要修改
	
	
	// 将 没用添加任何服务器的  edges 和 G 存储到 last 和 gtemp中
	last = edges;
	last2=edges;
	for (int i = 0; i<MAXN; i++)
	{
		gtemp[i] = G[i];
	}

	int del_flag = 0;
	//divide into different size
	if(nodeNum<300)
		{
		int mini;
		vector<int>mini_Server;
		temp = edges;
			for (int i = 0; i < count; i++)
			{
				Server.push_back(consumer[i].end);
			}
		
			minFinalCost = INF;
		
			for (int k = 0; k<cosNum; k++)
			{
				deleteNode_tuihuo(Server.size());
				if (minFinalCost<outcome || maxFlow<maxflow) break;
				else
				{
					minFinalCost = outcome;
				}

			}
			mini=minFinalCost;
			mini_Server=Server;
			mini_Server.push_back(del);
			
			cout << minFinalCost << ' ' << Server.size() << endl;
			//THE FINAL NODE IS SERVER +DEL
			for (int i = 0; i<MAXN; i++)
			{
				G[i] = gtemp[i];
			}
	
			edges = last;
		 cout<<"Server "<<Server.size()<<" Left "<<Left.size()<<endl;
			
			// do some change 
			// create  new result
			// change one node 	
			int last_cost=minFinalCost;
			int d=0,b=0;
			float dc=0;
			int curr=0;
			int add=0;
			int change=0;
			vector<int>::iterator delet;
			vector<int> Answer;
			vector<int> Left_last;
			vector<int>over;
			
			
			Left_last=Left;
			Answer=Server;
			srand(time(0));
			int way=0;
			float T=10000;
			float T_min=100;
			float R=1-0.00099;
			float rnd=(rand()%10000)/10000.0;
			
			int kkkk=0;
			minFinalCost=INF;
			my_time = my_print_time();
		while(T>T_min)	
		{	
			
			if(my_time<limit)
			
			{ 	way=kkkk%3;
				switch (way)
				{
		
				 case 0:	if(!Server.empty())
						{	d=rand()%Server.size();
							delet=Server.begin()+d;
							d=Server[d];	
							Server.erase(delet);
							Left.push_back(d);
							break;
						}
			
					//add
				case 1:	if(!Left.empty())
						{
						  	b=rand()%Left.size();
							add=Left[b];
							Server.push_back(add);
							delet=Left.begin()+b;
							Left.erase(delet);
							break;
						}	

					//change;
				 case 2:if(!Left.empty()&&!Server.empty())
						{
							d=rand()%Server.size();
							b=rand()%Left.size();
							change=Server[d];
							Server[d]=Left[b];
							Left[b]=change;
							break;
						}
				}
				rnd=0.98;
				for (int i = 0; i<(int)Server.size(); i++)
				{
					AddEdge(source, Server[i], INF, 0);
				}
				kkkk++;
				lastflow=0;
				curr = Mincost(source, destination)+Server.size()*serverCost;
			
			
				dc=curr-last_cost;
				cout<<my_time<<" way "<<way<<" cost "<<curr<<" dc  "<<dc<<" flow "<<lastflow<<endl;
				if(dc<0 && lastflow>=maxflow)
				{
					Answer=Server;
					last_cost=curr;
					Left_last=Left;
				//	cout<<"better "<<endl;
				}
				else 
				{
					double p=exp(-dc/T);
					if(p>rnd && lastflow>=maxflow)
					{
						Answer=Server;
						last_cost=curr;
						Left_last=Left;
					}
					else
						{
							Left=Left_last;
							Server=Answer;
						
						}
				}
				
				if(last_cost<minFinalCost&&lastflow>=maxflow)
				{
					cout<<"last "<<lastflow<<" maxFlow "<<maxflow<<endl;
					minFinalCost=last_cost;
					cout<<"change ******"<<endl;
					vector<int>over_temp;
					over_temp=Server;
					over.swap(over_temp);
				
					cout<<minFinalCost<<endl;
				}
			
			 	for (int i = 0; i<MAXN; i++)
				{
					G[i] = gtemp[i];
				}
				edges = last;
			
				T=R*T;
				if(kkkk>15000||my_time>60) break;
			}
			else break;
		}
		 cout<<"Server "<<Server.size()<<" Left "<<Left.size()<<endl;
		 cout << minFinalCost<<' '<<mini<<" " << ' ' << Server.size() << endl;
		 my_print_time();
		 cout<<" kkkk "<<kkkk<<endl;
		 cout<<endl;
		 cout<<maxflow<<endl;
			if(minFinalCost>=mini)
			{
				Server=mini_Server;
			}
		 	else	Server=over;
		   for (int i = 0; i<MAXN; i++)
			{
				G[i] = gtemp[i];
			}
			edges = last;
		
			sort(Server.begin(),Server.end());
			for (int i = 0; i<(int)Server.size(); i++)
			{
				AddEdge(source, Server[i], INF, 0);
				cout<<Server[i]<<" ";
			}
			cout<<endl;
			serverNum = Server.size() ;
			Server_del = Server;
	}
	else if (nodeNum<600)
	{
		int mini;
		vector<int>mini_Server;
		temp = edges;
			for (int i = 0; i < count; i++)
			{
				Server.push_back(consumer[i].end);
			}
		
			minFinalCost = INF;
		
			for (int k = 0; k<cosNum; k++)
			{
				deleteNode_tuihuo(Server.size());
				if (minFinalCost<outcome || maxFlow<maxflow) break;
				else
				{
					minFinalCost = outcome;
				}

			}
			mini=minFinalCost;
			mini_Server=Server;
			mini_Server.push_back(del);
			
			cout << minFinalCost << ' ' << Server.size() << endl;
			//THE FINAL NODE IS SERVER +DEL
			for (int i = 0; i<MAXN; i++)
			{
				G[i] = gtemp[i];
			}
	
			edges = last;
		 cout<<"Server "<<Server.size()<<" Left "<<Left.size()<<endl;
			
			// do some change 
			// create  new result
			// change one node 	
			int last_cost=minFinalCost;
			int d=0,b=0;
			float dc=0;
			int curr=0;
			int add=0;
			int change=0;
			vector<int>::iterator delet;
			vector<int> Answer;
			vector<int> Left_last;
			vector<int>over;
			
			
			Left_last=Left;
			Answer=Server;
			srand(time(0));
			int way=0;
			float T=10000;
			float T_min=100;
			float R=1-0.00200;
			float rnd=(rand()%10000)/10000.0;
			
			int kkkk=0;
			minFinalCost=INF;
			my_time = my_print_time();
		while(T>T_min)	
		{	
			
			if(my_time<limit)
			
			{ 	way=kkkk%3;
				switch (way)
				{
		
				 case 0:	if(!Server.empty())
						{	d=rand()%Server.size();
							delet=Server.begin()+d;
							d=Server[d];	
							Server.erase(delet);
							Left.push_back(d);
							break;
						}
			
					//add
				case 1:	if(!Left.empty())
						{
						  	b=rand()%Left.size();
							add=Left[b];
							Server.push_back(add);
							delet=Left.begin()+b;
							Left.erase(delet);
							break;
						}	

					//change;
				 case 2:if(!Left.empty()&&!Server.empty())
						{
							d=rand()%Server.size();
							b=rand()%Left.size();
							change=Server[d];
							Server[d]=Left[b];
							Left[b]=change;
							break;
						}
				}
				rnd=0.98;
				for (int i = 0; i<(int)Server.size(); i++)
				{
					AddEdge(source, Server[i], INF, 0);
				}
				kkkk++;
				lastflow=0;
				curr = Mincost(source, destination)+Server.size()*serverCost;
			
			
				dc=curr-last_cost;
				cout<<my_time<<" way "<<way<<" cost "<<curr<<" dc  "<<dc<<" flow "<<lastflow<<endl;
				if(dc<0 && lastflow>=maxflow)
				{
					Answer=Server;
					last_cost=curr;
					Left_last=Left;
				//	cout<<"better "<<endl;
				}
				else 
				{
					double p=exp(-dc/T);
					if(p>rnd && lastflow>=maxflow)
					{
						Answer=Server;
						last_cost=curr;
						Left_last=Left;
					}
					else
						{
							Left=Left_last;
							Server=Answer;
						
						}
				}
				
				if(last_cost<minFinalCost&&lastflow>=maxflow)
				{
					cout<<"last "<<lastflow<<" maxFlow "<<maxflow<<endl;
					minFinalCost=last_cost;
					cout<<"change ******"<<endl;
					vector<int>over_temp;
					over_temp=Server;
					over.swap(over_temp);
				
					cout<<minFinalCost<<endl;
				}
			
			 	for (int i = 0; i<MAXN; i++)
				{
					G[i] = gtemp[i];
				}
				edges = last;
			
				T=R*T;
				if(kkkk>1600||my_time>60) break;
			}
			else break;
		}
		 cout<<"Server "<<Server.size()<<" Left "<<Left.size()<<endl;
		 cout << minFinalCost<<' '<<mini<<" " << ' ' << Server.size() << endl;
		 my_print_time();
		 cout<<" kkkk "<<kkkk<<endl;
		 cout<<endl;
		 cout<<maxflow<<endl;
			if(minFinalCost>=mini)
			{
				Server=mini_Server;
			}
		 	else	Server=over;
		   for (int i = 0; i<MAXN; i++)
			{
				G[i] = gtemp[i];
			}
			edges = last;
		
			sort(Server.begin(),Server.end());
			for (int i = 0; i<(int)Server.size(); i++)
			{
				AddEdge(source, Server[i], INF, 0);
				cout<<Server[i]<<" ";
			}
			cout<<endl;
			serverNum = Server.size() ;
			Server_del = Server;
	}

	else{
		vector<int>S;
		temp = edges;
		for (int i = 0; i < count; i++)
		{
			Server.push_back(consumer[i].end);
		}

		int turn=0;
		int step=4;
		int num=cosNum;
		int oneturn=0;
		oneturn=num/step;
		
		for (int k = 0; k<cosNum; k++)
		{
			my_time = my_print_time();
			if(my_time<limit)
			{	
				
				if(turn==oneturn)
				{
					again++;
					num=num-num/step;
					if(again>1)
					{
						step=5;
					}
					else if(again>3)
					{
						step=6; 	
					}
					else if(again>4)
					{
						step=7;
					}
					
					oneturn=num/step;
					turn=0;
				}
	
				deleteNode3(Server.size(),turn%oneturn,step);
				turn++;
				
				if (maxFlow<maxflow)
				{
					del_flag = 1;
					break;
				}
				else
				{
					if(outcome<minFinalCost)
					{
						minFinalCost = outcome;
						S=Server;
					}
				}
			}	
			else break;		

		 }
		
		cout << minFinalCost << endl;
		edges = last2;
		//THE FINAL NODE IS SERVER +DEL
		for (int i = 0; i<MAXN; i++)
		{
			G[i] = gtemp[i];
		}

		for (int i = 0; i<(int)S.size(); i++)
		{
			AddEdge(source, S[i], INF, 0);
		}
		Server_del = S;
		serverNum = S.size();
		if (del_flag)
		{
			AddEdge(source, del, INF, 0);
			Server_del.push_back(del);
			serverNum = S.size() + 1;
		}
		
	}

	//out put 
	string out;
	saveFlag=1;
	
	int a=Mincost(source, destination)+serverNum*serverCost;
	ctrl_s(path, band);
	cout<<"final fee"<<a<<" flow "<<lastflow<<endl;
	//以下可不用管
	for (int i = 0; i<P; i++)
	{
		if (path[i].size() != 1)
			continue;
		path_final[xun_count].push_back(path[i][0]);
		band_final.push_back(band[i]);
		xun_count++;

	}

	for (int i = 0; i<xun_count; i++)//xun_count表示path_final中有多少条边
	{
		for (int k = 0; k<count; k++)
		{
			if (path_final[i][0] == consumer[k].end)
			{
				path_final[i].push_back(consumer[k].start);
			}
		}
	}

	string mystr;
	my_itoa(xun_count, mystr);
	out += mystr;
	//add the num of line

	out += '\n';
	out += '\n';
	// add the path
	for (int i = 0; i<xun_count; i++)
	{
		int k = path_final[i].size();
		for (int j = k - 2; j >= 0; j--)
		{
			mystr = "";
			my_itoa(path_final[i][j], mystr);
			out += mystr;
			out += ' ';
		}
		mystr = "";
		my_itoa(path_final[i][k - 1], mystr);
		out += mystr;
		out += ' ';
		// add the flow;
		mystr = "";
		my_itoa(band_final[i], mystr);
		out += mystr;
		if (i != xun_count - 1)
		{
			out += '\n';
		}
	}

	char * topo_file = (char*)out.data();
	
	write_result(topo_file, filename);
	cout << "serverCost " << serverCost;
}

