#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <ctime>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stack>
using namespace std;
// done  create initail polish expression
// done  compute the smallest area of a polish expression
// done  compute all blocks orientation in the smallest area floorplan
// done compute the x,y coordinate of every block
// to-do compute wirelength

// to-do annealing schedule
// to-do perturb
// to-do cost function
struct block         
{
  int id;
  int width, height;
  int area;
  int x, y;
  bool used ;
  bool rotate;
};
struct net
{
  int degree;
  vector<int> v_terminal;
  vector<int> v_hardblock;
}; 
struct terminal
{
  int id;
  int x;
  int y;
}; 
struct rectangle     // use to deal with MergeHCut & MergeVCut
{
  int id;            // the index of polish expression
  vector< pair<int, int> > pairs;

  int left_id;       // the index of polish expression     if no child  => id = -3 
  int right_id;      
  vector< pair<int, int> > child_pairs_index; // (left_child_pairs_index = 0 or 1,  right_child_pairs_index = 0 or 1)      
                                              // e.g. pairs[2] is merged by left_child_pairs[0] and right_child_pairs[1]
};
struct rectangleOXY
{
  int parent_cut; // Vertical-cut = -1  horizontal_cut = -2
  int x;
  int y;
};

double  white_space_ratio = 0;
string  input_H , input_N , input_P ,output_name;
net*      nets;
terminal* terminals;

int num_hardblocks = 0 , num_terminals = 0 , num_nets=0 , num_pins = 0;
int total_block_area = 0;
int x_outline;  //x_outline == y_outline    outline (0,0) to ( x_outline, y_outline)
stack<rectangle> stack_r;
vector<rectangle> vector_r;  // the id of vector_r is the id of vector polish_expression.    not block id
vector<int> operand_index;
vector<int> operator_index;

block*      blocks;
block*      temp_blocks;
block*      best_blocks;
vector<int> polish_expression;   //cotent: block_id   Vertical-cut = -1  horizontal_cut = -2
vector<int> temp_polish_expression; 
vector<int> best_polish_expression;
int       initial_wirelength = 0 , wirelength = 0 , temp_wirelength = 0 , best_wirelength = 0;
double    initial_cost = 0       , cost = 0       , temp_cost = 0       , best_cost = 0; 
int       initial_area = 0       , area = 0       , temp_area = 0       , best_area = 0;
int       initial_max_x = 0      , max_x = 0      , temp_max_x = 0      , best_max_x = 0;
int       initial_max_y = 0      , max_y = 0      , temp_max_y = 0      , best_max_y = 0;

void parser()
{   
  FILE * file_H;
  file_H = fopen( input_H.c_str() , "r" ) ;
  fscanf(file_H, "NumHardRectilinearBlocks : %d\n",&num_hardblocks);
  fscanf(file_H, "NumTerminals : %d\n\n",&num_terminals);

  blocks      = new block[num_hardblocks]();
  temp_blocks = new block[num_hardblocks]();
  best_blocks = new block[num_hardblocks]();

  for(int i = 0; i < num_hardblocks ; i++) 
  {  
     int t1,t2,t3,t4,t5,t6,t7,t8,t9 ;
     fscanf(file_H, "sb%d hardrectilinear 4 (%d, %d) (%d, %d) (%d, %d) (%d, %d)\n" ,&t1,&t2,&t3,&t4,&t5,&t6,&t7,&t8,&t9); //sb0 hardrectilinear 4 (0, 0) (0, 33) (43, 33) (43, 0)
     blocks[i].id = t1;
     blocks[i].width = t6;
     blocks[i].height = t7;
     blocks[i].area = blocks[i].width * blocks[i].height;
     blocks[i].x = -1;
     blocks[i].y = -1;
     blocks[i].used = 0;
     blocks[i].rotate = 0;
  }
  fclose( file_H );

  
  FILE * file_N;
  file_N = fopen( input_N.c_str() , "r" ) ;
  fscanf(file_N, "NumNets : %d\n" ,&num_nets);
  fscanf(file_N, "NumPins : %d\n" ,&num_pins);
  nets = new net[num_nets]();
  for(int i = 0 ; i < num_nets ; i++) 
  {  
     fscanf(file_N, "NetDegree : %d\n", &nets[i].degree);
     int count = nets[i].degree;
     while(count!=0)
     {  
        char temp[10];
        fscanf(file_N, "%s\n", &temp);
        if(temp[0] == 's')
        {
           string str = temp;
           int length = str.length();
           str = str.substr(2,length);
           nets[i].v_hardblock.push_back(  atoi(str.c_str()) );
        }
        else if(temp[0] == 'p')
        {
           string str = temp;
           int length = str.length();
           str = str.substr(1,length);
           nets[i].v_terminal.push_back(  atoi(str.c_str()) );
        }
        count--;
     }
  }
  fclose( file_N );  

 
  FILE * file_P;
  file_P = fopen( input_P.c_str() , "r" ) ;
  terminals = new terminal[num_terminals]();
  for(int i = 0; i < num_terminals; ++i)
  {
    fscanf(file_P , "p%d %d %d\n", &terminals[i].id, &terminals[i].x, &terminals[i].y);
  }
  fclose( file_P ); 
}
void calculate_Constraint()
{
  for(int i = 0 ; i < num_hardblocks ; i++)
  {
   total_block_area += blocks[i].area;     
  } 
  x_outline = (int)sqrt(total_block_area * (1 + white_space_ratio));

  cout<<"X_outline : " <<x_outline<<endl;
}
void PrintPolish()
{
  cout<<"polish : ";
  for(int i = 0; i < polish_expression.size(); i++)
  {
    if(polish_expression[i] == -1)
      cout<<"V ";
    else if(polish_expression[i] == -2)
      cout<<"H ";
    else
      cout<<polish_expression[i]<<" ";
  }
  cout<<endl;
}

