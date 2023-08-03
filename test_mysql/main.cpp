#include<iostream>
#include"MySQLLibrary.h"
using namespace std;
using namespace MyStd::MySQL;
void ShowQueryResult(const QueryResult& result)
{
    for (const string& name : result.GetFields())
    {
        cout.width(20);
        cout << name;
    }
    cout << endl;
    for (const QueryRow& row : result.GetRows())
    {
        for (const string& name : row.GetData())
        {
            cout.width(20);
            cout << name;
        }
        cout << endl;
    }
}
int main()
{
    try
    {
        DataBase db("root", "123456", "test", "gbk");
        cout << db.GetDataBaseName() << endl;

        db.Execute("insert into name (id,name) values(2,'hahahah')");
//        ShowQueryResult(db.Execute("select * from name").value());
//        db.Execute("use  information_schema");
//        cout << db.GetDataBaseName() << endl;
//        ShowQueryResult(db.Execute("show tables").value());
//        db.Execute("use mirai");//没有返回数据
    }
    catch (const MySQLException& e)
    {
        cout << e.what();
    }
    return 0;
}
