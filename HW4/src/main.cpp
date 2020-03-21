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

struct Node        
{
  string id;
  int width;
  int height;
  int x;
  int y;
  int modified_Global_x;
  double Global_x;
  double Global_y;
  int subrow_index;
  vector<int> trail_subrow_index;
};
struct Terminal        
{
  string id;
  int width;
  int height;
  double Global_x;
  double Global_y;
};
struct Cluster
{
  double begin_x;
  int width;
  int weight;
  vector<int> index;
};
struct Row
{
  int Coordinate;
  int Height;
  int Sitewidth;
  int Sitespacing;
  int Siteorient;
  int Sitesymmetry;
  int SubrowOrigin;
  int NumSites;  
  vector< Cluster > clusters;
};

string  input_AUX , input_NODE , input_GLOBAL ,input_ROW, output_name;
string  file_path;
int MaxDisplacement = 0;
vector<struct Node> Nodes_vec;
vector<struct Node> unchanged_Nodes_vec;
vector<struct Terminal> Terminals_vec;
vector<struct Row> Rows_vec;
vector<struct Row> subRows_vec;
vector<struct Row> unchanged_subRows_vec;
int node_num, terminal_num, row_num;

string SplitFilename (const std::string& str)
{
  std::size_t found = str.find_last_of("/\\");
  return str.substr(0,found);
}

void parser()
{   
  string temp_s; 
  
  ifstream file_AUX(input_AUX);
  file_AUX >> temp_s;             //RowBasedPlacement
  file_AUX >> temp_s;             //: 
  file_AUX >> input_NODE;         //adaptec1.nodes
  file_AUX >> input_GLOBAL;       //adaptec1.ntup.pl
  file_AUX >> input_ROW;          //adaptec1.scl
  file_AUX >> temp_s;             //MaxDisplacement
  file_AUX >> temp_s;             //:
  file_AUX >> MaxDisplacement;    //120
  file_AUX.close();
  
  file_path = SplitFilename(input_AUX) + '/';

  ifstream file_NODE( file_path + input_NODE);
  while(file_NODE >> temp_s)
  {   
  if(temp_s == "NumNodes")
  {
    file_NODE>>temp_s;
    file_NODE>>node_num;
    file_NODE>>temp_s;
    file_NODE>>temp_s;
    file_NODE>>terminal_num;
    break;
  }
  }
  for(int i = 0; i < node_num - terminal_num; i++)
  {   
    Node temp_n;  
    file_NODE>>temp_n.id;   
    file_NODE>>temp_n.width;
    file_NODE>>temp_n.height;
    Nodes_vec.push_back(temp_n);
    // cout<<temp_n.id<<" "<<temp_n.width<<" "<<temp_n.height<<endl;
  }
  for(int i = 0; i < terminal_num; i++)
  {   
    Terminal temp_t;  
    file_NODE>>temp_t.id;   
    file_NODE>>temp_t.width;
    file_NODE>>temp_t.height;
    file_NODE>>temp_s; 
    Terminals_vec.push_back(temp_t);
  }
  file_NODE.close();
  
  ifstream file_GLOBAL(file_path + input_GLOBAL);
  getline(file_GLOBAL, temp_s);
  getline(file_GLOBAL, temp_s);
  for(int i = 0; i < node_num - terminal_num; i++)
  {
    file_GLOBAL>>temp_s;
    file_GLOBAL>>Nodes_vec[i].Global_x;
    file_GLOBAL>>Nodes_vec[i].Global_y;
    file_GLOBAL>>temp_s;
    file_GLOBAL>>temp_s;
  }
  for(int i = 0; i < terminal_num; i++)
  {   
    file_GLOBAL>>temp_s;
    file_GLOBAL>>Terminals_vec[i].Global_x;
    file_GLOBAL>>Terminals_vec[i].Global_y;
    file_GLOBAL>>temp_s;
    file_GLOBAL>>temp_s;
    file_GLOBAL>>temp_s;
  }
  file_GLOBAL.close();
  
  //for(int i = 0; i < nodes_num - terminal_num; i++)
  //{
  // cout<<Nodes_vec[i].id<<" "<<Nodes_vec[i].width<<" "<<Nodes_vec[i].height<<" "<<Nodes_vec[i].Global_x<<" "<<Nodes_vec[i].Global_y<<endl;
  //}
  //for(int i = 0; i < terminal_num; i++)
  //{   
  // cout<<Terminals_vec[i].id<<" "<<Terminals_vec[i].width<<" "<<Terminals_vec[i].height<<" "<<Terminals_vec[i].Global_x<<" "<<Terminals_vec[i].Global_y<<endl;  
  //}
  
  ifstream file_ROW(file_path + input_ROW);
  while(file_ROW >> temp_s)
  {   
  if(temp_s == "NumRows")
  {
    file_ROW>>temp_s;
    file_ROW>>row_num;
    break;
  }
  }
  // cout<<row_num<<endl;
  for(int i = 0; i < row_num; i++)
  {   
    Row temp_r;
    file_ROW>>temp_s;
    file_ROW>>temp_s;
    file_ROW>>temp_s;
    file_ROW>>temp_s;
    file_ROW>>temp_r.Coordinate;
    file_ROW>>temp_s;
    file_ROW>>temp_s;
    file_ROW>>temp_r.Height;
    file_ROW>>temp_s;
    file_ROW>>temp_s;
    file_ROW>>temp_r.Sitewidth;
    file_ROW>>temp_s;
    file_ROW>>temp_s;
    file_ROW>>temp_r.Sitespacing;
    file_ROW>>temp_s;
    file_ROW>>temp_s;
    file_ROW>>temp_r.Siteorient;
    file_ROW>>temp_s;
    file_ROW>>temp_s;
    file_ROW>>temp_r.Sitesymmetry;
    file_ROW>>temp_s;
    file_ROW>>temp_s;
    file_ROW>>temp_r.SubrowOrigin;
    file_ROW>>temp_s;
    file_ROW>>temp_s;
    file_ROW>>temp_r.NumSites;
    file_ROW>>temp_s; 
    // cout<<temp_r.Coordinate<<" "<<temp_r.Height<<" "<<temp_r.Sitewidth<<" "<<temp_r.Sitespacing<<" "<<temp_r.Siteorient<<" "<<temp_r.Sitesymmetry<<" "<<temp_r.SubrowOrigin<<" "<<temp_r.NumSites<<endl;
    Rows_vec.push_back(temp_r);
  }
  file_ROW.close();
}