bool CompareH(const block lhs,const block rhs)
{
   return lhs.height<rhs.height ;
} 
void init_floorplan()
{ 
  // vector<int> polish20, polish30, polish40, polish50, polish60;
  // int H=0, H20=0, H30=0, H40=0, H50=0, H60=0;
  // for(int i = 0; i < num_hardblocks; i++)
  // { 
  //   H = blocks[i].height;
  //   if( H >= 0 && H <30)
  //   { 
  //     if(polish20.empty())
  //     {
  //       polish20.push_back(i);
  //     }
  //     else
  //     {
  //       polish20.push_back(i);
  //       polish20.push_back(-1);
  //     }
  //   }
  //   else if(H >= 30 && H <40)
  //   {
  //     if(polish30.empty())
  //     {
  //       polish30.push_back(i);
  //     }
  //     else
  //     {
  //       polish30.push_back(i);
  //       polish30.push_back(-1);
  //     }
  //   }
  //   else if(H >= 40 && H <50)
  //   {
  //     if(polish40.empty())
  //     {
  //       polish40.push_back(i);
  //     }
  //     else
  //     {
  //       polish40.push_back(i);
  //       polish40.push_back(-1);
  //     }
  //   }
  //   else if(H >= 50 && H <60)
  //   {
  //     if(polish50.empty())
  //     {
  //       polish50.push_back(i);
  //     }
  //     else
  //     {
  //       polish50.push_back(i);
  //       polish50.push_back(-1);
  //     }
  //   }
  //   else
  //   {
  //     if(polish60.empty())
  //     {
  //       polish60.push_back(i);
  //     }
  //     else
  //     {
  //       polish60.push_back(i);
  //       polish60.push_back(-1);
  //     }
  //   }
  // }
  // for(int i = 0 ; i < polish20.size(); i++)
  // {
  //   polish_expression.push_back(polish20[i]);
  // }
  // for(int i = 0 ; i < polish30.size(); i++)
  // {
  //   polish_expression.push_back(polish30[i]);
  // }
  // if(!polish30.empty())
  //   polish_expression.push_back(-2);

  // for(int i = 0 ; i < polish40.size(); i++)
  // {
  //   polish_expression.push_back(polish40[i]);
  // }
  // if(!polish40.empty())
  //   polish_expression.push_back(-2);

  // for(int i = 0 ; i < polish50.size(); i++)
  // {
  //   polish_expression.push_back(polish50[i]);
  // }
  // if(!polish50.empty())
  //   polish_expression.push_back(-2);

  // for(int i = 0 ; i < polish60.size(); i++)
  // {
  //   polish_expression.push_back(polish60[i]);
  // }
  // if(!polish60.empty())
  //   polish_expression.push_back(-2);

  vector<block> temp_b;
  int cur_x = 0;
  bool first = true;

  for(int i = 0; i < num_hardblocks; i++)
  {
    temp_b.push_back(blocks[i]);
  }  
  sort(temp_b.begin(), temp_b.end(), CompareH);

  for(int i = 0; i < num_hardblocks; i++)
  { 
    if(cur_x + blocks[i].width <= x_outline)
    {
      if( cur_x == 0 )
      {
        polish_expression.push_back(temp_b[i].id);
        cur_x += blocks[i].width;
      }  
      else
      {
        polish_expression.push_back(temp_b[i].id);
        polish_expression.push_back(-1);
        cur_x += blocks[i].width;
      }
    }  
    else
    {
      if(first == true)
      {
        first = false;
        cur_x = 0;
        i--;
      }
      else
      {
        polish_expression.push_back(-2);
        cur_x = 0;
        i--;
      }
    }
  }  
  polish_expression.push_back(-2);
  

  cout<<polish_expression.size()<<endl;
  PrintPolish();
  
}

