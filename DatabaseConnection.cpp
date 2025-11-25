#include<iostream>
#include<string>
#include<sqlite3.h>
using namespace std;

int exac_callback(
    void *data,
    int argc,
    char **argv,
    char **azColName   
)
{
    bool* exists = static_cast<bool*>(data);
    *exists = true;
    return 0;
}
void insertData(sqlite3 *db,
    const string& brand,
    int price,
    int per_day,
    int years,
    int months
)
{
    int totalcost = price * per_day * (years * 12 + months);

    string sql = 
        "INSERT INTO smoking (brand, price, per_day, years, months, total_cost, memo) "
        "VALUES ('"+ brand + "'," + 
        to_string(price) + "," +
        to_string(per_day) + "," +
        to_string(years) + ","+
        to_string(months) + "," +
        to_string(totalcost) +",'');";

    char* errmsg = nullptr;
    int rc = sqlite3_exec(db,sql.c_str(),nullptr,nullptr,&errmsg);

    if(rc != SQLITE_OK){
        cout<<"データ挿入エラー: "<<errmsg << endl;
        sqlite3_free(errmsg);
    }else{
            cout <<"データを登録しました"<< endl;
        }
}

int main(){
    sqlite3 *db = NULL;
    int rc = sqlite3_open("tobacco.db", &db);
    
    if(rc != SQLITE_OK){
        cout << "データベースを開けません: "<< sqlite3_errmsg(db) << endl;
        return 1;
    }
    cout << "SQLite データベースを開きました" << endl;

    cout << "銘柄を選択してください" << endl;
    cout << "１：セブンスター（600円）" << endl;
    cout << "２：メビウス（570円）" << endl;
    cout << "３：ウィンストン（540円）" << endl;
    cout << "４：アメリカンスピリット（420円）" << endl;
    cout << "５：ピース（600円）"<<endl;
    cout << "６：キャメル（450円）" << endl;
    cout << "７：ハイライト（520円）" << endl;
    cout << "８：マールボロ（600円）"<< endl;
    sqlite3_close(db);
    return 0;
    
}