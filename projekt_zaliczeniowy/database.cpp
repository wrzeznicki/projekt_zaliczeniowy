#include "database.h"
#include "sqlite/sqlite3.h"
#include <iostream>
#include <fstream> 
#include <conio.h>

static int callback(void* NotUsed, int argc, char** argv, char** azColName);
static int callback2(void* NotUsed, int argc, char** argv, char** azColName);
bool isUser = false;

void Database::deleteData() {
	string id;
	std::cout << "Podaj ID hasla ktore chcesz usunac: "; std::cin >> id;

	string sql = "DELETE FROM PASSWORDS WHERE ID= ? and user = ?;";

	sqlite3_stmt* stmt;
	const char* ozTest;

	sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &ozTest);
	sqlite3_bind_text(stmt, 1, id.c_str(), id.length(), NULL);
	sqlite3_bind_text(stmt, 2, login.c_str(), login.length(), NULL);

	int ret = sqlite3_step(stmt);

	if (ret != SQLITE_DONE || sqlite3_changes(db) == 0) {
		std::cerr << "Nie ma zapisanego hasla o takim ID.\n";
	}
	else
		std::cout << "Usunieto pomyslnie.\n";
}

void Database::insertData() {
	string password, website, website_login;
	std::cout << "Podaj strone do ktorej haslo chcesz zapisac: "; std::cin >> website;
	std::cout << "Podaj login do strony: "; std::cin >> website_login;
	std::cout << "Podaj haslo: "; std::cin >> password;

	std::string encryptedPassword = crypto.encrypt(password);

	string sql("INSERT INTO passwords (login, password, website, user) VALUES (?, ?, ?, ?);");

	sqlite3_stmt* stmt;
	const char* ozTest;

	sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &ozTest);

	sqlite3_bind_text(stmt, 1, website_login.c_str(), website_login.length(), NULL);
	sqlite3_bind_text(stmt, 2, encryptedPassword.c_str(), encryptedPassword.length(), NULL);
	sqlite3_bind_text(stmt, 3, website.c_str(), website.length(), NULL);
	sqlite3_bind_text(stmt, 4, login.c_str(), login.length(), NULL);

	int ret = sqlite3_step(stmt);

	if (ret != SQLITE_DONE) {
		std::cerr << "Nie udalo si? dodac.\n";
		sqlite3_free(msgError);
	}
	else
		std::cout << "Pomyslnie dodano\n";

}

void Database::selectData() {

	string sql = "select * from passwords where user = ?";

	sqlite3_stmt* stmt;
	const char* ozTest;
	sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &ozTest);


	sqlite3_bind_text(stmt, 1, login.c_str(), login.length(), NULL);
	int ret = sqlite3_step(stmt);


	if (ret == SQLITE_ROW) {

		std::cout << std::endl;
		std::cout << "Zapisane loginy i strony. Aby wyswietlic haslo do danej strony uzyj funkcji \"Wyswietl haslo po id lub nazwie\"." << std::endl;
		std::cout << std::endl;

		while (ret == SQLITE_ROW) {
			for (int i = 0; i < sqlite3_column_count(stmt) - 1; i++)
			{
				if (i == 2) continue;
				const char* colName = sqlite3_column_name(stmt, i);
				char* data = (char*)sqlite3_column_text(stmt, i);


				std::cout << colName << ": " << data << ", ";
			}
			std::cout << std::endl;
			ret = sqlite3_step(stmt);
		}

	}
	else {
		std::cerr << "Brak zapisanych hasel\n";
		sqlite3_free(msgError);
	}
}

void Database::selectOne() {

	string param;
	std::cout << "Podaj id lub strone z ktora haslo jest powiazane: "; std::cin >> param;

	string sql = "select * from passwords where user = ? and (id = ? or website = ?)";

	sqlite3_stmt* stmt;
	const char* ozTest;
	sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &ozTest);

	sqlite3_bind_text(stmt, 1, login.c_str(), login.length(), NULL);
	sqlite3_bind_text(stmt, 2, param.c_str(), param.length(), NULL);
	sqlite3_bind_text(stmt, 3, param.c_str(), param.length(), NULL);

	int ret = sqlite3_step(stmt);

	if (ret == SQLITE_ROW) {

		for (int i = 0; i < sqlite3_column_count(stmt) - 1; i++)
		{
			const char* colName = sqlite3_column_name(stmt, i);

			char* data = (char*)sqlite3_column_text(stmt, i);

			if (i == 2) {
				string str = crypto.decrypt((char*)sqlite3_column_text(stmt, i));
				data = &str[0];
			}
			else {
				data = (char*)sqlite3_column_text(stmt, i);
			}


			std::cout << colName << ": " << data << ", ";
		}
		std::cout << std::endl;
	}
	else {
		std::cerr << "Brak zapisanych hasel\n";
		sqlite3_free(msgError);
	}
}
 
void Database::createAccount() {
	std::cout << "Podaj login: "; std::cin >> login;
	std::cout << "podaj haslo: "; std::cin >> password;
	system("cls");

	string sql = "INSERT INTO users (login, password) VALUES (?, ?);";

	sqlite3_stmt* stmt;
	const char* ozTest;

	std::string encryptedPassword = crypto.encrypt(password);

	sqlite3_prepare_v2(db, sql.c_str() ,sql.length(), &stmt, &ozTest);
	sqlite3_bind_text(stmt, 1, login.c_str(), login.length(), NULL);
	sqlite3_bind_text(stmt, 2, encryptedPassword.c_str(), encryptedPassword.length(), NULL);

	int exit = sqlite3_step(stmt);

	if (exit != SQLITE_DONE) {
		std::cout << sqlite3_errmsg(db) << std::endl;
		registered = false;
	}
	else registered = true;

}