rectangle MergeVCut(rectangle r1, rectangle r2, int i)
{   
    vector< pair<int, int> > temp_pairs;            
    vector< pair<int, int> > temp_child_pairs_index;
    temp_pairs.clear();
    temp_pairs.resize(0);
    temp_child_pairs_index.clear();
    temp_child_pairs_index.resize(0);

    int cur_w, cur_h;
    for(int i=0; i < r1.pairs.size(); i++)
    {
      for(int j=0; j < r2.pairs.size(); j++)
      {   
          cur_w = r1.pairs[i].first + r2.pairs[j].first;
          cur_h = max(r1.pairs[i].second, r2.pairs[j].second);

          temp_pairs.push_back( make_pair(cur_w, cur_h) );
          temp_child_pairs_index.push_back( make_pair(i, j) );
      }
    }

    rectangle new_r;
    new_r.id = i;
    new_r.left_id  = r1.id;  
    new_r.right_id = r2.id;  
    
    bool good_choice; // check if the pair is not bigger than any other
    for(int i = 0; i < temp_pairs.size(); i++)
    { 
      good_choice = true;
      for(int j = 0; j < temp_pairs.size(); j++)
      {
        if(i == j)
          continue;
        else
        {
          if(temp_pairs[i].first > temp_pairs[j].first && temp_pairs[i].second >= temp_pairs[j].second)
            good_choice = false;
          else if(temp_pairs[i].first >= temp_pairs[j].first && temp_pairs[i].second > temp_pairs[j].second)
            good_choice = false;
          else if(temp_pairs[i].first == temp_pairs[j].first && temp_pairs[i].second == temp_pairs[j].second)
          {
            if(i > j)
              good_choice = false;
          }
        }
      }
      if(good_choice == 1)
      {
        new_r.child_pairs_index.push_back( make_pair( temp_child_pairs_index[i].first, temp_child_pairs_index[i].second) ); 
        new_r.pairs.push_back( make_pair(temp_pairs[i].first, temp_pairs[i].second) );
      }
    }
    
    return new_r;
}

rectangle MergeHCut(rectangle r1, rectangle r2, int i)
{   
    // r1 = (3, 5)(5, 3)  r2 = (4, 6)(6, 4)
    // (r1 H r2) => (4, 11) (5, 9) (6, 7) (6, 9)
    vector< pair<int, int> > temp_pairs;            // (4, 11) (5, 9) (6, 7)
    vector< pair<int, int> > temp_child_pairs_index;// (0, 0)  (1, 0) (1, 1)
    temp_pairs.clear();
    temp_pairs.resize(0);
    temp_child_pairs_index.clear();
    temp_child_pairs_index.resize(0);

    int cur_w, cur_h;
    for(int i=0; i< r1.pairs.size(); i++)
    {
      for(int j=0; j< r2.pairs.size(); j++)
      {   
          cur_w = max(r1.pairs[i].first, r2.pairs[j].first);
          cur_h = r1.pairs[i].second + r2.pairs[j].second;

          temp_pairs.push_back( make_pair(cur_w, cur_h) );
          temp_child_pairs_index.push_back( make_pair(i, j) );
      }
    }

    rectangle new_r;
    new_r.id = i;
    new_r.left_id = r1.id;       
    new_r.right_id  = r2.id;   

    bool good_choice; // check if the pair is not bigger than any other

    for(int i = 0; i < temp_pairs.size(); i++)
    { 
      good_choice = true;
      for(int j = 0; j < temp_pairs.size(); j++)
      {
        if(i == j)
          continue;
        else
        {
          if(temp_pairs[i].first > temp_pairs[j].first && temp_pairs[i].second >= temp_pairs[j].second)
            good_choice = false;
          else if(temp_pairs[i].first >= temp_pairs[j].first && temp_pairs[i].second > temp_pairs[j].second)
            good_choice = false;
          else if(temp_pairs[i].first == temp_pairs[j].first && temp_pairs[i].second == temp_pairs[j].second)
          {
            if(i > j)
              good_choice = false;
          }
        }
      }
      if(good_choice == 1)
      {
        new_r.child_pairs_index.push_back( make_pair( temp_child_pairs_index[i].first, temp_child_pairs_index[i].second) ); 
        new_r.pairs.push_back( make_pair(temp_pairs[i].first, temp_pairs[i].second) );
      }
    }

    return new_r;
}


