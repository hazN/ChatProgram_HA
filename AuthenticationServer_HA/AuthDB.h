#pragma once
#include "jdbc/mysql_driver.h"
#include "jdbc/mysql_connection.h"
#include "jdbc/mysql_error.h"
#include "jdbc/cppconn/statement.h"
#include "jdbc/cppconn/prepared_statement.h"
#include "jdbc/cppconn/resultset.h"
#include  <chrono>
#include "AuthHelper.h"
class AuthDB
{
public:
	bool Connect();
	void Disconnect();
	bool CreateAccount(const char* email, const std::string* salt, const char* password, int* error, int* userId);
	bool AuthenticateAccount(const char* email, const char* password, int* error, AuthHelper SERVER, int* userId, std::string* creation_date);
private:
	sql::Driver* m_pDriver;
	sql::Connection* m_pConnection;
	sql::ResultSet* m_pResultSet;
	sql::Statement* m_pStatement;
	sql::PreparedStatement* m_pInsertStatement;
};

enum AUTHFAILURE
{
	ACCOUNT_ALREADY_EXISTS = 1,
	INVALID_PASSWORD = 2,
	INTERNAL_SERVER_ERROR = 3,
	INVALID_CREDENTIALS = 4,
};