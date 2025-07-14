#include "usersqlite.h"

SqliteModel::SqliteModel(std::string file)
{
    rc = sqlite3_open(file.c_str(), &sq);

    if (rc)
    {
        std::cout << "打开数据库失败" << std::endl;
        return;
    }
    errmsg = nullptr;
    rc = sqlite3_exec(sq, "PRAGMA journal_mode=WAL;", nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK)
    {
        std::cerr << "设置WAL失败: " << errmsg << std::endl;
        sqlite3_free(errmsg);
    }
}
bool SqliteModel::createTable(std::string type, int id)
{
    std::string tablename = type + std::to_string(id);

    // 创建表 SQL
    char *createsql = sqlite3_mprintf(
        "CREATE TABLE IF NOT EXISTS %q (id INTEGER PRIMARY KEY, user INTEGER,username TEXT, msg TEXT, msgtime INTEGER);",
        tablename.c_str());
    rc = sqlite3_exec(sq, createsql, 0, 0, &errmsg);
    sqlite3_free(createsql);

    if (rc != SQLITE_OK)
    {
        std::cerr << "创建表失败: " << errmsg << std::endl;
        
        sqlite3_free(errmsg);
        return false;
    }
    const char *fields[] = {"username", "msg", "msgtime"};
    for (const auto &field : fields)
    {
        std::string indexName = "idx_" + tablename + "_" + field;
        char *indexsql = sqlite3_mprintf(
            "CREATE INDEX IF NOT EXISTS %q ON %q (%q);",
            indexName.c_str(), tablename.c_str(), field);
        rc = sqlite3_exec(sq, indexsql, 0, 0, &errmsg);
        sqlite3_free(indexsql);

        if (rc != SQLITE_OK)
        {
            std::cerr << "创建索引失败: " << errmsg << std::endl;
            sqlite3_free(errmsg);
            return false;
        }
    }

    return true;
}
bool SqliteModel::insertTable(std::string name, int userid, std::string username, std::string msg, uint64_t timestamp)
{
    std::string sql = "INSERT INTO " + name + " (user, username, msg, msgtime) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;

    rc = sqlite3_prepare_v2(sq, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "预处理失败: " << sqlite3_errmsg(sq) << std::endl;
        
        return false;
    }

    sqlite3_bind_int(stmt, 1, userid);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, msg.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 4, timestamp);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        std::cerr << "插入失败: " << sqlite3_errmsg(sq) << std::endl;
         if (stmt)
            sqlite3_finalize(stmt); // 尽早清理
        return false;
    }
    return true;
}
bool SqliteModel::deleteTable(std::string name)
{
    std::string sql = "DROP TABLE IF EXISTS " + name + ";";
    rc = sqlite3_exec(sq, sql.c_str(), 0, 0, &errmsg);
    if (rc != SQLITE_OK)
    {
        std::cerr << "删除表失败: " << errmsg << std::endl;
         
        sqlite3_free(errmsg);
        return false;
    }
    return true;
}
bool SqliteModel::deleteMessage(std::string name, int userid, std::string username, std::string msg, uint64_t timestamp)
{
    std::string sql = "DELETE FROM " + name + " WHERE msg = ? AND msgtime = ?;";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(sq, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "预处理失败: " << sqlite3_errmsg(sq) << std::endl;
         if (stmt)
            sqlite3_finalize(stmt); // 尽早清理
        return false;
    }

    sqlite3_bind_text(stmt, 1, msg.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, timestamp);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        std::cerr << "删除消息失败: " << sqlite3_errmsg(sq) << std::endl;
         if (stmt)
            sqlite3_finalize(stmt); // 尽早清理
        return false;
    }
    return true;
}
std::vector<fixbug::Msg> SqliteModel::queryTable(std::string name)
{
    std::vector<fixbug::Msg> results;
    std::string sql = "SELECT user, username, msg, msgtime FROM " + name + " ORDER BY msgtime ASC;";
    sqlite3_stmt *stmt;

    rc = sqlite3_prepare_v2(sq, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "查询预处理失败: " << sqlite3_errmsg(sq) << std::endl;
         if (stmt)
            sqlite3_finalize(stmt); // 尽早清理
        return results;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        fixbug::Msg m;
        m.set_userid(sqlite3_column_int(stmt, 0));
        m.set_username((const char *)sqlite3_column_text(stmt, 1));
        m.set_msg((const char *)sqlite3_column_text(stmt, 2));
        m.set_time(sqlite3_column_int64(stmt, 3));
        results.push_back(m);
    }

    sqlite3_finalize(stmt);
    return results;
}
std::vector<fixbug::Msg> SqliteModel::queryTable(std::string name, int userid, std::string username, std::string msg, uint64_t timestamp)
{
    std::vector<fixbug::Msg> results;
    std::string sql = "SELECT user, username, msg, msgtime FROM " + name + " WHERE user=? AND username=? AND msg=? AND msgtime=?;";
    sqlite3_stmt *stmt = nullptr;
    rc = sqlite3_prepare_v2(sq, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "查询预处理失败: " << sqlite3_errmsg(sq) << std::endl;
        if (stmt)
            sqlite3_finalize(stmt); // 尽早清理
        return results;
    }

    sqlite3_bind_int(stmt, 1, userid);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, msg.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 4, timestamp);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        fixbug::Msg m;
        m.set_userid(sqlite3_column_int(stmt, 0));
        m.set_username((const char *)sqlite3_column_text(stmt, 1));
        m.set_msg((const char *)sqlite3_column_text(stmt, 2));
        m.set_time(sqlite3_column_int64(stmt, 3));
        results.push_back(m);
    }

    sqlite3_finalize(stmt);
    return results;
}
std::vector<fixbug::Msg> SqliteModel::queryTable(std::string name, std::string msg)
{
    // 查询该表的关键字msg的所有消息
    std::vector<fixbug::Msg> results;
    std::string sql = "SELECT user, username, msg, msgtime FROM " + name + " WHERE msg LIKE ?;";
    sqlite3_stmt *stmt;

    rc = sqlite3_prepare_v2(sq, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "查询预处理失败: " << sqlite3_errmsg(sq) << std::endl;
         if (stmt)
            sqlite3_finalize(stmt); // 尽早清理
        return results;
    }

    std::string pattern = "%" + msg + "%"; // 模糊匹配
    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        fixbug::Msg m;
        m.set_userid(sqlite3_column_int(stmt, 0));
        m.set_username((const char *)sqlite3_column_text(stmt, 1));
        m.set_msg((const char *)sqlite3_column_text(stmt, 2));
        m.set_time(sqlite3_column_int64(stmt, 3));
        results.push_back(m);
    }

    sqlite3_finalize(stmt);
    return results;
}
SqliteModel::~SqliteModel()
{
    if (sq)
    {
        int rc = sqlite3_close(sq);
        if (rc != SQLITE_OK)
        {
            std::cerr << "数据库关闭失败: " << sqlite3_errmsg(sq) << std::endl;
        }
        else
        {
            std::cout << "数据库已关闭。" << std::endl;
        }
    }
}