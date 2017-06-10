#include <pthread.h>
#include <fstream>
#include <math.h>
#include <vector>
#include <set>
#include <unistd.h>
#include <iostream>
#include <algorithm>
using namespace std;

vector< set<char> > info;
vector< set<char> > f_sets;
vector< set<char> > c_sets;
vector< set<char> > p_sets;
set< char > temp;
set< char > remp;
vector<int> occupy;
vector<int> supp_vec;
vector<int> p_supp_vec;
int size,flag;
char array[15];

void permutations(char array[],string construct,int n,int k) 
{
    if(k==0) 
    {
    	flag=1;
    	temp.clear();
    	for(int j=0;j<construct.length();j++)
    	{
    		temp.insert(construct[j]);
    	}
    	if(temp.size()==construct.length())
    	{
    		for(int j=0;j<construct.length()-1;j++)
    		{
    			if(construct[j]>construct[j+1])
    			{
    				flag=0;
    				break;
    			}
    		}
    		if(flag==1)
    		{
    			for(std::set<char>::iterator it=temp.begin(); it!=temp.end(); ++it)
				{
    				c_sets[size].insert(*it);
    			}
    			size++;
    		}	
    	}
        return;
    }

    for(int i=0;i<n;i++)
    {	
        string new_construct = construct + array[i]; 
        permutations(array,new_construct,n,k-1); 
    }
}

void pairs(int s,int k) 
{
    for(int i=0;i<size;i++)
    {
    	for(std::set<char>::iterator it=f_sets[i].begin(); it!=f_sets[i].end(); ++it)
    	{
    		remp.insert(*it);
    	}
    }

    int j=0;
    for(std::set<char>::iterator it=remp.begin(); it!=remp.end(); ++it)
	{
		array[j]=*it;
		j++;
	}
    size=0;
    permutations(array,"",remp.size(),k);
}

void *worker(void *arg)
{
	long tid,i,j,begin,no_of_trans,count;
    tid = (long)arg;
    no_of_trans=occupy[tid];
  
	//begin
	begin=0;
	for(i=0;i<tid;i++)
	{
		begin=begin+occupy[i];
	}

	//operation
	for(i=0;i<size;i++)
	{
		count=0;
		for(j=begin;j<begin+no_of_trans;j++)
		{
			if(includes(info[j].begin(),info[j].end(),c_sets[i].begin(),c_sets[i].end()))
			{
				count++;
			}
		}
		supp_vec[i]+=count;
	}
	
	pthread_exit(NULL);
}

int main()
{
	cout<<"***APRIORI ALGORITHM***\n\n";
	int i,j,k,m,max,no_of_trans=0,no_of_threads=0,check,value,remainder;
	double support;
	string trans;
	ifstream input;

	//counting number of transactions and inserting the items
	input.open("Transactions.txt");
	no_of_trans=0;
	if(input.is_open())
	{
		while(getline(input,trans))
		{
			no_of_trans++;
			for(i=0;i<trans.length();i++)
			{
				if(trans[i]!=' ')
				{
					temp.insert(trans[i]);
				}
			}
		}
	}
	input.close();

	//input
	cout<<"Number of transactions: "<<no_of_trans<<endl;
	cout<<"Enter support percentage : ";
	cin>>support;
	support=ceil((support*no_of_trans)/100);
	cout<<"Therefore, your minimum support value: "<<support<<endl;
	do
	{
	if(no_of_threads>no_of_trans)
	{
		cout<<"INVALID NUMBER OF THREADS!"<<endl;
	}
	cout<<"Enter the number of threads: ";
	cin>>no_of_threads;
	}
	while(no_of_threads>no_of_trans);

	//divide
	if(no_of_trans%no_of_threads==0)
	{
		value=no_of_trans/no_of_threads;
		for(i=0;i<no_of_threads;i++)
		{
			occupy.push_back(value);
		}
	}
	else
	{
		value=no_of_trans/no_of_threads;
		remainder=no_of_trans%no_of_threads;
		for(i=0;i<no_of_threads;i++)
		{	
			if(remainder>0)
			{
				occupy.push_back(value+1);
				remainder--;
			}
			else
			{
				occupy.push_back(value);
			}
		}   
	}

	//reading file to vector
	info.resize(no_of_trans);
	input.open("Transactions.txt");
	if(input.is_open())
	{
		j=0;
		while(getline(input,trans))
		{
			for(i=0;i<trans.length();i++)
			{
				if(trans[i]!=' ')
				{
					info[j].insert(trans[i]);
				}
			}
			j++;
		}
	}
	input.close();

	//initialize
	c_sets.resize(100);
	f_sets.resize(100);
	i=0;
	for(std::set<char>::iterator it=temp.begin(); it!=temp.end(); ++it)
	{
		c_sets[i].insert(*it);
		i++;
	}
	size=temp.size();
	k=1;
	m=2;

	//operations
	while(k!=0)
	{
		k=0;
		supp_vec.clear();
		supp_vec.resize(50,0);
		//threads
		pthread_t threads[no_of_threads];
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		for(i=0;i<no_of_threads;i++)
		{
			check=pthread_create(&threads[i],&attr,&worker,(void *)i);
			if(check!=0) 
			{
	            printf("Error: pthread_create() failed\n");
	            exit(-1);
	        }
	        check=pthread_join(threads[i],NULL); //join with a terminated thread
		}

		for(i=0;i<size;i++)
		{	
			if(supp_vec[i]<support)
			{
				c_sets.erase(c_sets.begin()+i);
				supp_vec.erase(supp_vec.begin()+i);
				size--;
				i--;
			}
			else
			{
				f_sets[k].insert(c_sets[i].begin(),c_sets[i].end());
				k++;
				max=k;
			}
		}

		if(k==0)
		{
			break;
		}

		p_sets.clear();
		p_sets.resize(100);

		//transfering content f_sets-->p_sets
		for(i=0;i<size;i++)
		{
			for(std::set<char>::iterator it=f_sets[i].begin(); it!=f_sets[i].end(); ++it)
	    		p_sets[i].insert(*it);
		}

		p_supp_vec.clear();
		p_supp_vec.resize(50);
		for(i=0;i<supp_vec.size();i++)
		{
			p_supp_vec[i]=supp_vec[i];
		}

		remp.clear();
		c_sets.clear();
		c_sets.resize(100);

		//combinations
		pairs(size,m);
		m++;
		f_sets.clear();
		f_sets.resize(100);

		if(size==0)
		{
			break;
		}
	}

	cout<<"\nANSWER :"<<endl;
	cout<<"      Combination\t\t\tSupport"<<endl;
	cout<<"---------------------------------------------------"<<endl;
	for(i=0;i<max;i++)
	{
		  cout<<"\t{ ";
		  for(std::set<char>::iterator it=p_sets[i].begin(); it!=p_sets[i].end(); ++it)
    		  std::cout<<*it<<" ";
    	  cout<<"}\t\t|\t   "<<p_supp_vec[i]<<endl;
	}
	cout<<"---------------------------------------------------"<<endl;
}

//TRANSACTIONS
/*
1 3 4
2 3 5
1 2 3 5
2 5
1 3 5

40%
*/
/*
1 2 3
1 3
1 4
2 5 6

40%
*/
/*
1 3 4
2 3 5
1 2 3 5
2 5

50%
*/
/*
1 2 5
2 4
2 3
1 2 4
1 3
2 3
1 3
1 2 3 5
1 2 3

22%
*/
/*
1 2 3 4
1 2 4
1 2
2 3 4
2 3
3 4
2 4

20%
*/
/*
A B C
A C
A D
B E F

40%
*/
