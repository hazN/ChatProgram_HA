/*
	Hassan Assaf
	INFO-6016
	Project #2: Authentication Server
	Due 2022-11-09
*/
#include "AuthDB.h"
//https://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c
const std::string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);

	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

	return buf;
}

bool AuthDB::Connect()
{
	try
	{
		m_pDriver = sql::mysql::get_driver_instance();
	}
	catch (sql::SQLException e)
	{
		std::cout << "Failed to get_driver_instance: " << e.what() << std::endl;
		return false;
	}

	try
	{
		sql::SQLString hostName("127.0.0.1:3306");
		sql::SQLString username("default");
		sql::SQLString password("password");
		m_pConnection = m_pDriver->connect(hostName, username, password);
		m_pConnection->setSchema("authenticationdb");
	}
	catch (sql::SQLException e)
	{
		std::cout << "Failed to connect to our database: " << e.what() << std::endl;
		return false;
	}
	std::cout << "Connected to database..." << std::endl;
	return true;
}

void AuthDB::Disconnect()
{
	try
	{
		m_pConnection->close();
	}
	catch (sql::SQLException e)
	{
		std::cout << "Failed to close connection to our database: " << e.what() << std::endl;
		return;
	}
	std::cout << "Closed connection to database..." << std::endl;
}

bool AuthDB::CreateAccount(const char* email, const std::string* salt, const char* password, int* error, int* userId)
{
	// Insert into user table
	try {
		m_pStatement = m_pConnection->createStatement();
		m_pInsertStatement = m_pConnection->prepareStatement(
			"INSERT INTO user (creation_date) VALUES (?);");
	}
	catch (sql::SQLException e) {
		std::cout << "Failed to create statements: " << e.what() << std::endl;
		*error = INTERNAL_SERVER_ERROR;
		return false;
	}
	m_pInsertStatement->setString(1, currentDateTime());
	// Execute
	try {
		m_pInsertStatement->execute();
	}
	catch (sql::SQLException e) {
		std::cout << "Failed to add user to the database: " << e.what() << std::endl;
		*error = INTERNAL_SERVER_ERROR;
		return false;
	}

	// Get userId
	try {
		m_pResultSet = m_pStatement->executeQuery("SELECT * FROM user ORDER BY id DESC LIMIT 1");
	}
	catch (sql::SQLException e) {
		std::cout << "Failed to query our database: " << e.what() << std::endl;
		*error = INTERNAL_SERVER_ERROR;
		return false;
	}
	int id;
	while (m_pResultSet->next()) {
		id = m_pResultSet->getInt("id");
	}
	// Insert into web_auth table
	try {
		m_pInsertStatement = m_pConnection->prepareStatement(
			"INSERT INTO web_auth (email, salt, hashed_password, userId) VALUES (?, ?, ?, ?);");
	}
	catch (sql::SQLException e) {
		std::cout << "Failed to create statements: " << e.what() << std::endl;
		*error = INTERNAL_SERVER_ERROR;
		return false;
	}
	m_pInsertStatement->setString(1, email);
	m_pInsertStatement->setString(2, *salt);
	m_pInsertStatement->setString(3, password);
	m_pInsertStatement->setInt(4, id);
	// Execute
	try {
		m_pInsertStatement->execute();
	}
	catch (sql::SQLException e) {
		std::cout << "Failed to add user to the database: " << e.what() << std::endl;
		// Delete User
		std::string err = e.what();
		if (err.substr(0, 15) == "Duplicate entry")
		{
			*error = ACCOUNT_ALREADY_EXISTS;
		}
		try {
			m_pStatement = m_pConnection->createStatement();
			m_pStatement->executeQuery("DELETE FROM user WHERE id = " + std::to_string(id));
		}
		catch (sql::SQLException e) {
			//std::cout << "Failed to delete user: " << e.what() << std::endl;
		}

		return false;
	}

	std::cout << "Successfully created account!" << std::endl;
	*userId = id;
	return true;
}

bool AuthDB::AuthenticateAccount(const char* email, const char* password, int* error, AuthHelper SERVER, int* userId, std::string* creation_date)
{
	std::string unhashedPassword = password;
	// Get userId
	try {
		m_pStatement = m_pConnection->createStatement();
		m_pResultSet = m_pStatement->executeQuery("SELECT * FROM web_auth WHERE email = '" + std::string{ email } + "'");
	}
	catch (sql::SQLException e) {
		std::cout << "Failed to query our database: " << e.what() << std::endl;
		*error = INTERNAL_SERVER_ERROR;
		return false;
	}
	// Extract variables from sql result
	int id;
	std::string sqlEmail;
	std::string sqlSalt;
	std::string sqlHash;
	if (!m_pResultSet->next())
	{
		*error = INVALID_CREDENTIALS;
		return false;
	}
	id = m_pResultSet->getInt("userId");
	sqlEmail = m_pResultSet->getString("email");
	sqlSalt = m_pResultSet->getString("salt");
	sqlHash = m_pResultSet->getString("hashed_password");

	// Hash password then compare
	std::string hashedPassword = SERVER.hashSHA256(password + sqlSalt);
	//std::cout << "attempt at password: " << hashedPassword << std::endl;
	//std::cout << "attempt at salt: " << sqlSalt << std::endl;
	if (hashedPassword == sqlHash)
	{
		std::cout << "account login successful" << std::endl;
		m_pStatement->execute("UPDATE user SET last_login = '" + currentDateTime() + "' WHERE id = " + std::to_string(id) + ";");
		*userId = id;
			// Get creationdate
			try {
			m_pResultSet = m_pStatement->executeQuery("SELECT * FROM user WHERE id = '" + std::to_string(id) + "'");
		}
		catch (sql::SQLException e) {
			std::cout << "Failed to query our database: " << e.what() << std::endl;
			*error = INTERNAL_SERVER_ERROR;
			return false;
		}
		m_pResultSet->next();
		*creation_date = m_pResultSet->getString("creation_date");
		return true;
	}
	else
	{
		std::cout << "account login unsuccessful " << std::endl;
		*error = INVALID_CREDENTIALS;
		return false;
	}
	//std::cout << password << " " << sqlSalt << " " << hashedPassword << std::endl << sqlHash;
}