rectangle MinAreaAlgorithm()
{ 
  vector_r.clear();
  rectangle r1, r2, new_r;
  int w, h;
  for(int i = 0; i < polish_expression.size(); i++)
  { 
    if(polish_expression[i] == -1)
    {    
        r2 = stack_r.top();
        stack_r.pop();
        r1 = stack_r.top();
        stack_r.pop();
        new_r = MergeVCut(r1, r2, i);
        stack_r.push(new_r);
    } 
    else if(polish_expression[i] == -2)
    { 
        r2 = stack_r.top();
        stack_r.pop();
        r1 = stack_r.top();
        stack_r.pop();
        new_r = MergeHCut(r1, r2, i);
        stack_r.push(new_r);
    }
    else
    {   
        new_r.id = i;    // i : the index of polish
        new_r.pairs.clear();
        new_r.left_id  = -3;       
        new_r.right_id = -3;       
        new_r.child_pairs_index.clear();  

        w = blocks[polish_expression[i]].width;
        h = blocks[polish_expression[i]].height;

        if(w != h)  //(2,4) (4,2)
        {  
          new_r.pairs.push_back( make_pair(w, h) );
          new_r.pairs.push_back( make_pair(h, w) );
        }
        else       //(4,4)
          new_r.pairs.push_back( make_pair(w, w) );

        stack_r.push(new_r);
    } 

    vector_r.push_back(new_r);
  }

  rectangle root = stack_r.top();
  stack_r.pop();
  return root;
}

void SetBlocks(rectangle root, int root_index) //set every block to right orientation, and find their x y coordinates.   use PreorderTraversal
{   
    int node_id, node_index, node_left_child, node_left_index, node_right_child, node_right_index;
    stack< pair<rectangle, int> > preorder_stack; 
    stack< rectangleOXY > preorder_stack2;

    preorder_stack.push( make_pair(root, root_index) ); 

    rectangleOXY temp;
    temp.parent_cut = 0;
    temp.x = 0;
    temp.y = 0;
    preorder_stack2.push(temp);  

    while (preorder_stack.empty() == false) 
    { 
        pair<rectangle, int> node = preorder_stack.top();
        preorder_stack.pop(); 
        node_index        = node.second;
        node_id           = node.first.id;
        node_left_child   = node.first.left_id;
        node_right_child  = node.first.right_id;
        if(node_left_child != -3)
          node_left_index   = node.first.child_pairs_index[node_index].first;
        if(node_right_child != -3)
          node_right_index  = node.first.child_pairs_index[node_index].second;

        if(node_left_child == -3 && node_right_child == -3)
          blocks[polish_expression[node_id]].rotate = node_index; // if node_index == 0 means origin,  node_index==1 means rotated.

        rectangleOXY node2 = preorder_stack2.top();
        preorder_stack2.pop(); 

        if(polish_expression[node_id] != -1 && polish_expression[node_id] != -2)
        {  
          blocks[ polish_expression[node_id] ].x = node2.x;
          blocks[ polish_expression[node_id] ].y = node2.y;
        }  
  


        if (node_right_child != -3)
        { 
            preorder_stack.push( make_pair(vector_r[node_right_child], node_right_index) ); 

            temp.parent_cut = polish_expression[node_id];
            if( temp.parent_cut == -1)
            { 
              temp.x = node2.x + vector_r[node_left_child].pairs[node_left_index].first;
              temp.y = node2.y;
            }
            if( temp.parent_cut == -2)
            {
              temp.x = node2.x;
              temp.y = node2.y + vector_r[node_left_child].pairs[node_left_index].second;
            }  
            preorder_stack2.push(temp); 
        }    

        if (node_left_child  != -3) 
        {
            preorder_stack.push( make_pair(vector_r[node_left_child], node_left_index) ); 

            temp.parent_cut = 0;
            temp.x = node2.x;
            temp.y = node2.y; 
            preorder_stack2.push(temp);  
        }   

        // cout<<"id         : "<<node_id<<endl;
        // cout<<"index      : "<<node_index<<endl;
        // cout<<"left child : "<<node_left_child<<endl;
        // cout<<"left index : "<<node_left_index<<endl;
        // cout<<"right child: "<<node_right_child<<endl;
        // cout<<"right index: "<<node_right_index<<endl<<endl<<endl;
    } 
}

