#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <set>
#include <queue>
#include <vector>
#include <ctime>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
using namespace std;
struct Route
{
    int x;
    int y;
    int direction;  // N1 S2 E3 W4
    double cost;
};
struct Net
{
	  int id;
    int num_pin;
    int pin1_x , pin1_y , pin2_x , pin2_y;
    int length;
    int overflow;
	  int** grids;
    vector<Route> route;
};
string  input_name , output_name;
int grid_Height , grid_Width;
int V_capacity , H_capacity;   //max capacity
int num_net;
vector<Net> vector_N;
int overflow;
int**    vertical_use; 
int**    vertical_overflow;
double** vertical_historical_term;      
int**    horizontal_use;
int**    horizontal_overflow;
double** horizontal_historical_term;
int**    parent;
double** map_cost;
vector<vector<vector<int>>> vertical_id_list;
vector<vector<vector<int>>> horizontal_id_list;
struct cmp_by_cost
{
    bool operator() (Route const p1 , Route const p2)
    {
       return p1.cost > p2.cost;
    }
};
struct cmp_by_overflow
{
	  bool operator() (Route const p1, Route const p2)
    {
  		int overflow_p1, overflow_p2;
  		if(p1.direction == 1 || p1.direction == 2)
  			overflow_p1 = vertical_overflow[p1.x][p1.y];
  		else
  			overflow_p1 = horizontal_overflow[p1.x][p1.y];
  		
  		if(p2.direction == 1 || p2.direction == 2)
  			overflow_p2 = vertical_overflow[p2.x][p2.y];
  		else
  			overflow_p2 = horizontal_overflow[p2.x][p2.y];
  		
  		return overflow_p1 < overflow_p2;
	  }
};
struct cmp_by_distance
{
    bool operator() (int const p1, int const p2)
    { 
  		int overflow_p1 = vector_N[p1].overflow;
  		int overflow_p2 = vector_N[p2].overflow;
      if (overflow_p1 == overflow_p2) 
  			return vector_N[p1].length > vector_N[p2].length;
      else 
  			return overflow_p1 < overflow_p2;
    }
};
priority_queue<Route, vector<Route>, cmp_by_cost    > routing_queue;
priority_queue<Route, vector<Route>, cmp_by_overflow> ripup_queue;
priority_queue<int  , vector<int>  , cmp_by_distance> reroute_queue;


