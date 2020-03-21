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
#include <stdlib.h>
#include <math.h>
#define C
using namespace std;
class ListNode
{
    public:
        ListNode(int a) :  id(a),next(0),prev(0){}; //id in vc
        int       id;
        ListNode *next;
        ListNode *prev;
};
typedef struct cell
{
    string name;
    int    size;
    vector<string> connect_nets;
    bool in_A;
    bool lock;
    int  gain;
    ListNode *goto_list;
} Cell;

typedef struct net
{
    string name;
    vector<string> cells_name;
    vector<int>    cells_id;  // cell index in vc
    int in_A;
    int in_B;
} Net;

string input_cell , input_net ,output_name;
vector<Cell> vc;
vector<Net>  vn;
vector<int> move_order;
int k=0;
int total_cell_size=0;
int vc_size=0 , vn_size=0;
int cut_size=0;
int Constraint=0;
int Pmax=0 , Smax=0;
int total_A_size=0, total_B_size=0;
int vA_size=0 , vB_size=0 , bucketA_size=0 , bucketB_size=0;
int total_gain_after_move=0;
int max_total_gain = -999;
map <int, ListNode*> bucket_list[2]; // 1 A 0 B
map<string, int> cell_map_name_id;

void init_partition()
{
     for(int i=0;i<vc_size;i++)
     {
        total_cell_size+= vc[i].size;
     }


     for(int i=0 ; i<vc_size ; i++ ){
        if( total_A_size <= total_cell_size/2 )
        {
             vA_size++;
             vc[i].in_A=true;
             total_A_size += vc[i].size;
        }
        else
        {
             vB_size++;
             vc[i].in_A=false;
             total_B_size += vc[i].size;                       
        }
     }
     
}
void find_connect_net()
{
    for(int i=0;i<vn_size ;i++)
    {
        vn[i].in_A=0;
        vn[i].in_B=0;
    }

    for(int i=0; i<vn_size; i++)
    {
        sort(vn[i].cells_id.begin(), vn[i].cells_id.end());
        int cell_size = vn[i].cells_id.size();
        for(int j=0; j<cell_size ; j++)
        {
            int c_id = vn[i].cells_id[j];

            if(vc[c_id].in_A == true)
                vn[i].in_A++;
            else
                vn[i].in_B++;
            vc[c_id].connect_nets.push_back(vn[i].name);
        }

    }
}
void countCutSize()
{
    cut_size=0;
    for(int i=0; i<vn_size ; i++)
    {
        if(vn[i].in_A && vn[i].in_B)
            cut_size++;
    }
}
void countConstraint()
{
    Constraint = total_cell_size/10;
}
void countPmax()
{
     int P , S;
     for(int i=0;i<vc_size;i++)
     {
        P=vc[i].connect_nets.size();
        S = vc[i].size;
        if( Pmax < P)
            Pmax = P;
     }
}
void buildBucketList()  //ok
{
    bucket_list[1].clear();
    bucket_list[0].clear();

    for (int i = -Pmax; i <= Pmax; i++)
    {
            bucket_list[1][i] = new ListNode(-1);
            bucket_list[0][i] = new ListNode(-1);
    }
    for (int i = 0; i <vc_size ; i++)
    {
        int  gain = vc[i].gain;
        bool in_A = vc[i].in_A;

        ListNode *temp = new ListNode(i);
        if(in_A)
        {
            vc[i].goto_list = temp;
            temp->prev = bucket_list[1][gain];
            temp->next = bucket_list[1][gain]->next;
            bucket_list[1][gain]->next = temp;
            if (temp->next != NULL)
                temp->next->prev = temp;
        }
        else
        {
            vc[i].goto_list = temp;
            temp->prev = bucket_list[0][gain];
            temp->next = bucket_list[0][gain]->next;
            bucket_list[0][gain]->next = temp;
            if (temp->next != NULL)
                temp->next->prev = temp;
        }
    }

    //cout<<"bucket A"<<bucketA_size<< endl;
    //cout<<"bucket B"<<bucketB_size<< endl;
   /* for (int i = -Pmax; i <= Pmax; i++) {
            ListNode *current = bucket_list[1][i]->next;
            cout<<i<<" ";
            while (current != 0) {
                cout << current->id << " ";
                current = current->next;
            }
            cout << endl;
    }
    for (int i = -Pmax; i <= Pmax; i++) {
            ListNode *current = bucket_list[0][i]->next;
            cout<<i<<" ";
            while (current != 0) {
                cout << current->id << " ";
                current = current->next;
            }
            cout << endl;
    }*/
}
void initial_gain()  //ok
{
    for (int i = 0; i < vc_size; i++)
    {
        vc[i].gain = 0;
        vc[i].lock = false;
    }

    for (int i = 0; i < vc_size ; i++){
        for (int j = 0 ; j < vc[i].connect_nets.size(); j++)
        {
             string str =vc[i].connect_nets[j];
             int length = str.length();
             string str2 =vc[i].connect_nets[j].substr(1,length);
             int index = atoi(str2.c_str()) - 1;
            if (vc[i].in_A == true)
            {
                if (vn[index].in_A==1)
                        vc[i].gain++;
                if (vn[index].in_B==0)
                        vc[i].gain--;
            }
            else
            {
                if (vn[index].in_B==1)
                        vc[i].gain++;
                if (vn[index].in_A==0)
                        vc[i].gain--;
            }
        }
    }

    bucketA_size = vA_size;
    bucketB_size = vB_size;
}
int findMaxGain(int in_A)
{
    int i;
    for( i = Pmax ; i> -Pmax ; i--)
    {
        if(bucket_list[in_A][i]->next != NULL)
             break;
    }
     int temp = bucket_list[in_A][i]->next->id;
    return temp;
}
void move_to_right_bucket(Cell *c)
{
    ListNode *temp = c->goto_list ;

    temp->prev->next = temp->next;
    if (temp->next != NULL)
        temp->next->prev = temp->prev;

    int  gain = c->gain;
    bool in_A = c->in_A;
    if(in_A){
        c->goto_list = temp;
        temp->prev = bucket_list[1][gain];
        temp->next = bucket_list[1][gain]->next;
        bucket_list[1][gain]->next = temp;
        if (temp->next != NULL)
            temp->next->prev = temp;
    }
    else{
        c->goto_list = temp;
        temp->prev = bucket_list[0][gain];
        temp->next = bucket_list[0][gain]->next;
        bucket_list[0][gain]->next = temp;
        if (temp->next != NULL)
            temp->next->prev = temp;
    }
}
void remove_from_bucket(Cell *c)
{
    ListNode *temp = c->goto_list ;

    temp->prev->next = temp->next;
    if (temp->next != NULL)
        temp->next->prev = temp->prev;
}
void updateGain(int cell_id)
{
    vc[cell_id].lock=true;
    total_gain_after_move += vc[cell_id].gain;
    
    
   // cout<<"id "<<cell_id<<" gain "<<vc[cell_id].gain<<endl;   
   /* for (int i = -Pmax; i <= Pmax; i++) {
            ListNode *current = bucket_list[1][i]->next;
            cout<<i<<" ";
            while (current != 0) {
                cout << current->id << " ";
                current = current->next;
            }
            cout << endl;
    }
    for (int i = -Pmax; i <= Pmax; i++) {
            ListNode *current = bucket_list[0][i]->next;
            cout<<i<<" ";
            while (current != 0) {
                cout << current->id << " ";
                current = current->next;
            }
            cout << endl;
    }*/
    
    
    
    
    if(vc[cell_id].in_A)  // cell ?¨A è¦?A->B
    {    
         bucketA_size--;
         total_A_size -= vc[cell_id].size;
         total_B_size += vc[cell_id].size;
         int connect_nets_size = vc[cell_id].connect_nets.size();
         for(int i = 0 ;i < connect_nets_size ; i++)
         {
             int length = vc[cell_id].connect_nets[i].length();
             string str = vc[cell_id].connect_nets[i].substr(1,length);
             int net_id = atoi(str.c_str()) - 1;


             int cells_size = vn[net_id].cells_id.size();

             for(int j=0 ; j<cells_size ; j++)
             {
                 int cell_id2 = vn[net_id].cells_id[j];

                 if(vn[net_id].in_B == 0)
                 {
                   if(vc[cell_id2].lock == false )
                   {
                      vc[cell_id2].gain++;
                      move_to_right_bucket(&vc[cell_id2]);
                   }
                 }
                 else if(vn[net_id].in_B == 1)
                 {
                   if(vc[cell_id2].lock == false && vc[cell_id2].in_A == false)
                   {
                       vc[cell_id2].gain--;
                       move_to_right_bucket(&vc[cell_id2]);
                   }
                 }
             }

             vn[net_id].in_A--;
             vn[net_id].in_B++;
             vc[cell_id].in_A=false;

             for(int j=0 ; j<cells_size ; j++)
             {
                 int cell_id2 = vn[net_id].cells_id[j];

                 if(vn[net_id].in_A == 0)
                 {
                   if(vc[cell_id2].lock == false )
                    {
                      vc[cell_id2].gain--;
                      move_to_right_bucket(&vc[cell_id2]);
                    }
                 }
                 else if(vn[net_id].in_A == 1)
                 {
                   if(vc[cell_id2].lock == false && vc[cell_id2].in_A == true)
                    {
                       vc[cell_id2].gain++;
                       move_to_right_bucket(&vc[cell_id2]);
                    }
                 }
             }

         }
    }

    else if(!vc[cell_id].in_A)  // cell ?¨B è¦?B->A
    {    
         bucketB_size--;
         total_A_size += vc[cell_id].size;
         total_B_size -= vc[cell_id].size;
         int connect_nets_size = vc[cell_id].connect_nets.size();
         for(int i = 0 ;i < connect_nets_size ; i++)
         {
             int length = vc[cell_id].connect_nets[i].length();
             string str = vc[cell_id].connect_nets[i].substr(1,length);
             int net_id = atoi(str.c_str()) - 1;


             int cells_size = vn[net_id].cells_id.size();

             for(int j=0 ; j<cells_size ; j++)
             {
                 int cell_id2 = vn[net_id].cells_id[j];

                 if(vn[net_id].in_A == 0)
                 {
                   if(vc[cell_id2].lock == false )
                   {
                       vc[cell_id2].gain++;
                       move_to_right_bucket(&vc[cell_id2]);
                   }
                 }
                 else if(vn[net_id].in_A == 1)
                 {
                   if(vc[cell_id2].lock == false && vc[cell_id2].in_A == true)
                   {
                      vc[cell_id2].gain--;
                      move_to_right_bucket(&vc[cell_id2]);
                   }
                 }
             }

             vn[net_id].in_B--;
             vn[net_id].in_A++;
             vc[cell_id].in_A=true;

             for(int j=0 ; j<cells_size ; j++)
             {
                 int cell_id2 = vn[net_id].cells_id[j];

                 if(vn[net_id].in_B == 0)
                 {
                   if(vc[cell_id2].lock == false )
                   {
                      vc[cell_id2].gain--;
                      move_to_right_bucket(&vc[cell_id2]);
                   }
                 }
                 else if(vn[net_id].in_B == 1)
                 {
                   if(vc[cell_id2].lock == false && vc[cell_id2].in_A == false)
                   {
                       vc[cell_id2].gain++;
                       move_to_right_bucket(&vc[cell_id2]);
                   }
                 }
             }

         }
    }
    
    move_order.push_back(cell_id);
    remove_from_bucket(&vc[cell_id]);

}
void FM_algorithm()
{
    initial_gain();
    buildBucketList();
    bool done = false;
    int times = 0;
    while (!done && times < vc_size){
            int cell_id_a , cell_id_b;
            if(bucketA_size != 0)
                cell_id_a =  findMaxGain(1);
            else
                cell_id_a = -1;

            if(bucketB_size != 0)
                cell_id_b =  findMaxGain(0);
            else
                cell_id_b = -1;

            if(cell_id_a == -1 && cell_id_b == -1 )
            {
                done = true;
            }
            else if(cell_id_a != -1 && cell_id_b == -1)
            {
                if (abs( ( total_A_size - vc[cell_id_a].size )- ( total_B_size + vc[cell_id_a].size ) ) <  Constraint)
                       updateGain(cell_id_a);
                else
                {
                  bucketA_size--;
                  vc[cell_id_a].lock = true;
                  remove_from_bucket(&vc[cell_id_a]);
                }
            }
            else if(cell_id_a == -1 && cell_id_b != -1)
            {
                if (abs( (total_B_size - vc[cell_id_b].size ) - (total_A_size + vc[cell_id_b].size) ) <  Constraint)
                       updateGain(cell_id_b);
                else
                {  
                   bucketB_size--;
                   vc[cell_id_a].lock = true;
                   remove_from_bucket(&vc[cell_id_b]);            
                }
            }
            else if(cell_id_a != -1 && cell_id_b != -1)
            {  
              // cout<<"a gain: "<<vc[cell_id_a].gain<<"  b gain "<<vc[cell_id_b].gain<<endl;
               if (vc[cell_id_a].gain >= vc[cell_id_b].gain) {
                if (abs( ( total_A_size - vc[cell_id_a].size )- ( total_B_size + vc[cell_id_a].size )  ) <  Constraint)
                       updateGain(cell_id_a);
                else if (abs( (total_B_size - vc[cell_id_b].size ) - (total_A_size + vc[cell_id_b].size)) < Constraint)
                       updateGain(cell_id_b);
                else
                {
                   remove_from_bucket(&vc[cell_id_a]);
                   remove_from_bucket(&vc[cell_id_b]);
                   bucketA_size--;
                   bucketB_size--;
                   vc[cell_id_a].lock = true;
                   vc[cell_id_b].lock = true;
                }
              }
              else {
                  if ( abs( (total_B_size - vc[cell_id_b].size ) - (total_A_size + vc[cell_id_b].size)) < Constraint)
                         updateGain(cell_id_b);
                  else if (abs( ( total_A_size - vc[cell_id_a].size )- ( total_B_size + vc[cell_id_a].size ) ) <  Constraint)
                         updateGain(cell_id_a);
                  else
                  {
                     remove_from_bucket(&vc[cell_id_a]);
                     remove_from_bucket(&vc[cell_id_b]);
                     bucketA_size--;
                     bucketB_size--;
                     vc[cell_id_a].lock = true;
                     vc[cell_id_b].lock = true;
                  }
              }
            }   
            
            
       if(max_total_gain < total_gain_after_move)
       {
          max_total_gain = total_gain_after_move;
          k = times;
       }
             
      // countCutSize();
      // cout<<"total_gain_after_move : "<<total_gain_after_move<<endl;
      // cout<<"times : "<<times<<" cut size : "<<cut_size<<endl<<endl<<endl;
       times++;
    }
    //cout<<"best times : "<< k <<" max_total_gain : " <<max_total_gain<<endl;
}
void recover()
{
    int size = move_order.size();
    for(int i=k+1 ; i<size ; i++)
    {
       vc[ move_order[i] ].in_A= !vc[ move_order[i] ].in_A;
    }

}
void output()
{  
   int A_count=0 ,B_count=0 , A_size=0 ,B_size=0 ;
   for(int i=0 ; i<vc_size ;i++)
   {
     if(vc[i].in_A)
     {
          A_count++;
          A_size += vc[i].size;
     }
     else
     {
         B_count++; 
         B_size +=  vc[i].size;
     }
   }
   cout<<"A area: "<<A_size<<endl;
   cout<<"B area: "<<B_size<<endl;
   cout<<"Cut size: "<<cut_size<<endl<<endl;
   
   ofstream fout(output_name);
   fout<<"cut_size "<<cut_size<<endl;
   fout<<"A "<<A_count<<endl;
   for(int i=0 ; i<vc_size ;i++)
   {
     if(vc[i].in_A)
        fout<<vc[i].name<<endl;   
   }
   fout<<"B "<<B_count<<endl;
   for(int i=0 ; i<vc_size ;i++)
   {
     if(!vc[i].in_A)
        fout<<vc[i].name<<endl;   
   }
  
}
int main(int argc, char **argv)
{   
    if (argc != 4) {
        exit(1);
    }
    input_cell = argv[1];
    input_net = argv[2];
    output_name = argv[3];
    ifstream c_fin(input_cell);
    ifstream n_fin(input_net);

    clock_t start, stop;
    double time1, time2, time3, time4, time5;
    start=clock();
        string str;
        int sz, i = 0;
        while(c_fin>>str>>sz)
        {
            Cell temp;
            temp.name=str;
            temp.size=sz;
            vc.push_back(temp);
            cell_map_name_id.insert(pair<string, int>(temp.name, i));
            i++;
        }
    
        while(n_fin>>str && str=="NET" )
        {
            Net temp;
            n_fin>>str;
            temp.name=str;
            n_fin>>str;
            while(n_fin>>str && str != "}")
            {
                temp.cells_name.push_back(str);
                temp.cells_id.push_back(cell_map_name_id[str]);
            }
            vn.push_back(temp);
        }
        vc_size=vc.size();
        vn_size=vn.size();
    stop=clock();
    time1 = double(stop-start);
    
    start=clock();
        init_partition();
    stop=clock();
    time2 = double(stop-start);

    
    start=clock();
        find_connect_net();
        countCutSize();
        countConstraint();
        countPmax();
    stop=clock();
    time3 = double(stop-start);
    cout<<"Pmax: "<<Pmax<<endl;
    cout<<"Constraint: "<<Constraint<<endl;
    
    start=clock();
        FM_algorithm();
    stop=clock();
    time4 = double(stop-start);
    
    start=clock();
        recover();
        cut_size = cut_size - max_total_gain ;
        output();
    stop=clock();
    time5 = double(stop-start);
    
    cout<<"Parse Data                         : "<<time1/(CLOCKS_PER_SEC/1000)<<" millisec"<<endl;
    cout<<"Initial Partition                  : "<<time2/(CLOCKS_PER_SEC/1000)<<" millisec"<<endl;
    cout<<"Count Cut Size & Constraint & Pmax : "<<time3/(CLOCKS_PER_SEC/1000)<<" millisec"<<endl;
    cout<<"FM Algorithm                       : "<<time4/(CLOCKS_PER_SEC/1000)<<" millisec"<<endl;
    cout<<"Recovery & Output                  : "<<time5/(CLOCKS_PER_SEC/1000)<<" millisec"<<endl;

    return 0;
}

