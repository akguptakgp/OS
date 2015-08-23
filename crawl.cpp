#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <string>
#include <list>         
#include <curl/curl.h>
#include <htmlcxx/html/ParserDom.h>
#include <fstream>
#include <iomanip>      


using namespace std;
using namespace htmlcxx;
using namespace HTML;
ofstream fout,uout;
	

pthread_mutex_t to_do_mutex,level_mutex,print_mutex,to_do_nxt_mutex;//,done_mutex;
pthread_barrier_t barrier1,barrier2;

int URL_Level=1,MAX_LEVEL=3;
list<string> to_do,to_do_next;
map< string, pair<int,int > > done_list;


void *thread_function(void* threadno);
void parse(string);
void download_and_parse(string);


int main(int argc,char* argv[])
{

	if(argc==2) {MAX_LEVEL=atoi(argv[1]);}

	fout.open("all.txt");
	uout.open("crawl.txt");
	string master_url("http://cse.iitkgp.ac.in");
	pthread_t thread1[5];

	pthread_mutex_init(&print_mutex,NULL);
	pthread_mutex_init(&level_mutex,NULL);
	pthread_mutex_init(&to_do_nxt_mutex,NULL);
	pthread_mutex_init(&to_do_mutex,NULL);
    pthread_barrier_init (&barrier1, NULL,5);
    pthread_barrier_init (&barrier2, NULL,5);
    to_do.push_back(master_url);
	
    for(int i=1;i<6;i++)
    {
            int  iret = pthread_create( thread1+i-1, NULL,thread_function,(void*)i);
            if(iret){
                fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);        
                exit(EXIT_FAILURE);    
            }
            
    }
    
    
    for(int i=0;i<5;i++)
    {
        pthread_join( thread1[i], NULL);      
    }

    cout<<"URL's crawled and remaining to crawl are:"<<endl;
    cout<<setw(15)<<"<depth>"<<setw(15)<<"<thread id>"<<setw(30)<<"<URL>"<<endl;
    for(map<string,pair<int,int> >::iterator it=done_list.begin();it!=done_list.end();it++)
    {
    	cout<<setw(15)<<it->second.second<<setw(15)<<it->second.first<<setw(10)<<"\""<<it->first<<"\""<<endl;
    }
    for(list<string>::iterator it=to_do.begin();it!=to_do.end();it++)
    {
    	cout<<setw(15)<<MAX_LEVEL<<setw(15)<<0<<setw(10)<<"\""<<*it<<"\""<<endl;
    }
    pthread_mutex_destroy(&to_do_nxt_mutex);
 	pthread_mutex_destroy(&to_do_mutex);
 	pthread_mutex_destroy(&print_mutex);
    pthread_mutex_destroy(&level_mutex);
    pthread_barrier_destroy(&barrier1);
    pthread_barrier_destroy(&barrier2);
 
    
    return 0;
}

void *thread_function(void* threadno)
{	   
	long tid;

	long i=(long)threadno;

	pthread_mutex_lock( &print_mutex);
    cout<<"thread "<<i<<" starting"<<endl;
    pthread_mutex_unlock( &print_mutex);
    
	// int p=0;
    while(1)
    {
		pthread_mutex_lock(&to_do_mutex);
    	if(to_do.empty())
    	{
    		pthread_mutex_unlock( &to_do_mutex);
    		pthread_mutex_lock( &print_mutex);
		    cout<<"Thread "<<i<<" found to_do list empty"<<endl;
		    pthread_mutex_unlock( &print_mutex);
    		
    		
    		pthread_barrier_wait (&barrier1);

    		if(i==1)
    		{
    			pthread_mutex_lock(&to_do_mutex);
    			swap(to_do_next,to_do);
    			pthread_mutex_lock( &level_mutex);
	    		URL_Level++;
	    		cout<<"new URL level= "<<URL_Level<<endl;
	    		uout<<endl;
	    		fout<<endl;
	  		  	pthread_mutex_unlock( &level_mutex);
	    		pthread_mutex_unlock( &to_do_mutex);
    		}
    		pthread_barrier_wait (&barrier2);

    		pthread_mutex_lock( &level_mutex);
    		bool flag=false;
	  		if(URL_Level==MAX_LEVEL) flag=true;
	  		pthread_mutex_unlock( &level_mutex);
	  		if(flag) break;
    	}
    	else
    	{
    		string url=to_do.front();
    		pthread_mutex_lock( &print_mutex);
	    	cout<<"Dequeing URL \""<<url<<"\" from to_do queue by the thread "<<i<<endl;
			pthread_mutex_unlock( &print_mutex);
			to_do.pop_front();
			pair<map<string,pair<int,int> >::iterator,bool> ret;
			
			ret =done_list.insert(make_pair(url,make_pair(i,URL_Level)));	
			if(ret.second==false) 
			{
	    		pthread_mutex_unlock( &to_do_mutex);
	    		pthread_mutex_lock( &print_mutex);
	    		cout<<"ignoring duplicate URL "<<url<<"by the thread "<<i<<endl;
	    		pthread_mutex_unlock( &print_mutex);
	  		}
	  		else
	  		{
	  			uout<<url<<endl;
	  			pthread_mutex_unlock( &to_do_mutex);	
	  			download_and_parse(url);
	  		}
	  	}
	}  
	cout<<"Thread "<<i<<" joining to main function"<<endl;
}

int writer(char *data, size_t size, size_t nmemb,string* writerData){
  if (writerData==NULL)
    return 0;
  writerData->append(data, size*nmemb);
  return size * nmemb;
}

void parse(string html)
{
	ParserDom parser;
  	tree<Node> dom = parser.parseTree(html);
  
	  //Print whole DOM tree
	  // cout << dom << endl;
	  
	  //Dump all links in the tree
	  tree<HTML::Node>::iterator it = dom.begin();
	  tree<HTML::Node>::iterator end = dom.end();
	  // int cnt=0;
	  for (; it != end; ++it)
	  {
	  	if (it->tagName() == "a")
	  	{
	  		it->parseAttributes();
	  		std::pair<bool, std::string> tmp=it->attribute("href");
	  		if(tmp.second.find("http")==0)
	  			{
	  			 // pthread_mutex_lock(&to_do_mutex);
	  			 // to_do_next.push(tmp.second);
	  			 // pthread_mutex_unlock(&to_do_mutex);
	  			}//cout <<tmp.second<<endl;}
	  		else{
	  			if(tmp.second.find("@")==-1 && tmp.second!=string("")){
	  			 fout<<"http://cse.iitkgp.ac.in/"<<tmp.second<<endl;
	  			 pthread_mutex_lock(&to_do_nxt_mutex);
	  			 to_do_next.push_back(string("http://cse.iitkgp.ac.in/"+tmp.second));
	  			 pthread_mutex_unlock(&to_do_nxt_mutex);
	  			 // if(cnt++==2) break;
	  			}
	  			}//cout<<"http://cse.iitkgp.ac.in/"<<tmp.second<<endl;
	  	}
	  }
	  // cout<<"request for parse done"<<endl;
	 // fout.close(); 
}


void download_and_parse(string url) 
{
	// cout<<"request for "<<url<<endl;
	CURL *curl;
	CURLcode res;
	string html;
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if(curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL,url.c_str());

		res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);

		res = curl_easy_setopt(curl, CURLOPT_WRITEDATA,&html);

		res = curl_easy_perform(curl);
		parse(html);
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
		
	}
	curl_easy_cleanup(curl);
	curl_global_cleanup();

   // cout<<"request for done"<<url<<endl;
	return;
}

