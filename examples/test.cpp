#include<iostream>  
#include<sstream>        //istringstream 必须包含这个头文件
#include<string>  
using namespace std;  
int main()  
{  
    string str="中国_ns 贸易_v 数据_n 疲弱_a ，_wp 亚太区_ns 股市_n 午後_n 普遍_a 向_p 下_nd ，_wp 尤其_d 是_v 澳_j 股_n 跌_v 1.6%_m 至_p 5_m ,_wp 129_m 。_wp ";  
    istringstream is(str);  
    string s;  
    while(is>>s)  
    {  
        int index=s.find("_");
        cout<<s.substr(0,index)<<s.substr(index+1)<<endl;  
    }  
      
} 