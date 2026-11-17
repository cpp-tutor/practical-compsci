#include "Database.hpp"
#include <iostream>

int main() {
    using namespace CoreDB;

    std::cout << "====================================================\n";
    std::cout << "      CoreDB Modern C++ Relational Engine Demo      \n";
    std::cout << "====================================================\n\n";

    try {
        Database db;

        // 1. Create Tables
        std::cout << "[Step 1] Creating schema for 'engineers'...\n";
        db.execute_ddl(
            "CREATE TABLE engineers ("
            "  id INT PRIMARY KEY,"
            "  name TEXT NOT NULL,"
            "  salary FLOAT NOT NULL,"
            "  dept TEXT"
            ");"
        );

        // 2. Insert Records
        std::cout << "[Step 2] Inserting records into 'engineers'...\n";
        db.execute_insert("INSERT INTO engineers VALUES (101, 'Ada Lovelace', 8500.50, 'Systems');");
        db.execute_insert("INSERT INTO engineers VALUES (102, 'Alan Turing', 9200.00, 'Cryptography');");
        db.execute_insert("INSERT INTO engineers VALUES (103, 'Grace Hopper', 8900.00, 'Compilers');");
        db.execute_insert("INSERT INTO engineers VALUES (104, 'Claude Shannon', 9100.00, 'Information Theory');");
        db.execute_insert("INSERT INTO engineers VALUES (105, 'John von Neumann', 9950.00, 'Architecture');");

        // 3. Query All Data (π * σ true)
        std::cout << "[Step 3] Query: SELECT * FROM engineers;";
        db.print_query("SELECT * FROM engineers;");

        // 4. Query with Projection and Selection Filter
        std::cout << "[Step 4] Query: SELECT name, salary FROM engineers WHERE salary > 9000.00;";
        db.print_query("SELECT name, salary FROM engineers WHERE salary > 9000.00;");

        // 5. Index-Accelerated Point Query on Primary Key
        std::cout << "[Step 5] Query: SELECT id, name, dept FROM engineers WHERE id = 103;";
        db.print_query("SELECT id, name, dept FROM engineers WHERE id = 103;");

        // 6. Demonstrate Transaction Atomicity and Constraint Rollback
        std::cout << "[Step 6] Testing Transactional ACID Guarantees...\n";
        Table& eng_table = db.get_table("engineers");

        std::cout << " -> Starting Transaction...\n";
        eng_table.begin_transaction();

        eng_table.insert_record({106, "Barbara Liskov", 9400.0, "Distributed"});
        std::cout << " -> Inserted 'Barbara Liskov' successfully.\n";

        try {
            std::cout << " -> Attempting to insert duplicate Primary Key (id: 101)...\n";
            eng_table.insert_record({101, "Imposter Record", 1000.0, "Security"});
        } catch (const std::exception& e) {
            std::cerr << " -> [Caught Expected Constraint Violation]: " << e.what() << "\n";
            std::cout << " -> Executing Atomic ROLLBACK...\n";
            eng_table.rollback();
        }

        std::cout << "\n[Step 7] Verifying table state post-rollback (ID 106 should NOT exist):";
        db.print_query("SELECT id, name FROM engineers;");

    } catch (const std::exception& e) {
        std::cerr << "CRITICAL ERROR: " << e.what() << "\n";
        return 1;
    }

    return 0;
}