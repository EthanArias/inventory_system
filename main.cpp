#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <iostream>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <mysql_driver.h>
#include <string>
#include <algorithm>
#include <cctype>

std::string getName() {
    std::cout << "Enter product name: ";
    std::string name;
    std::getline(std::cin, name); // Use getline to capture spaces and punctuation

    name.erase(std::remove_if(name.begin(), name.end(), [](char c) {
        return !(std::isalnum(static_cast<unsigned char>(c)) ||
            c == ' ' || c == ',' || c == ':');
        }), name.end());
    return name;
}
int getQty() {
    std::cout << "Enter product quantity: ";
    int num;
    std::cin >> num;
    return abs(num);
}
double getPrice() {
    std::cout << "Enter product price: ";
    int num;
    std::cin >> num;
    return abs(num);
}

void show();
int menu();
int check_num(int x=0);

void add(sql::Connection* connection);
void remove(sql::Connection* connection);
void sell(sql::Connection* connection);
void buy(sql::Connection* conn);
void display(sql::Statement* Statement);

int main()
{
    std::string hostName, userName, password;
    try {
        sql::mysql::MySQL_Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        int choice;

        // Create a connection
        std::cout << "Enter MySQL host (e.g., tcp://localhost:3306): ";
        std::getline(std::cin, hostName);
        std::cout << "Enter username: ";
        std::getline(std::cin, userName);
        std::cout << "Enter password: ";
        std::getline(std::cin, password);
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(hostName, userName, password);

        // Connect to the database
        stmt = con->createStatement();
        stmt->execute("CREATE DATABASE IF NOT EXISTS gfgcppinventory");
        con->setSchema("gfgcppinventory");

        // Create a statement
        stmt = con->createStatement();

        // SQL query to create a table
        std::string createTableSQL
            = "CREATE TABLE IF NOT EXISTS products ("
            "id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
            "name VARCHAR(255),"
            "quantity INT UNSIGNED,"
            "price DOUBLE)";

        stmt->execute(createTableSQL);
        show();
        choice = check_num(3);
        do
        {
            switch (choice) {
            case 1: // Add new product
                add(con);
                choice = menu();
                break;
            case 2: // Remove product
                remove(con);
                choice = menu();
                break;
            case 3: // sell/decrement product quantity
                sell(con);
                choice = menu();
                break;
            case 4: // buy/increment product quantity
                buy(con);
                choice = menu();
                break;
            case 5: // Display current table
                display(stmt);
                choice = menu();
                break;
            default:
                break;
            }
        } while (choice != 5);

        delete stmt;
        delete con;
    }
    catch (sql::SQLException& e) {
        std::cerr << "SQL Error: " << e.what() << std::endl;
        std::cerr << "MySQL Error Code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }

    return 0;
}

void show()
{
    std::cout << "\n\t ***** MySQL Grocery Store Product Inventory System ***** \n" << std::endl;
    std::cout << " Press 1 to add new product                   \n"
        << " Press 2 to remove product                          \n"
        << " Press 3 to sell/decrement product quantity         \n"
        << " Press 4 to buy/increment product quantity          \n"
        << " Press 5 to display current table                   \n"
        << " Press 6 to Exit                                    \n";
}

int menu()
{
    int num_cases;
    std::cout << " Do you want to perform another operation?\n"
        << " 1.Yes\t2.No\n";
    int t = check_num(1);
    if (t == 1) {
        system("CLS");
        show();
        std::cout << " Enter your desired choice: " << std::endl;
        num_cases = check_num(3);
        return num_cases;
    }
    else {
        return 6;
    }
}

int check_num(int x)
{
    int num;
    switch (x)
    {
    case 1:
    case 2:
        while (!(std::cin >> num) || num > 2) {
            std::cin.clear();
            while (std::cin.get() != '\n') continue;
            std::cout << "Sorry, wrong input!!" << std::endl;
        }
        while (std::cin.get() != '\n') continue;
        break;
    case 3:
        while (!(std::cin >> num) || num > 6) {
            std::cin.clear();
            while (std::cin.get() != '\n') continue;
            std::cout << "Sorry, wrong input!!" << std::endl;
        }
        while (std::cin.get() != '\n') continue;
        break;
    default:
        while (!(std::cin >> num)) {
            std::cin.clear();
            while (std::cin.get() != '\n') continue;
            std::cout << "Sorry, wrong input!!" << std::endl;
        }
        while (std::cin.get() != '\n') continue;
        break;
    }
    return num;
}

void add(sql::Connection* conn)
{
    system("CLS");
    try {
        // Step 1: Find the lowest available ID
        std::unique_ptr<sql::PreparedStatement> idStmt(
            conn->prepareStatement(
                "SELECT id FROM products ORDER BY id"
            ));
        std::unique_ptr<sql::ResultSet> res(idStmt->executeQuery());

        int expectedId = 1;
        while (res->next()) {
            if (res->getInt("id") != expectedId)
                break;
            ++expectedId;
        }

        std::string n = getName();
        int q = getQty();
        double p = getPrice();

        // Step 2: Insert with chosen ID
        std::unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement(
                "INSERT INTO products (id, name, quantity, price) VALUES (?, ?, ?, ?)"
            ));
        pstmt->setInt(1, expectedId);
        pstmt->setString(2, n);
        pstmt->setUInt(3, q);
        pstmt->setDouble(4, p);

        pstmt->executeUpdate();
        std::cout << "Product added successfully.\n" << std::endl;
    }
    catch (sql::SQLException& e) {
        std::cerr << "SQL Error: " << e.what() << std::endl;
    }
}