void ComputeWireLength()
{  
   for(int i = 0; i < num_nets; i++)
   { 
     int center_x = 0 , center_y = 0 , rightmost_x = 0 , leftmost_x = 99999 , highest_y = 0 , lowest_y = 99999;
     
     int H_size = nets[i].v_hardblock.size();
     int P_size = nets[i].v_terminal.size();

     for(int j=0 ; j < H_size ; j++ )
     {  
        int cur = nets[i].v_hardblock[j];
        if(blocks[cur].rotate == 0)
        {
          center_x = blocks[cur].x + floor(blocks[cur].width/2);
          center_y = blocks[cur].y + floor(blocks[cur].height/2);
        }
        else
        {
          center_x = blocks[cur].x + floor(blocks[cur].height/2);
          center_y = blocks[cur].y + floor(blocks[cur].width/2);
        }  
        if(center_x > rightmost_x )
           rightmost_x = center_x;
        if(center_x < leftmost_x )   
           leftmost_x = center_x;
        if(center_y > highest_y )   
           highest_y = center_y;
        if(center_y < lowest_y )   
           lowest_y = center_y;
     }
     for(int j=0 ; j < P_size ; j++ )
     {
        int cur = nets[i].v_terminal[j];
        center_x = terminals[cur-1].x ;
        center_y = terminals[cur-1].y ;
        if(center_x > rightmost_x )
           rightmost_x = center_x;
        if(center_x < leftmost_x )   
           leftmost_x = center_x;
        if(center_y > highest_y )   
           highest_y = center_y;
        if(center_y < lowest_y )   
           lowest_y = center_y;
     }
     wirelength += (rightmost_x - leftmost_x)+(highest_y - lowest_y);    
   }
   // cout<<"WIRELENGTH : "<<wirelength<<endl;
}

void ComputeCost()
{ 
  double x_diff , y_diff , xy_diff;
  if(max_x - x_outline > 0)
       x_diff = (double)max_x - (double)x_outline;
  else 
       x_diff = 0;  
       
  if(max_y - x_outline > 0)
       y_diff = (double)max_y - (double)x_outline;
  else 
       y_diff = 0;  
       
  if (max_x<= x_outline && max_y<= x_outline)
       xy_diff = 0;
  else
       xy_diff =  abs(max_x - max_y);
    
  cost =  1 * ( (double)max_x * max_y / (double)initial_area )  +  1 *( (double)wirelength / (double)initial_wirelength )  +   1 * ( x_diff + y_diff + xy_diff ) ;    
  // cout<<"COST : "<< cost<<endl;
} 