void Database::loginToAccount() {

	std::cout << "Podaj login: "; std::cin >> login;
	std::cout << "podaj haslo: "; std::cin >> password;
	system("cls");
	string sql = "select count(*) from users where login = ? and password = ?;";

	sqlite3_stmt* stmt;
	const char* ozTest;

	std::string encryptedPassword = crypto.encrypt(password);

	int exit = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &ozTest);
	sqlite3_bind_text(stmt, 1, login.c_str(), login.length(), NULL);
	sqlite3_bind_text(stmt, 2, encryptedPassword.c_str(), encryptedPassword.length(), NULL);

	exit = sqlite3_step(stmt);
	int count = sqlite3_column_int(stmt, 0);
	
	logged = count > 0;
	sqlite3_finalize(stmt);
}

void Database::changePassword() {
	string id;
	string newPassword;
	std::cout << "Podaj ID hasla ktore chcesz zmienic: "; std::cin >> id;


	sqlite3_stmt* stmt;
	const char* ozTest;

	string sql = "SELECT * FROM passwords WHERE (id=? and user=?);";
	int result = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL);

	sqlite3_bind_text(stmt, 1, id.c_str(), id.length(), NULL);
	sqlite3_bind_text(stmt, 2, login.c_str(), login.length(), NULL);


	if (result != SQLITE_OK && sqlite3_step(stmt) != SQLITE_ROW)
	{
		std::cerr << "Nie ma hasla o podanym id.\n";
		return;
	}

	sql = "UPDATE passwords SET password=? WHERE (id=? and user=?);";
	std::cout << "Podaj nowe haslo: "; std::cin >> newPassword;

	std::string encryptedPassword = crypto.encrypt(password);


	sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &ozTest);
	sqlite3_bind_text(stmt, 1, encryptedPassword.c_str(), encryptedPassword.length(), NULL);
	sqlite3_bind_text(stmt, 2, id.c_str(), id.length(), NULL);
	sqlite3_bind_text(stmt, 3, login.c_str(), login.length(), NULL);


	int ret = sqlite3_step(stmt);

	std::cout << sqlite3_errmsg(db);

	if (ret != SQLITE_DONE) {
		std::cerr << "Nie udalo sie zmienic.\n";
	}
	else
		std::cout << "Zmieniono pomyslnie.\n";

}


Database::Database() 
{

	std::ofstream dbfile("database.db", std::fstream::in | std::fstream::out | std::fstream::app);
	dbfile.close();

	int exit;
	char* zErrMsg = 0;

	exit = sqlite3_open("database.db", &db);


	string sql;

	sql = "CREATE TABLE IF NOT EXISTS users (" \
		"id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
		"login TEXT NOT NULL," \
		"password TEXT NOT NULL);";

	exit = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

	if (exit != SQLITE_OK) {
		std::cout << zErrMsg << std::endl;
	}

	sql = "CREATE TABLE IF NOT EXISTS passwords (" \
		"id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
		"login TEXT NOT NULL," \
		"password TEXT NOT NULL," \
		"website TEXT NOT NULL," \
		"user TEXT NOT NULL);";

	exit = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

	if (exit != SQLITE_OK) {
		std::cout << zErrMsg << std::endl;
	}

}


Database::~Database()
{
	sqlite3_close(db);
}

void Database::menuToDatabase() {

	char choice{};
	int attempts = 1;
	sqlite3_stmt* stmt2;
	const char* ozTest2;
	string valid = "select * from users; ";
	sqlite3_prepare_v2(db, valid.c_str(), valid.length(), &stmt2, &ozTest2);
	int rows_count = 0;

	while (sqlite3_step(stmt2) == SQLITE_ROW)
	{
		rows_count++;
	}

	if (rows_count > 0) {
		std::cout << "-----------------------" << std::endl;
		std::cout << "MENADZER HASEL" << std::endl;
		std::cout << "-----------------------" << std::endl;
		std::cout << std::endl;
		std::cout << "1.Zaloguj sie\n2.Zarejestruj sie\n";
		choice = _getch();

		switch (choice)
		{
		case '1':
			do {
				if (!logged && attempts > 1) std::cout << "\nNieprawidlowe dane\n";
				loginToAccount();
				attempts++;
			} while (logged != true);
			break;
		case '2':
			do {
				createAccount();
				attempts++;
			} while (registered != true);
			break;
		default:
			std::cout << "Nieprawidlowy znak\n";
			break;
		}
	}
	else {
		std::cout << "Menedzer hasel\n\n1.Zarejestruj sie\n";
		choice = _getch();

		switch (choice)
		{
		case '1':
			do {
				createAccount();
				attempts++;
			} while (registered != true);
			break;
		default:
			std::cout << "Nieprawidlowy znak\n";
			break;
		}
	}

}

static int callback(void* NotUsed, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++) {
		std::cout <<azColName[i]<<": " << argv[i] << ", ";
	}

	std::cout << std::endl;

	return 0;
}

static int callback2(void* NotUsed, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++) {
		if (atoi(argv[i]) == 1) isUser = true;
	}

	return 0;
}
