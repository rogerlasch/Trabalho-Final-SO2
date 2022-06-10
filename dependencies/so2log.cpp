


#include<iostream>
#include<fstream>
#include<mutex>
#include"so2log.h"

using namespace std;

void log_write(const std::string& s)
{
static mutex mtx;
static ofstream ofn("default logfile.txt");
unique_lock<mutex> lck(mtx);
ofn<<s<<endl;
cout<<s<<endl;
}
