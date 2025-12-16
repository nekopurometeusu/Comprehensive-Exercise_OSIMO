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
                int months,
                int totalcost)
{
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
        cout << "データを登録しました\n";
    }
}

// ======================================
// リワード表示（今回分の金額）
// ======================================
void showReward(sqlite3* db, int totalcost)
{
    sqlite3_stmt* stmt;
    string sql =
        "SELECT amount, description "
        "FROM rewards "
        "WHERE amount <= ? "
        "ORDER BY amount DESC "
        "LIMIT 1;";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cout << "SQL準備エラー: " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_int(stmt, 1, totalcost);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int amount = sqlite3_column_int(stmt, 0);
        const char* description =
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        cout << "\n=== 今回の金額でできること ===\n";
        cout << "今回 " << totalcost << "円 → "
             << amount << "円相当："
             << description << endl;
    } else {
        cout << "\n今回の金額では該当するリワードがありません。\n";
    }

    sqlite3_finalize(stmt);
}

// ======================================
// main
// ======================================
int main()
{
    sqlite3* db = nullptr;

    // DBを開く
    if (sqlite3_open("tobacco.db", &db) != SQLITE_OK) {
        cout << "データベースを開けません: "
             << sqlite3_errmsg(db) << endl;
        return 1;
    }
    cout << "SQLite データベースを開きました\n";

    // ----------------------------------
    // 銘柄一覧取得
    // ----------------------------------
    vector<Brand> brandList;
    sqlite3_stmt* stmt;
    string sql = "SELECT id, brand, price FROM brands;";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Brand b;
            b.id = sqlite3_column_int(stmt, 0);
            b.brand =
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            b.price = sqlite3_column_int(stmt, 2);
            brandList.push_back(b);
        }
    }
    sqlite3_finalize(stmt);

    // ----------------------------------
    // 銘柄表示
    // ----------------------------------
    cout << "\n銘柄を選択してください\n";
    for (const auto& b : brandList) {
        cout << b.id << "："
             << b.brand << "（" << b.price << "円）\n";
    }

    // ----------------------------------
    // ユーザー入力
    // ----------------------------------
    int choice;
    cout << "\nchoice 入力待ち… ";
    cin >> choice;

    Brand selected;
    bool found = false;
    for (const auto& b : brandList) {
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

    int per_day, years, months;
    cout << "1日に吸う本数: ";
    cin >> per_day;
    cout << "喫煙歴（年）: ";
    cin >> years;
    cout << "喫煙歴（月）: ";
    cin >> months;

    // ----------------------------------
    // 今回分の合計金額を計算
    // ----------------------------------
    int totalcost =
        selected.price * per_day * (years * 12 + months);

    // ----------------------------------
    // DB登録
    // ----------------------------------
    insertData(db,
               selected.brand,
               selected.price,
               per_day,
               years,
               months,
               totalcost);

    // ----------------------------------
    // 今回分のリワード表示
    // ----------------------------------
    showReward(db, totalcost);

    sqlite3_close(db);
    return 0;
}