bool CompareByX(const pair<int, int> &a, const pair<int, int> &b)
{
    return a.first < b.first;
}
void CutSubRows()
{ 
  vector< pair<int, int> > terminal_x_width; // the (x_coordinate, width) of a Terminal in this row
  if(Terminals_vec.size() == 0)
  {
    subRows_vec = Rows_vec;
  }
  else
  {  
    for(int i = 0; i < Rows_vec.size(); i++)
    { 
      terminal_x_width.clear();
      terminal_x_width.resize(0);

      for(int j = 0; j < Terminals_vec.size(); j++ )
      { 
        if( Rows_vec[i].Coordinate >= Terminals_vec[j].Global_y && Rows_vec[i].Coordinate < Terminals_vec[j].Global_y + Terminals_vec[j].height) // the row is in the height range of terminal
        { 
          if( Terminals_vec[j].Global_x >= Rows_vec[i].SubrowOrigin && Terminals_vec[j].Global_x < Rows_vec[i].SubrowOrigin + Rows_vec[i].NumSites * Rows_vec[i].Sitewidth) // the terminal is in the width range of the row
          {
             terminal_x_width.push_back( make_pair(Terminals_vec[j].Global_x, Terminals_vec[j].width) );
          }   
        }
      }

      sort(terminal_x_width.begin(), terminal_x_width.end(), CompareByX);

      // cout<<"Row : "<<i<<"  ===> ";
      for(int j = 0; j < terminal_x_width.size(); j++ )
      { 
        Row temp_Row;
        temp_Row = Rows_vec[i];
        if(j == 0 )
        {
          temp_Row.NumSites = ( terminal_x_width[j].first - temp_Row.SubrowOrigin ) / temp_Row.Sitewidth;
        }
        else
        {
          temp_Row.SubrowOrigin = terminal_x_width[j - 1].first + terminal_x_width[j - 1].second;
          temp_Row.NumSites = ( terminal_x_width[j].first - temp_Row.SubrowOrigin ) / temp_Row.Sitewidth;
        }
        if(temp_Row.NumSites != 0) //filter out 0 width subrow
        {
          // cout<<"("<<temp_Row.SubrowOrigin<<", "<< temp_Row.SubrowOrigin + temp_Row.NumSites * temp_Row.Sitewidth<<") ";
          subRows_vec.push_back(temp_Row);
        }

        if( j == terminal_x_width.size() - 1) // deal with the last subrow
        { 
          double Row_end = Rows_vec[i].SubrowOrigin + Rows_vec[i].Sitewidth * Rows_vec[i].NumSites;
          temp_Row.SubrowOrigin = terminal_x_width[j].first + terminal_x_width[j].second;
          temp_Row.NumSites = ( Row_end - temp_Row.SubrowOrigin ) / temp_Row.Sitewidth;
          if(temp_Row.NumSites != 0) //filter out 0 width subrow
          {
            // cout<<"("<<temp_Row.SubrowOrigin<<", "<< temp_Row.SubrowOrigin + temp_Row.NumSites * temp_Row.Sitewidth<<") "<<endl;
            subRows_vec.push_back(temp_Row);
          }  
        }
      }
    }
   } 
  cout<<"row size   : "<<Rows_vec.size()<<endl;
  cout<<"subrow size: "<<subRows_vec.size()<<endl;
}
bool ComparebyX(const Node &a, const Node &b)
{
    return (a.modified_Global_x + a.width/2) < (b.modified_Global_x + b.width/2);
}
void SortNode()
{
  sort(Nodes_vec.begin(), Nodes_vec.end(), ComparebyX);
}
/* 
to-do: 怎麼決定begin_x 再想想
to-do: 不要用整個X Y座標去除site 不然還要四捨五入會有誤差 看能不能改為一個sitewidth為一單位去做


*/
void AddCell(Cluster *C, int node_index)
{ 
  Node *NODE = &Nodes_vec[node_index];
  double temp_1 = C->weight * C->begin_x/subRows_vec[0].Sitewidth ;
  double temp_2 = 1 * NODE->modified_Global_x/subRows_vec[0].Sitewidth - C->width/subRows_vec[0].Sitewidth;
  double temp_3 = 1 + C->weight;
  C->begin_x =  (int)( (temp_1 + temp_2)/temp_3 ) * subRows_vec[0].Sitewidth;
  C->width   = C->width   + NODE->width;
  C->weight  = C->weight  + 1;
  C->index.push_back(node_index);
}
void AddCluster(Cluster *C, Cluster *C_prev)
{
  for(int i = 0; i < C->index.size(); i++)
  {
    C_prev->index.push_back(C->index[i]);
  }
  double temp_1 = C_prev->weight * C_prev->begin_x/subRows_vec[0].Sitewidth ;
  double temp_2 = C->weight * ( C->begin_x/subRows_vec[0].Sitewidth - C_prev->width/subRows_vec[0].Sitewidth );
  double temp_3 = C_prev->weight + C->weight;
  C_prev->begin_x = (int)( (temp_1 + temp_2)/temp_3 ) * subRows_vec[0].Sitewidth;
  C_prev->width   = C_prev->width   + C->width;
  C_prev->weight  = C_prev->weight + C->weight;
}
void Collapse(int cluster_index, int subrow_index)
{ 
  Row    *SUBROW = &subRows_vec[subrow_index];
  double Row_end = SUBROW->SubrowOrigin + SUBROW->Sitewidth * SUBROW->NumSites;
  while(cluster_index != 0)
  { 
    Cluster *C = &SUBROW->clusters[cluster_index];
    if(C->begin_x < SUBROW->SubrowOrigin) //smaller than left boundary
      C->begin_x = SUBROW->SubrowOrigin;

    if(C->begin_x + C->width >  Row_end) //bigger than right boundary
      C->begin_x = Row_end - C->width;

    if(SUBROW->clusters[cluster_index-1].begin_x + SUBROW->clusters[cluster_index-1].width > C->begin_x)
    {
        AddCluster(&SUBROW->clusters[cluster_index], &SUBROW->clusters[cluster_index - 1]);
        SUBROW->clusters.pop_back();
        cluster_index--;
    } 
    else
      break;      
  }
  Cluster *C = &SUBROW->clusters[0];
  if(C->begin_x < SUBROW->SubrowOrigin) //smaller than left boundary
    C->begin_x = SUBROW->SubrowOrigin;

  if(C->begin_x + C->width >  Row_end) //bigger than right boundary
    C->begin_x = Row_end - C->width;
}
void PlaceRow(int node_index, int subrow_index)
{ 
  Node *NODE   = &Nodes_vec[node_index];
  Row  *SUBROW = &subRows_vec[subrow_index];
  
  if( SUBROW->clusters.empty() )
  { 
    Cluster temp_c;
    temp_c.begin_x = NODE->modified_Global_x;
    temp_c.width   = NODE->width;
    temp_c.index.push_back(node_index);
    SUBROW->clusters.push_back(temp_c);
  }    
  else
  { 
    int cluster_id = SUBROW->clusters.size() - 1;
    if( NODE->Global_x >= SUBROW->clusters[cluster_id].begin_x + SUBROW->clusters[cluster_id].width )
    {
      Cluster temp_c;
      temp_c.begin_x = NODE->modified_Global_x;
      temp_c.width   = NODE->width;
      temp_c.index.push_back(node_index);
      SUBROW->clusters.push_back(temp_c);
    }
    else
    {
      AddCell(&SUBROW->clusters[cluster_id], node_index);
      Collapse(cluster_id, subrow_index);
    }
  }
}
void SetNodes(int subrow_index)
{
  Row  *SUBROW = &subRows_vec[subrow_index];
  for(int i = 0; i < SUBROW->clusters.size(); i++)
  { 
    int temp_x = SUBROW->clusters[i].begin_x;
    for(int j = 0; j < SUBROW->clusters[i].index.size(); j++)
    { 
      int node_index = SUBROW->clusters[i].index[j];
      Nodes_vec[node_index].x = temp_x;
      Nodes_vec[node_index].y = SUBROW->Coordinate;
      Nodes_vec[node_index].subrow_index = subrow_index;
      // cout<<" ( "<<Nodes_vec[node_index].x<<" , "<<Nodes_vec[node_index].x + Nodes_vec[node_index].width<<" ) "<<"        "<<Nodes_vec[node_index].modified_Global_x<<endl;
      temp_x += Nodes_vec[node_index].width;
    }
    // cout<<"next cluster"<<endl;
  }
  // cout<<endl<<endl;
}
int ComputeCost(int node_id, int subrow_index)
{ 
  double cost = 0;
  Row  *SUBROW = &subRows_vec[subrow_index];
  for(int i = 0; i < SUBROW->clusters.size(); i++)
  { 
    int temp_x = SUBROW->clusters[i].begin_x;
    for(int j = 0; j < SUBROW->clusters[i].index.size(); j++)
    {  
      int node_index = SUBROW->clusters[i].index[j];

      if(temp_x <  SUBROW->SubrowOrigin || (temp_x +  Nodes_vec[node_index].width) > (SUBROW->SubrowOrigin + SUBROW->NumSites * SUBROW->Sitewidth))
      {
        // cout<<"x : "<<temp_x<<" left-bound : "<<SUBROW->SubrowOrigin<<" right-bound : "<<(SUBROW->SubrowOrigin + SUBROW->NumSites * SUBROW->Sitewidth)- Nodes_vec[node_index].width<<endl;
        return 99999999;
      } 

      if(node_id == node_index)
      {  
        double x_diff = temp_x - Nodes_vec[node_index].Global_x;
        double y_diff = SUBROW->Coordinate - Nodes_vec[node_index].Global_y;
        cost = sqrt( pow(x_diff, 2) + pow(y_diff, 2) ); 
      }
        
      temp_x += Nodes_vec[node_index].width; 
    }
  }
  return cost;
}
void Abacus()
{ 
  SortNode(); // left to right or right to left?
  double cost, cost_best, best_row;
  double total_cost = 0;
  for(int i = 0 ; i < Nodes_vec.size(); i++)
  { 
    // cout<<i<<endl;
    cost_best = 999999999;
    for(int j = 0; j < Nodes_vec[i].trail_subrow_index.size(); j++)
    {   
        int row_index = Nodes_vec[i].trail_subrow_index[j];
        vector<Cluster> unchanged_clusters = subRows_vec[row_index].clusters;
        PlaceRow(i, row_index);
        cost = ComputeCost(i, row_index); 
        if(cost < cost_best)
        { 
          cost_best = cost;
          best_row = row_index;
        }
        subRows_vec[row_index].clusters = unchanged_clusters;
    }
    if(cost_best > 1000)
    {
      for(int j = 0; j < subRows_vec.size(); j++)
      {   
          vector<Cluster> unchanged_clusters = subRows_vec[j].clusters;
          PlaceRow(i, j);
          cost = ComputeCost(i, j); 
          if(cost < cost_best)
          { 
            cost_best = cost;
            best_row = j;
          }
          subRows_vec[j].clusters = unchanged_clusters;
      }
    }


    // cout<<"row "<<best_row<<"  cost : "<<cost_best<<"    Node H = "<<Nodes_vec[i].Global_y<<"  Row H = "<<subRows_vec[best_row].Coordinate<<endl;
    PlaceRow(i, best_row);
    SetNodes(best_row);
    total_cost += cost_best;
  }
  // cout<<"Total Cost : "<<total_cost<<endl;
}
bool ComparebyId(const Node &a, const Node &b)
{
    return a.id > b.id;
}
void output()
{ 
  std::size_t found1 = input_AUX.find_last_of(".");
  string temp_name = input_AUX.substr(0,found1);
  std::size_t found2 = temp_name.find_last_of("/\\");

  output_name = "../output/" + temp_name.substr(found2 + 1,temp_name.size()) + ".result";
  ofstream fout(output_name);
  fout<<"UCLA pl 1.0 "<<endl<<endl;
  sort(Nodes_vec.begin(), Nodes_vec.end(), ComparebyId);
  for(int i = 0; i < node_num - terminal_num; i++)
  {
    fout<<Nodes_vec[i].id<<" "<<Nodes_vec[i].x<<" "<<Nodes_vec[i].y<<" : N"<<endl;
  }
  for(int i = 0; i < terminal_num; i++)
  {
    fout<<Terminals_vec[i].id<<" "<<Terminals_vec[i].Global_x<<" "<<Terminals_vec[i].Global_y<<" : N / FIXED"<<endl;
  }
}
void Check()
{ 
  int X_begin, X_end, Y_begin, Y_end;
  int index;
  for(int i = 0; i < Nodes_vec.size(); i++)
  {
    index = Nodes_vec[i].subrow_index;
    X_begin = subRows_vec[index].SubrowOrigin;
    X_end   = X_begin + subRows_vec[index].NumSites * subRows_vec[index].Sitewidth;
    Y_begin = subRows_vec[index].Coordinate;
    Y_end   = Y_begin + subRows_vec[index].Height;

    // cout<<"Node : "<< Nodes_vec[i].x <<" "<<Nodes_vec[i].y<<" "<<Nodes_vec[i].width<<" "<<Nodes_vec[i].height<<endl;
    // cout<<"X_begin : "<<X_begin<<" X_end : "<<X_end<<" Y_begin : "<<Y_begin<<" Y_end : "<<Y_end<<endl; 
    if(  ( (int)Nodes_vec[i].x - (int)X_begin) % subRows_vec[index].Sitewidth != 0 )
        cout<<"violate align"<<endl;
    if(  (Nodes_vec[i].x < X_begin) || Nodes_vec[i].x + Nodes_vec[i].width > X_end )
        cout<<"violate align"<<endl; 

    if( (int)Nodes_vec[i].width % subRows_vec[index].Sitewidth != 0)  
      cout<<"width problem"<<endl;
  }
}
void Modify()
{
  for(int i = 0 ; i < Nodes_vec.size(); i++)
  { 
    double q;
    q = Nodes_vec[i].Global_x / subRows_vec[0].Sitewidth;
    if(q >= 0)
      Nodes_vec[i].modified_Global_x = (int)(q+0.5) * subRows_vec[0].Sitewidth;
    else
      Nodes_vec[i].modified_Global_x = (int)(q-0.5) * subRows_vec[0].Sitewidth;

    int Node_Y = Nodes_vec[i].Global_y;
    for(int j = 0 ; j < subRows_vec.size(); j++)
    {
      if( abs(subRows_vec[j].Coordinate - Node_Y) < 50)
      {
        Nodes_vec[i].trail_subrow_index.push_back(j);
      }
    }

  }
}
int main(int argc, char **argv)
{   
    if (argc != 2) 
    {
        cout<<"not match the parameters"<<endl;
        exit(1);
    }
    input_AUX = argv[1];
    parser();
    CutSubRows();
    Modify();
    Abacus();
    // Check();
    //optimize() //try to legalize a row from right to left again to get smaller displacement
    output();
    
    return 0;
}