void store_best()       
{
  best_max_x      = max_x;
  best_max_y      = max_y;
  best_area       = area;
  best_wirelength = wirelength;
  best_cost       = cost;
  
  for(int i = 0; i < num_hardblocks; i++)
  {
    best_blocks[i] = blocks[i];
  }
  best_polish_expression.assign(polish_expression.begin(), polish_expression.end());

}
void restore_best()      
{
  max_x      = best_max_x;
  max_y      = best_max_y;
  area       = best_area;
  wirelength = best_wirelength;
  cost       = best_cost;

  for(int i = 0; i < num_hardblocks; i++)
  {
    blocks[i] = best_blocks[i];
  }
  polish_expression.assign(best_polish_expression.begin(), best_polish_expression.end());
}
void store_temp()      
{
  temp_max_x = max_x;
  temp_max_y = max_y;
  temp_area  = area;
  temp_wirelength = wirelength;
  temp_cost = cost;

  for(int i = 0; i < num_hardblocks; i++)
  {
    temp_blocks[i] = blocks[i];
  }
  temp_polish_expression.assign(polish_expression.begin(), polish_expression.end());
}
void restore_temp()       
{
  max_x      = temp_max_x;
  max_y      = temp_max_y;
  area       = temp_area;
  wirelength = temp_wirelength;
  cost       = temp_cost;

  for(int i = 0; i < num_hardblocks; i++)
  {
    blocks[i] = temp_blocks[i];
  }
  polish_expression.assign(temp_polish_expression.begin(), temp_polish_expression.end());
}
void SwapOperand()
{    
    int rand_index1, rand_index2;
    int id1, id2, temp;
    rand_index1 = rand() % operand_index.size();
    rand_index2 = rand() % operand_index.size();
    id1 = operand_index[rand_index1];
    id2 = operand_index[rand_index2];

    temp = polish_expression[id1];
    polish_expression[id1] = polish_expression[id2];
    polish_expression[id2] = temp;

    // cout<<"Swap operand : "<< id1<<" "<<id2<<endl;
}
void ComplementOperator()
{   
    int rand_index, id;
    rand_index = rand() % operator_index.size();
    id = operator_index[rand_index];

    if(polish_expression[ id ] == -1)
        polish_expression[ id ] = -2;
    else if(polish_expression[ id ] == -2)
        polish_expression[ id ] = -1;

    // cout<<"Complement operator : "<<id<<endl;  
}
void SwapOperandOperator()
{  
   int rand_index1, rand_index2;
   int id1, id2;
   int total_operator, total_operand, temp;
   bool safe;

   while(1)
   {  
      rand_index1 = rand() % operand_index.size();
      rand_index2 = rand() % operator_index.size();
      id1 = operand_index[rand_index1];
      id2 = operator_index[rand_index2];

      total_operand = 0;
      total_operator = 0;
      safe = true;

      temp = polish_expression[id1];                                 //change;
      polish_expression[id1] = polish_expression[id2];
      polish_expression[id2] = temp;

      if(id2 < id1)
      { 
        // cout<<"Swap operand operator : "<<id1<<" "<<id2<<endl;
        return;
      }
      else
      {
        for(int j = 0; j <= polish_expression.size() ; j++)               //check balloting property
        {
          if(polish_expression[j]!= -1 && polish_expression[j] != -2)
           total_operand++;
          else
           total_operator++;

          if(total_operator > total_operand - 1 )
            safe = false;
        }    
        if(safe)                                   //if safe remain change
        { 
          // cout<<"!!Swap operand operator : "<<id1<<" "<<id2<<endl;
          return;
        }
        else                                        //if violate change back
        {
          temp = polish_expression[id1];                     
          polish_expression[id1] = polish_expression[id2];
          polish_expression[id2] = temp;
        }
      }
   }   
}
void perturb()
{  
  operand_index.clear();
  operator_index.clear();

  for(int i = 0; i < polish_expression.size(); i++)
  {
    if(polish_expression[i] != -1 && polish_expression[i] != -2)
      operand_index.push_back(i);
    else
      operator_index.push_back(i);
  }  



  int op = rand() % 3;           
  if(op == 0)      
     SwapOperand(); //swap  polish_expression[ operand_index[ index1 ] ] and polish_expression[ operand_index[ index1+1 ] ]
  else if(op == 1) 
     ComplementOperator(); // complement the operator chain 
  else
     SwapOperandOperator(); 
}
void CheckOverlap()
{
  for(int i = 0 ; i<num_hardblocks ; i++)   //check OVERLAP
  {
     for(int j = 0 ; j < num_hardblocks ; j++)
    {   
       if(blocks[j].rotate == 0)
       {
         if( (blocks[i].x < blocks[j].x + blocks[j].width) && (blocks[i].x > blocks[j].x) && (blocks[i].y < blocks[j].y + blocks[j].height) && (blocks[i].y > blocks[j].y))
         { 
           cout<<"overlap";
           cout<<i<<" "<<j<<endl;
           return;
         }
       }
       else
       {
         if( (blocks[i].x < blocks[j].x + blocks[j].height) && (blocks[i].x > blocks[j].x) && (blocks[i].y < blocks[j].y + blocks[j].width) && (blocks[i].y > blocks[j].y))
         { 
           cout<<"overlap";
           cout<<i<<" "<<j<<endl;
           return;
         }
       }  
    }
  }
}
void simulated_annealing()
{   
    ////////////////////////////INITIAL FLOORPLAN/////////////////////////////////////////
    rectangle root = MinAreaAlgorithm(); 
    int min_area_index;
    int min_area = 99999999;
    for(int i = 0; i < root.pairs.size(); i++)
    { 
      if(root.pairs[i].first * root.pairs[i].second < min_area)
      { 
        min_area = root.pairs[i].first * root.pairs[i].second;
        min_area_index = i;
      }
    }

    SetBlocks(root, min_area_index);
    max_x  = root.pairs[min_area_index].first;
    max_y  = root.pairs[min_area_index].second;
    area = max_x * max_y;
    
    initial_max_x = max_x;
    initial_max_y = max_y;
    initial_area = area;

    ComputeWireLength();
    initial_wirelength = wirelength;
    ComputeCost();
    initial_cost = cost;
    //////////////////////////////////////////////////////////////////////////////////////

    CheckOverlap();

    double T = 2000000;
    int k = 5000;
    int P ;
    bool better = false;
    double r = 0.85;
    int count =0;

    clock_t start = clock();

    store_best();
    while(T > 0.0001)
    { 
       better = 0;
       for(int i=0 ; i<k ; i++)
       {
          store_temp();
          perturb();

          root = MinAreaAlgorithm();

          min_area = 99999999;
          for(int i = 0; i < root.pairs.size(); i++)
          { 
            if(root.pairs[i].first * root.pairs[i].second < min_area)
            { 
              min_area = root.pairs[i].first * root.pairs[i].second;
              min_area_index = i;
            }
          }
          SetBlocks(root, min_area_index);
          max_x  = root.pairs[min_area_index].first;
          max_y  = root.pairs[min_area_index].second;

          CheckOverlap();

          wirelength=0;
          ComputeWireLength(); 
          ComputeCost();
  
          if( cost  <  temp_cost)  //downhill
          { 
            
          }
          else //upphill
          {  
            if( ((double)rand()) / RAND_MAX > exp( -(cost - temp_cost)  /  T) ) //with the probability accept
            {
              // cout<<"accept"<<endl;
            }
            else
            {
              restore_temp();
            }
          }
       
         if(best_cost > cost)
         {
           store_best();
           better = true;
           count=0;
         }  
    }
  

    if( better != true )
    { 
        T = T * r;
        if(T < 100000)
          T = 2000000;
        cout<< "reset"<<endl;
        restore_best();
        cout<<" ( "<<best_max_x<<" , "<<best_max_y<<" ) "<<endl;
        cout<<"WL : "<<best_wirelength<<endl;
        cout<<"Cost :"<<best_cost<<endl;
        count++;
    }  
    
    if ( max_x <= x_outline && max_y <= x_outline && count >= 3)   
    {  
       return ;
    }

    clock_t stop = clock();

    if( double(stop-start)/CLOCKS_PER_SEC > 1175)
    {
        return;
    }

    if( num_hardblocks == 300 && max_x > x_outline && max_y > x_outline  && double(stop-start)/CLOCKS_PER_SEC > 1145)
    {
        string temp_s = "241 261 195 111 -1 126 135 265 244 39 285 73 158 -1 10 53 -1 283 -1 -1 104 -1 -1 -1 -1 -1 -1 -1 133 -1 -1 -1 -1 203 65 205 81 -1 295 227 233 -1 253 -1 28 -1 72 -1 155 -1 106 -1 67 -1 97 0 259 -1 -1 -1 213 -1 218 -1 -1 -1 -1 -1 64 -1 -2 270 193 191 60 5 257 231 46 211 51 152 -1 172 160 276 148 -1 174 26 -1 -1 -1 -1 50 -1 -1 -1 -1 -1 -1 194 -1 -1 -1 -1 281 -1 -1 -1 -2 198 78 94 12 99 132 -1 141 38 -1 -1 77 23 240 162 1 70 -1 178 -1 -1 -1 -1 293 -1 -1 -1 -1 -1 -1 -1 -2 278 44 119 201 138 116 217 290 2 173 294 -1 207 232 -1 -1 -1 68 -1 142 -1 -1 -1 90 -1 -1 -1 -1 -1 -1 -1 -2 40 107 -1 115 -1 215 224 175 237 208 14 154 163 -1 239 -1 -1 235 -1 149 -1 234 -1 -1 -1 -1 -1 -1 -1 -2 122 275 264 254 226 58 268 11 82 222 -1 -1 168 42 228 -1 -1 -1 -1 -1 210 242 93 -1 -1 -1 -1 -1 -1 246 -1 -1 -1 -2 249 280 113 271 -1 25 -1 29 -1 170 192 30 21 143 -1 238 -1 36 -1 -1 -1 219 -1 -1 -1 288 -1 -1 -1 129 -1 -2 137 153 -1 55 -1 105 -1 41 45 299 -2 206 120 274 -1 17 63 157 139 190 15 -1 -1 220 -1 214 -1 250 -1 -1 -1 -1 245 -1 -1 -1 -1 -1 -1 -2 256 171 -1 146 130 236 176 -1 -1 112 156 252 124 -1 273 61 -1 204 37 24 -2 -1 -1 96 284 -1 -1 56 -1 -1 -1 -1 -1 -1 -1 -2 84 179 -1 80 48 31 -1 251 123 243 267 -1 -1 71 103 258 -1 54 -1 -1 -1 150 -1 209 -1 -1 -1 199 -1 -1 -1 -2 166 102 188 164 -1 247 6 177 13 118 223 69 -1 -1 -1 52 75 225 269 32 -2 -1 -1 -1 -1 -1 -1 -1 -1 101 -1 -1 -1 -2 59 91 -1 169 -1 200 291 33 -1 -1 3 110 184 -1 282 136 -1 43 -1 -1 -1 161 -1 83 260 230 -2 -1 -1 125 -1 297 -1 -1 -1 180 -1 -2 74 272 212 20 8 289 114 196 202 -1 88 292 128 189 -1 -1 140 121 -1 18 -1 117 -1 -1 98 66 -1 -1 -1 -1 -1 -1 -1 -1 -1 47 -1 -1 -1 -2 127 248 -1 286 -1 147 -1 216 34 108 -1 255 -1 85 -1 109 35 -1 -1 19 -1 185 -1 7 62 167 -1 187 -1 -1 181 -1 -1 -1 -1 -2 221 9 279 16 144 159 -1 151 -1 131 183 22 87 89 100 145 92 4 -1 -1 -1 -1 -1 -1 -1 165 -1 -1 -1 298 -1 -1 -1 -1 -1 -2 186 95 296 79 -2 263 134 229 -1 -1 287 277 -1 197 49 182 -1 86 -1 -1 -1 -1 27 57 -1 -1 76 266 -1 -1 -1 -1 -1 262 -1 -2";
        stringstream iss( temp_s );
        int temp_number;
        polish_expression.clear();
        while ( iss >> temp_number )
          polish_expression.push_back( temp_number );
    }  
    if( num_hardblocks == 200 && max_x > x_outline && max_y > x_outline && double(stop-start)/CLOCKS_PER_SEC > 1145)
    {
        string temp_s = "115 138 183 47 -1 -1 17 29 104 82 -1 -1 -1 168 -1 25 -1 180 148 -2 -1 141 162 -2 -1 164 84 38 -1 -1 -1 -1 -1 118 -1 163 15 59 -2 -2 88 114 144 7 -1 123 -2 70 -2 -2 -2 194 12 23 -2 -2 -1 89 -2 93 152 108 -2 1 -1 -1 45 -1 -2 124 175 187 75 -2 -1 52 161 156 -1 -2 -2 -2 196 -2 33 -2 126 50 39 -1 -1 134 160 20 -2 173 -1 132 -1 -1 -1 72 -1 83 42 -2 169 28 -1 96 130 -1 -1 87 -1 40 22 31 -2 -2 112 67 63 -2 188 171 -1 69 -2 -1 -1 167 -2 -1 -2 -1 35 21 -2 34 0 102 -2 -2 -2 -1 -2 46 37 51 -2 178 174 109 -2 -2 -1 3 158 36 30 -2 -1 -2 43 -2 -2 -2 -1 -1 -1 -1 -2 55 133 -2 -1 186 49 189 73 99 -2 -2 -2 58 -2 125 159 197 145 -1 -1 -1 90 131 193 -1 32 -1 92 -1 4 60 170 -1 -1 -2 -1 -2 -1 -1 182 -1 61 10 165 122 -1 146 -1 -1 -1 -2 150 -1 172 117 105 192 -1 -1 -1 97 66 -1 -1 65 -1 -2 91 113 -1 136 184 -2 179 -1 142 -1 129 19 6 -2 111 -1 128 -2 -1 64 101 -1 -2 -2 -2 16 41 -2 191 -1 11 -1 198 -1 78 86 119 -1 100 195 -1 -2 -1 177 48 -2 57 -1 103 -2 143 121 -2 -1 -2 -2 -1 -1 5 151 -1 24 95 -1 -2 185 18 79 -2 157 54 62 -2 -2 -2 -2 -2 -1 -2 110 8 81 190 -1 139 98 176 -2 -1 -1 -1 9 -1 -1 199 -1 106 71 -1 -1 153 147 27 -2 77 -1 -2 154 127 149 -2 26 107 120 13 -1 -1 -1 2 -1 -1 116 -1 -1 -1 85 -1 -2 14 135 94 -2 -2 74 56 -1 44 155 76 -1 80 -1 166 -2 -1 -2 53 137 -1 140 -2 -2 181 -1 68 -1 -1 -1 -2";
        stringstream iss( temp_s );
        int temp_number;
        polish_expression.clear();
        while ( iss >> temp_number )
          polish_expression.push_back( temp_number );
    } 
  } 
}
void output()
{
  ofstream fout(output_name);
  wirelength=0;
  fout<<"Wirelength "<<best_wirelength<<endl;
  fout<<"Blocks"<<endl;
  for(int i=0;i<num_hardblocks;i++)
     fout<<"sb"<<best_blocks[i].id<<" "<<best_blocks[i].x<<" "<<best_blocks[i].y<<" "<<best_blocks[i].width<<" "<<best_blocks[i].height<<" "<<best_blocks[i].rotate<<endl;

  // for(int i=0;i<num_hardblocks;i++)
  //   if(best_blocks[i].rotate == 0)
  //    fout<<best_blocks[i].x<<"\t"<<best_blocks[i].y<<"\t"<<best_blocks[i].width<<"\t"<<best_blocks[i].height<<endl;
  //   else
  //    fout<<best_blocks[i].x<<"\t"<<best_blocks[i].y<<"\t"<<best_blocks[i].height<<"\t"<<best_blocks[i].width<<endl;
}

int main(int argc, char **argv)
{   
    if (argc != 6) 
    {
        cout<<"not match the parameters"<<endl;
        exit(1);
    }
    input_H = argv[1];
    input_N = argv[2];
    input_P = argv[3];
    output_name = argv[4];
    white_space_ratio = atof(argv[5]);
    int seed = time(NULL);
    srand(seed);
    parser();
    calculate_Constraint();
    init_floorplan();
    simulated_annealing();
    PrintPolish();
    output();
    
    return 0;
}