void remove(sql::Connection* conn) { 
    system("CLS");
    int productId;
    std::cout << "Enter Product ID to delete: ";
    std::cin >> productId;

    try {
        // Confirm product exists
        std::unique_ptr<sql::PreparedStatement> checkStmt(
            conn->prepareStatement("SELECT * FROM products WHERE id = ?"));
        checkStmt->setInt(1, productId);
        std::unique_ptr<sql::ResultSet> res(checkStmt->executeQuery());

        if (!res->next()) {
            std::cout << "No product found with ID " << productId << ".\n";
            return;
        }

        // Perform delete
        std::unique_ptr<sql::PreparedStatement> delStmt(
            conn->prepareStatement("DELETE FROM products WHERE id = ?"));
        delStmt->setInt(1, productId);
        int affectedRows = delStmt->executeUpdate();

        if (affectedRows > 0) {
            std::cout << "Product with ID " << productId << " deleted successfully.\n";
        }
        else {
            std::cout << "Failed to delete product.\n";
        }
    }
    catch (sql::SQLException& e) {
        std::cerr << "SQL Error: " << e.what() << std::endl;
    }
}

void sell(sql::Connection* conn)
{
    system("CLS");
    int productId;
    int reduceBy;

    std::cout << "Enter Product ID to reduce stock: ";
    std::cin >> productId;

    std::cout << "Enter quantity to reduce: ";
    std::cin >> reduceBy;

    try {
        // Step 1: Check current quantity
        std::unique_ptr<sql::PreparedStatement> checkStmt(
            conn->prepareStatement("SELECT quantity FROM products WHERE id = ?"));
        checkStmt->setInt(1, productId);
        std::unique_ptr<sql::ResultSet> res(checkStmt->executeQuery());

        if (!res->next()) {
            std::cout << "Product with ID " << productId << " not found.\n";
            return;
        }

        int currentQuantity = res->getInt("quantity");

        if (reduceBy <= 0) {
            std::cout << "Reduction value must be a positive number.\n";
            return;
        }

        int newQuantity = std::max(0, currentQuantity - reduceBy);

        // Step 2: Update stock
        std::unique_ptr<sql::PreparedStatement> updateStmt(
            conn->prepareStatement("UPDATE products SET quantity = ? WHERE id = ?"));
        updateStmt->setInt(1, newQuantity);
        updateStmt->setInt(2, productId);

        updateStmt->executeUpdate();
        std::cout << "Stock updated. New quantity: " << newQuantity << "\n" << std::endl;
    }
    catch (sql::SQLException& e) {
        std::cerr << "SQL Error: " << e.what() << std::endl;
    }
}

void buy(sql::Connection* conn)
{
    system("CLS");
    int productId;
    int addQuantity;

    std::cout << "Enter Product ID to restock: ";
    std::cin >> productId;

    std::cout << "Enter quantity to add: ";
    std::cin >> addQuantity;

    try {
        // Step 1: Retrieve current quantity
        std::unique_ptr<sql::PreparedStatement> checkStmt(
            conn->prepareStatement("SELECT quantity FROM products WHERE id = ?"));
        checkStmt->setInt(1, productId);
        std::unique_ptr<sql::ResultSet> res(checkStmt->executeQuery());

        if (!res->next()) {
            std::cout << "Product with ID " << productId << " not found.\n";
            return;
        }

        int currentQuantity = res->getInt("quantity");

        if (addQuantity <= 0) {
            std::cout << "Addition value must be greater than zero.\n";
            return;
        }

        int newQuantity = currentQuantity + addQuantity;

        // Step 2: Update quantity
        std::unique_ptr<sql::PreparedStatement> updateStmt(
            conn->prepareStatement("UPDATE products SET quantity = ? WHERE id = ?"));
        updateStmt->setInt(1, newQuantity);
        updateStmt->setInt(2, productId);

        updateStmt->executeUpdate();
        std::cout << "Stock restocked. New quantity: " << newQuantity << "\n" <<std::endl;

    }
    catch (sql::SQLException& e) {
        std::cerr << "SQL Error: " << e.what() << std::endl;
    }

}

void display(sql::Statement* statement)
{
    system("CLS");
    try {
        std::unique_ptr<sql::ResultSet> res(statement->executeQuery("SELECT * FROM products ORDER BY id"));

        std::cout << "ID\tName\t\tQuantity\tPrice\n";
        std::cout << "---------------------------------------------\n";

        while (res->next()) {
            std::cout << res->getInt("id") << "\t"
                << res->getString("name") << "\t\t"
                << res->getUInt("quantity") << "\t\t"
                << res->getDouble("price") << "\n";
        }
        std::cout << "\nTable displayed successfully.\n" << std::endl;
    }
    catch (sql::SQLException& e) {
        std::cerr << "SQL Error: " << e.what() << std::endl;
    }
}