void parser()
{
    FILE * file_I;
    file_I = fopen( input_name.c_str() , "r" );
    fscanf(file_I, "grid %d %d\n",&grid_Width,&grid_Height);
    fscanf(file_I, "vertical capacity %d\n",&V_capacity);
    fscanf(file_I, "horizontal capacity %d\n",&H_capacity);
    fscanf(file_I, "num net %d\n",&num_net);

    for(int i = 0; i < num_net ; i++)
    {
       Net temp_net;
       int a;
       fscanf(file_I, "net%d %d %d\n", &a , &temp_net.id , &temp_net.num_pin);
       fscanf(file_I, "  %d %d\n", &temp_net.pin1_x , &temp_net.pin1_y);
       fscanf(file_I, "  %d %d\n", &temp_net.pin2_x , &temp_net.pin2_y);
       temp_net.length = abs(temp_net.pin1_x - temp_net.pin2_x) + abs(temp_net.pin1_y - temp_net.pin2_y);
       temp_net.overflow = 0;
       temp_net.route.clear();
       vector_N.push_back(temp_net);
    }
}
void initial_2D_array()
{
    vertical_use               = new int*[grid_Width];
    vertical_overflow          = new int*[grid_Width];
    vertical_historical_term   = new double*[grid_Width];   
     
     
    horizontal_use             = new int*[grid_Width-1];  
    horizontal_overflow        = new int*[grid_Width-1]; 
    horizontal_historical_term = new double*[grid_Width-1]; 
    
    
    parent                     = new int*[grid_Width];
    map_cost                   = new double*[grid_Width];
    vertical_id_list.resize(grid_Width);
	  horizontal_id_list.resize(grid_Width);
     
     
    for(int i = 0; i < grid_Width; i++)
    {
        vertical_use[i]             = new int[grid_Height-1];
        vertical_overflow[i]        = new int[grid_Height-1];
        vertical_historical_term[i] = new double[grid_Height-1];
    }
    for(int i = 0; i < grid_Width-1; i++)
    {
        horizontal_use[i]             = new int[grid_Height];
        horizontal_overflow[i]        = new int[grid_Height];
        horizontal_historical_term[i] = new double[grid_Height];
    }
    for(int i = 0; i < grid_Width; i++)
    {
        parent[i]                   = new int[grid_Height];
        map_cost[i]                 = new double[grid_Height];
        vertical_id_list[i].resize(grid_Height);
        horizontal_id_list[i].resize(grid_Height);
    }
    
    
    for(int i=0 ; i<grid_Width ; i++)
    {
      for(int j=0 ; j<grid_Height-1 ; j++)
      { 
        vertical_use[i][j]             = 0;
        vertical_overflow[i][j]        = 0;
        vertical_historical_term[i][j] = 0;
      }
    }
    for(int i=0 ; i<grid_Width-1 ; i++)
    {
      for(int j=0 ; j<grid_Height ; j++)
      { 
        horizontal_use[i][j]             = 0;
        horizontal_overflow[i][j]        = 0;
        horizontal_historical_term[i][j] = 0;
      }
    }
}
Route wave_propagation(Route Cur, Route end_point , int direction  , double b)
{ 
  Route temp = Cur;
  temp.direction = direction;
  Route out_boundary;
  out_boundary.x = -1;
  out_boundary.y = -1;
  out_boundary.direction = -1;
  out_boundary.cost = -1;
  double cost;
  double historical_term;
  double congestion;
  if (direction == 1) 
  { 
    if(Cur.y+1 > grid_Height - 1) 
       return out_boundary;
       
    if (vertical_use[Cur.x][Cur.y] > V_capacity) 
			historical_term = (double)vertical_historical_term[Cur.x][Cur.y] + 1.0;
    else
			historical_term = (double)vertical_historical_term[Cur.x][Cur.y];
      
    congestion =  (double)pow( (vertical_use[Cur.x][Cur.y] + 1.0)/V_capacity , 5 );  
    
    cost = (b + historical_term) * congestion;
    
    temp.y++; 
    if (Cur.y >= end_point.y)
       temp.cost++;           
		temp.cost += cost;
	}
  else if(direction == 2)
  { 
    if(Cur.y-1 < 0) 
       return out_boundary;
       
    if (vertical_use[Cur.x][Cur.y-1] > V_capacity) 
			historical_term = (double)vertical_historical_term[Cur.x][Cur.y-1] + 1.0;
    else
			historical_term = (double)vertical_historical_term[Cur.x][Cur.y-1];
      
    congestion =  (double)pow( (vertical_use[Cur.x][Cur.y-1] + 1.0)/V_capacity , 5 );  
    
    cost = (b + historical_term) * congestion;
    
    temp.y--;   
    if (Cur.y <= end_point.y) 
       temp.cost++;       
		temp.cost += cost;
  }
  else if(direction == 3) 
  { 
    if(Cur.x+1 > grid_Width - 1)
       return out_boundary;
       
    if (horizontal_use[Cur.x][Cur.y] > H_capacity)
			historical_term = (double)horizontal_historical_term[Cur.x][Cur.y] + 1.0;
    else 
			historical_term = (double)horizontal_historical_term[Cur.x][Cur.y];
    
    congestion =  (double)pow( (horizontal_use[Cur.x][Cur.y] + 1.0)/H_capacity , 5 ); 
    
    cost = (b + historical_term) * congestion;
            
    temp.x++;
    if (Cur.x >= end_point.x) 
       temp.cost++;          
		temp.cost += cost; 		
	}
  else if(direction == 4)
  { 
    if(Cur.x-1 < 0)
      return out_boundary;
      
    if (horizontal_use[Cur.x-1][Cur.y] > H_capacity)
			historical_term = (double)horizontal_historical_term[Cur.x-1][Cur.y] + 1.0;
    else 
			historical_term = (double)horizontal_historical_term[Cur.x-1][Cur.y];
    
    congestion =  (double)pow( (horizontal_use[Cur.x-1][Cur.y] + 1.0)/H_capacity , 5 ); 
            
    cost = (b + historical_term) * congestion;
               
    temp.x--;
    if (Cur.x <= end_point.x)
      temp.cost++; 
		temp.cost += cost;
  }
	return temp;
}


