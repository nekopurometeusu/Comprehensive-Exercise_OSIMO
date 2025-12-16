#include <iostream>
#include <string>
#include <vector>
#include <sqlite3.h>
using namespace std;

// ======================================
// 銘柄情報を保持する構造体
// ======================================
struct Brand {
    int id;
    string brand;
    int price;
};

// ======================================
// smokingテーブルへデータ挿入
// ======================================
void insertData(sqlite3 *db,
                const string& brand,
                int price,
                int per_day,
                int years,
                int months)
{
    int totalcost = price * per_day * (years * 12 + months);

    string sql =
        "INSERT INTO smoking "
        "(brand, price, per_day, years, months, total_cost, memo) "
        "VALUES ('" + brand + "', " +
        to_string(price) + ", " +
        to_string(per_day) + ", " +
        to_string(years) + ", " +
        to_string(months) + ", " +
        to_string(totalcost) + ", '');";

    char* errmsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errmsg);

    if (rc != SQLITE_OK) {
        cout << "データ挿入エラー: " << errmsg << endl;
        sqlite3_free(errmsg);
    } else {
        cout << "データを登録しました" << endl;
    }
}

void showReward(sqlite3* db, int totalcost)
{
    sqlite3_stmt* stmt;
    string sql = "SELECT amount, description "
                 "FROM rewards "
                 "WHERE amount <= ? "
                 "ORDER BY amount DESC "
                 "LIMIT 1;";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "SQL準備エラー: " << sqlite3_errmsg(db) << endl;
        return;
    }

    // totalcost をバインド
    sqlite3_bind_int(stmt, 1, totalcost);

    // 実行して結果を取得
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int amount = sqlite3_column_int(stmt, 0);
        const char* description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        cout << "\n=== この金額でできること ===\n";
        cout << "累計 " << amount << "円 で: " << description << endl;
    } else {
        cout << "該当するリワードはありません。\n";
    }

    sqlite3_finalize(stmt);
}

// ======================================
// main
// ======================================
int main()
{
    sqlite3* db = nullptr;

    // -----------------------------
    // DBを開く
    // -----------------------------
    int rc = sqlite3_open("tobacco.db", &db);
    if (rc != SQLITE_OK) {
        cout << "データベースを開けません: " << sqlite3_errmsg(db) << endl;
        return 1;
    }
    cout << "SQLite データベースを開きました\n";

    // -----------------------------
    // brandsテーブルから一覧を取得
    // -----------------------------
    vector<Brand> brandList;

    sqlite3_stmt* stmt;
    string sql = "SELECT id, brand, price FROM brands;";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Brand b;
            b.id    = sqlite3_column_int(stmt, 0);
            b.brand  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            b.price = sqlite3_column_int(stmt, 2);

            brandList.push_back(b);
        }
    }
    sqlite3_finalize(stmt);

    // -----------------------------
    // 銘柄一覧を表示
    // -----------------------------
    cout << "銘柄を選択してください\n";
    for (size_t i = 0; i < brandList.size(); i++) {
        cout << brandList[i].id << "："
             << brandList[i].brand << "（" << brandList[i].price << "円）\n";
    }

    // -----------------------------
    // ユーザーに選択させる
    // -----------------------------
    int choice;
    cout << "choice 入力待ち… ";
    cin >> choice;

    // 選択された銘柄を探す
    Brand selected;
    bool found = false;

    for (auto &b : brandList) {
        if (b.id == choice) {
            selected = b;
            found = true;
            break;
        }
    }

    if (!found) {
        cout << "無効な選択です。\n";
        sqlite3_close(db);
        return 1;
    }

    // -----------------------------
    // 喫煙情報の入力
    // -----------------------------
    int per_day, years, months;
    cout << "1日に吸う本数: ";
    cin >> per_day;

    cout << "喫煙歴（年）: ";
    cin >> years;

    cout << "喫煙歴（月）: ";
    cin >> months;

    // -----------------------------
    // smokingテーブルへINSERT
    // -----------------------------
    insertData(db,
               selected.brand,
               selected.price,
               per_day,
               years,
               months);

    // -----------------------------
    // 登録済みデータ表示
    // -----------------------------
    cout << "\n=== 現在の登録データ ===\n";
    sql = "SELECT * FROM smoking;";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            cout << "ID: " << sqlite3_column_int(stmt, 0)
                 << ", Brand: " << sqlite3_column_text(stmt, 1)
                 << ", TotalCost: " << sqlite3_column_int(stmt, 6)
                 << endl;
            
        }
        int sumTotal = 0;
        sql = "SELECT SUM(total_cost) FROM smoking;";
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                sumTotal = sqlite3_column_int(stmt, 0);
            }
        }
        sqlite3_finalize(stmt);

        showReward(db, sumTotal);
    }   
    sqlite3_finalize(stmt);

    sqlite3_close(db);
    return 0;
}