//
// Created by mim on 23-8-2.
//

#ifndef TEST_MYSQL_MYSQLLIBRARY_H
#define TEST_MYSQL_MYSQLLIBRARY_H

#pragma once

//#include<Windows.h>
#include<mysql/mysql.h>
#include<string>
#include<stdexcept>
#include<ios>
#include<vector>
//#include<format>
#include<optional>
#include <sstream>

using namespace std;

namespace MyStd {
    bool IgnoreCaseCompare(const std::string &a, const std::string &b) noexcept {
        if (a.size() != b.size())
            return false;
        for (size_t i = 0; i < a.size(); i++) {
            if (tolower(a[i]) != tolower(b[i]))
                return false;
        }
        return true;
    };
    namespace MySQL {
        class MySQLException : public std::ios_base::failure {
        public:
            MySQLException(const std::string &message);
        };

        class MySQLConnectException : public MySQLException {
        public:
            MySQLConnectException(const std::string &message);
        };

        class MySQLExecuteException : public MySQLException {
        public:
            MySQLExecuteException(const std::string &message);
        };

        class MySQLFieldNotFoundException : public MySQLException {
        public:
            MySQLFieldNotFoundException(const std::string &message);
        };

        class MySQLEscapeException : public MySQLException {
        public:
            MySQLEscapeException(const std::string &message);
        };

        class QueryResult;

        class QueryRow {
        public:
            QueryRow(const QueryResult &result, MYSQL_ROW row);

            QueryRow(const QueryResult &newres, QueryRow &&row);

            const std::vector<std::string> &GetData() const noexcept;

            std::string operator[](const std::string &index) const;

        private:
            std::vector<std::string> data;
            const QueryResult &res;
        };

        class QueryResult {
        public:
            QueryResult(MYSQL_RES &&result);

            QueryResult(QueryResult &&result);

            const std::vector<QueryRow> &GetRows() const noexcept;

            const std::vector<std::string> &GetFields() const noexcept;

            bool IsEmpty() const noexcept;

            ~QueryResult();

        private:
            std::vector<QueryRow> rows;
            std::vector<std::string> fields;
        };

        class DataBase {
        public:
            DataBase(const std::string &user_name, const std::string &password, const std::string &database,
                     const std::string &characters, const std::string &host = "localhost",
                     const unsigned int port = 3306, const char *unix_socket = nullptr,
                     const unsigned long client_flag = 0);

            std::optional<QueryResult> Execute(const std::string &str);

            std::string GetDataBaseName() const noexcept;

            std::string EscapeString(const std::string &str);

            ~DataBase();

        private:
            MYSQL db;
        };

        DataBase::DataBase(const std::string &user_name, const std::string &password, const std::string &database,
                           const std::string &characters, const std::string &host, const unsigned int port,
                           const char *unix_socket, const unsigned long client_flag) {
            mysql_init(&db);
            if (!mysql_real_connect(&db, host.data(), user_name.data(), password.data(), database.data(), port,
                                    unix_socket, client_flag))
                throw MySQLConnectException(mysql_error(&db));
            //设置访问编码
            mysql_set_character_set(&db, characters.data());
        }

        std::optional<QueryResult> DataBase::Execute(const std::string &str) {
            if (mysql_real_query(&db, str.data(), str.size()))
                throw MySQLExecuteException(mysql_error(&db));
            MYSQL_RES *result = mysql_store_result(&db);
            if (result) {
                return QueryResult(std::move(*result));
            }
            if (mysql_field_count(&db) == 0)//无返回数据，不是查询语句
            {
                return std::nullopt;
            }
            throw MySQLExecuteException(mysql_error(&db));
        }

        std::string DataBase::GetDataBaseName() const noexcept {
            return db.db;
        }

        std::string DataBase::EscapeString(const std::string &str) {
            char *temp = new char[str.size() * 2 + 1];
            const unsigned long ret = mysql_real_escape_string(&db, temp, str.data(), str.size());
            if (ret == -1)
                throw MySQLEscapeException("格式化出现错误！");
            return std::string(temp, ret);
        }

        DataBase::~DataBase() {
            mysql_close(&db);
        }

        MySQLException::MySQLException(const std::string &message) : std::ios::failure(message) {
        }

        MySQLConnectException::MySQLConnectException(const std::string &message) : MySQLException(message) {
        }

        MySQLExecuteException::MySQLExecuteException(const std::string &message) : MySQLException(message) {
        }

        QueryResult::QueryResult(MYSQL_RES &&result) {
            //处理列
            MYSQL_FIELD *fs = mysql_fetch_fields(&result);
            const unsigned int field_count = mysql_num_fields(&result);
            fields.reserve(field_count);
            for (unsigned int i = 0; i < field_count; ++i) {
                fields.push_back(fs[i].name);
            }
            //处理行
            MYSQL_ROW row;
            while (row = mysql_fetch_row(&result)) {
                rows.emplace_back(*this, row);
            }
            mysql_free_result(&result);
        }

        QueryResult::QueryResult(QueryResult &&result) : fields(std::move(result.fields)) {
            rows.reserve(result.rows.size());
            for (auto &&row: result.rows) {
                rows.emplace_back(*this, std::move(row));
            }
        }

        const std::vector<QueryRow> &QueryResult::GetRows() const noexcept {
            return rows;
        }

        const std::vector<std::string> &QueryResult::GetFields() const noexcept {
            return fields;
        }

        inline bool QueryResult::IsEmpty() const noexcept {
            return rows.empty();
        }

        QueryResult::~QueryResult() {
        }

        QueryRow::QueryRow(const QueryResult &result, MYSQL_ROW row) : res(result) {
            for (size_t i = 0; i < res.GetFields().size(); ++i) {
                if (row[i])
                    data.push_back(row[i]);
                else
                    data.push_back("");
            }
        }

        QueryRow::QueryRow(const QueryResult &newres, QueryRow &&row) : res(newres), data(std::move(row.data)) {
        }

        const std::vector<std::string> &QueryRow::GetData() const noexcept {
            return data;
        }

        std::string QueryRow::operator[](const std::string &index) const {
            for (size_t i = 0; i < res.GetFields().size(); ++i) {
                if (IgnoreCaseCompare(res.GetFields()[i], index))
                    return data[i];
            }
            std::ostringstream buffer;
            buffer << "未找到名为" << index << "的列";
            throw MySQLFieldNotFoundException(buffer.str());
        }

        MySQLFieldNotFoundException::MySQLFieldNotFoundException(const std::string &message) : MySQLException(message) {
        }

        MySQLEscapeException::MySQLEscapeException(const std::string &message) : MySQLException(message) {
        }
    }
}


#endif //TEST_MYSQL_MYSQLLIBRARY_H