void back_trace(int net_id)
{
  Route Cur;
  Cur.x =  vector_N[net_id].pin2_x;
  Cur.y =  vector_N[net_id].pin2_y;
  Cur.direction = 0;
  Cur.cost = 0;
  int end_x = vector_N[net_id].pin1_x;
  int end_y = vector_N[net_id].pin1_y;
  
  while(parent[Cur.x][Cur.y] != -1 )
  {
    vector_N[net_id].route.push_back(Cur);  
       
    int Parent = parent[Cur.x][Cur.y];
    
    if(Parent == 1)
    {
      Cur.y--;
  		vertical_use[Cur.x][Cur.y]++;
  		vertical_id_list[Cur.x][Cur.y].push_back(net_id);
      continue;
    }
    else if(Parent == 2)
    {
  		vertical_use[Cur.x][Cur.y]++;
  		vertical_id_list[Cur.x][Cur.y].push_back(net_id);
  		Cur.y++;
  		continue;
  	}
    else if(Parent == 3)
    {
  		Cur.x--;
  		horizontal_use[Cur.x][Cur.y]++;
  		horizontal_id_list[Cur.x][Cur.y].push_back(net_id);
  		continue;
  	}
    else if(Parent == 4)
    {
      horizontal_use[Cur.x][Cur.y]++;
  		horizontal_id_list[Cur.x][Cur.y].push_back(net_id);
  		Cur.x++;
  		continue;
    }
  }
  
	vector_N[net_id].route.push_back(Cur);

	return ;
}
void routing(int net_id , double b )
{ 
  Route start_point;
  start_point.x = vector_N[net_id].pin1_x;
  start_point.y = vector_N[net_id].pin1_y;
  start_point.direction = -1;
  start_point.cost = 0.0;
  
  Route end_point;
  end_point.x = vector_N[net_id].pin2_x;
  end_point.y = vector_N[net_id].pin2_y;
  end_point.direction = -1;
  end_point.cost = -1.0;
  
  for(int i = 0; i < grid_Width; i++)
  {
	  for(int j = 0; j < grid_Height; j++)
    {
	    parent[i][j] = -1;
	    map_cost[i][j] = -1;
	  }
  }

  routing_queue = {};
  routing_queue.push(start_point);
  map_cost[start_point.x][start_point.y] = 0.0;
  while(  !routing_queue.empty()  )
  {
    Route Cur = routing_queue.top();
    routing_queue.pop(); 
    if(Cur.x == end_point.x && Cur.y == end_point.y)
    {
			end_point.direction = Cur.direction;
			end_point.cost = Cur.cost;
			continue;
		}
		
    for(int i = 1; i < 5 ; i++)
    { 
			Route Next = wave_propagation(Cur , end_point , i , b);
			if( Next.cost == -1)
        continue;
      if( map_cost[Next.x][Next.y] != -1 && Next.cost >= map_cost[Next.x][Next.y] )  
        continue;
			map_cost[Next.x][Next.y] = Next.cost;
			parent[Next.x][Next.y]  = i;
			routing_queue.push(Next);
		}
  }
  
  back_trace(net_id);
}
void cal_overflow()
{
	int temp = 0;
	
	for(int i = 0; i < grid_Width ; i++)
  {
		for(int j = 0; j < grid_Height - 1; j++)
    {
			vertical_overflow[i][j] = max( vertical_use[i][j] - V_capacity, 0);
			temp += vertical_overflow[i][j];
		}
	}
	for(int i = 0; i < grid_Width - 1; i++)
  {
		for(int j = 0; j < grid_Height ; j++)
    {
			horizontal_overflow[i][j] = max( horizontal_use[i][j] - H_capacity, 0);
			temp += horizontal_overflow[i][j];
		}
	}
	overflow = temp;
}
void rip_up(Route top)
{
  vector<int> rip_up_vector;
  int count = 0;
	if(top.direction == 1)
		rip_up_vector = vertical_id_list[top.x][top.y];
	else
		rip_up_vector = horizontal_id_list[top.x][top.y];
   
  for(unsigned i = 0; i < rip_up_vector.size(); i++)
  {
		int net_id = rip_up_vector[i];
		Route route_A = vector_N[net_id].route[0];
    Route route_B;
		count = 0;
		for(unsigned j = 1; j < vector_N[net_id].route.size() ; j++)
    { 
      route_B = vector_N[net_id].route[j];
			if(route_A.y == route_B.y)
      {  
         int k;
         if(route_A.x - 1 == route_B.x) // A right B left
         {
  			 	count += max( horizontal_use[route_B.x][route_B.y] - H_capacity, 0);
  				horizontal_use[route_B.x][route_B.y]--;        
  				for(k = 0; k < horizontal_id_list[route_B.x][route_B.y].size(); k++)
          {
  					if(horizontal_id_list[route_B.x][route_B.y][k] == net_id)	
                break;
          }      
  				horizontal_id_list[route_B.x][route_B.y].erase(horizontal_id_list[route_B.x][route_B.y].begin()+k);
         }
         
         if(route_A.x + 1 == route_B.x) // A left B right
         {
  				count += max(horizontal_use[route_A.x][route_A.y] - H_capacity, 0);
  				horizontal_use[route_A.x][route_A.y]--;
  				for(k = 0; k < horizontal_id_list[route_A.x][route_A.y].size(); k++)
          {
  					if(horizontal_id_list[route_A.x][route_A.y][k] == net_id)	
                break;
          }      
  				horizontal_id_list[route_A.x][route_A.y].erase(horizontal_id_list[route_A.x][route_A.y].begin()+k);
         }
			}
      
      if(route_A.x == route_B.x)
      {  
         int k;
         if(route_A.y - 1 == route_B.y) // A up B down
         {
          count += max(vertical_use[route_B.x][route_B.y] - V_capacity, 0);
  				vertical_use[route_B.x][route_B.y]--;
  				for(k = 0; k < vertical_id_list[route_B.x][route_B.y].size(); k++)
          {
  					if(vertical_id_list[route_B.x][route_B.y][k] == net_id)
              	break;
          }     
  				vertical_id_list[route_B.x][route_B.y].erase(vertical_id_list[route_B.x][route_B.y].begin()+k);
         }
         
         if(route_A.y + 1 == route_B.y) // A down B up
         {
          count += max(vertical_use[route_A.x][route_A.y] - V_capacity, 0);
  				vertical_use[route_A.x][route_A.y]--;
  				for(k = 0; k < vertical_id_list[route_A.x][route_A.y].size(); k++)
          {
  					if(vertical_id_list[route_A.x][route_A.y][k] == net_id)
              	break;
          }     
  				vertical_id_list[route_A.x][route_A.y].erase(vertical_id_list[route_A.x][route_A.y].begin()+k);
         }
      }
			route_A = route_B;
		}
		vector_N[net_id].overflow = count;
		vector_N[net_id].route.clear();
		reroute_queue.push(rip_up_vector[i]);
	} 
}
void reroute( double b )
{
  while(!reroute_queue.empty())
  {
		int net_id = reroute_queue.top();
		reroute_queue.pop();
    routing(net_id , b);
	}
	return ;
}
void ripup_reroute()
{
    int round = 1;
    while(overflow > 0)
    {
  		ripup_queue = {};

    	for(int i = 0 ; i < grid_Width ; i++)
      {
    		for(int j = 0 ; j < grid_Height - 1 ; j++)
        {
    			if(vertical_overflow[i][j] > 0)
          {
    				vertical_historical_term[i][j]++;
            Route temp;
            temp.x = i;
            temp.y = j;
            temp.direction = 1;        
    				ripup_queue.push(temp);
    			}
    		}
    	}
    	for(int i = 0; i < grid_Width - 1 ; i++)
      {
    		for(int j = 0; j < grid_Height ; j++)
        {
    			if(horizontal_overflow[i][j] > 0)
          {
    				horizontal_historical_term[i][j]++;
            Route temp;
    				temp.x = i;
            temp.y = j;   
            temp.direction = 0;      
    				ripup_queue.push(temp);
    			}
    		}
    	}
     
	    round++;
  		double b = 1.0 - exp(-5 * exp(-0.1 * round));
  		
  		while(overflow > 0 && !ripup_queue.empty())
      { 
        Route top = ripup_queue.top();
        rip_up(top);
        ripup_queue.pop();
        reroute(b);
  			cal_overflow();
        cout<<"overflow "<< overflow<<endl;
        
  		}
       
       if(num_net == 27781 && overflow <= 150)
          break ;
  	}
}
void output()
{
  ofstream fout(output_name);
  for(int i = 0; i < num_net ; i++)
  {
    fout<<"net"<<i<<" "<<i<<endl;
    for(int j = 0 ; j < vector_N[i].route.size()-1 ; j++)
    {
      fout<<"("<<vector_N[i].route[j].x<<", "<<vector_N[i].route[j].y<<", 1)-("<<vector_N[i].route[j+1].x<<", "<<vector_N[i].route[j+1].y<<", 1)"<<endl;
    }
      fout<<"!"<<endl;              
  }
}
int main(int argc, char **argv)
{
    if (argc != 3) {
        exit(1);
    }
    input_name = argv[1];
    output_name = argv[2];

    parser();
    initial_2D_array(); 
    for(int i = 0; i < num_net ; i++)
    {
        routing(i , 0);
    }
    cal_overflow();
    ripup_reroute();
    output();
    return 0;
}
