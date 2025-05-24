# Database Module (`database`)

The `database` module is responsible for all interactions with the SQLite database. It will provide an abstraction layer over the raw SQLite C API, offering a cleaner and more C++ idiomatic way to perform database operations.

## Key Responsibilities

*   **Connection Management:** Handling the connection and disconnection to the SQLite database file.
*   **Schema Management:** Potentially including functions to initialize the database schema (create tables, indices, etc.) if it doesn't exist.
*   **CRUD Operations:** Providing interfaces for Create, Read, Update, and Delete operations for various game entities (accounts, characters, items, etc.).
*   **Query Execution:** Offering a safe way to execute custom SQL queries, including prepared statements to prevent SQL injection.
*   **Transaction Management:** Support for database transactions to ensure atomicity of multiple operations.

## Core Components (Planned)

### `Database` Class (e.g., `src/database/include/database/database.h`)

This will be the main class for interacting with the database.

*   **`Database(const std::string& db_path)`:** Constructor that takes the path to the SQLite database file. It will attempt to open a connection.
*   **`~Database()`:** Destructor that closes the database connection.
*   **`bool execute(const std::string& sql)`:** Executes a given SQL statement that doesn't return data (e.g., INSERT, UPDATE, DELETE, CREATE TABLE).
*   **`std::vector<std::vector<std::any>> query(const std::string& sql)`:** Executes a SQL query that returns data (e.g., SELECT). The result could be a vector of rows, where each row is a vector of `std::any` or a similar mechanism to hold different data types.
*   **`bool prepareStatement(const std::string& sql, /* some SQLite statement handle */ &stmt)`:** Prepares a SQL statement for later execution.
*   **`bool bindValue( /* statement handle */ stmt, int index, /* value */)`:** Binds a value to a prepared statement. Overloads for different types (int, string, double, etc.) will be needed.
*   **`bool stepStatement( /* statement handle */ stmt)`:** Executes one step of a prepared statement.
*   **`/* return type */ columnValue( /* statement handle */ stmt, int col_index)`:** Retrieves a value from the current result row of a stepped statement. Overloads for different types.
*   **`bool finalizeStatement( /* statement handle */ stmt)`:** Finalizes a prepared statement.
*   **`bool beginTransaction()`:** Starts a database transaction.
*   **`bool commitTransaction()`:** Commits the current transaction.
*   **`bool rollbackTransaction()`:** Rolls back the current transaction.
*   **`sqlite3* getDbHandle()`:** Returns the raw `sqlite3` pointer for advanced use or integration with other libraries if necessary, though direct use should be minimized.
*   **`std::string getLastErrorMsg()`:** Returns the last error message from SQLite.

### Data Models / Repositories (Conceptual)

While not strictly part of the `Database` class itself, the database module will be used by "repository" classes or data access objects (DAOs) that are specific to game entities. For example:

*   `AccountRepository`: Handles all database operations for `Account` objects.
    *   `createAccount(...)`
    *   `getAccountByLogin(...)`
    *   `updateAccount(...)`
    *   `deleteAccount(...)`
*   `CharacterRepository`: Handles all database operations for `Character` objects.
    *   `createCharacter(...)`
    *   `getCharactersByAccountId(...)`
    *   `getCharacterById(...)`
    *   `updateCharacter(...)`
    *   `deleteCharacter(...)`

These repositories would use an instance of the `Database` class to perform their operations. This keeps the entity-specific logic separate from the general database interaction logic.

## Error Handling

Methods should return `bool` to indicate success/failure or throw exceptions for more critical errors. The `getLastErrorMsg()` method can be used to retrieve SQLite-specific error details. Debug logging within the database module methods, especially when `DEBUG_MODE` is enabled, will be crucial.

## Future Enhancements

*   **Object-Relational Mapping (ORM):** A simple ORM-like functionality to map C++ objects to database rows and vice-versa could be considered, but might be overly complex for the initial implementation. Sticking to repositories and manual mapping is a good start.
*   **Connection Pooling:** If the server expects a very high number of concurrent database operations (less likely with SQLite in an embedded scenario compared to a client-server DB), connection pooling might be relevant, but SQLite is often used with a single writer, multiple reader model.

This documentation will be updated as the `database` module is implemented. 