#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <ctime>
#include <limits>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <fstream> // Required for file writing

using namespace std;

// ------------------------------------------------------------
// Utility Helpers
// ------------------------------------------------------------

void clearInputError() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int readInt(const string &prompt, int lo, int hi) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= lo && value <= hi) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        clearInputError();
        cout << "  Invalid input. Please enter a number between " << lo << " and " << hi << ".\n";
    }
}

double readDouble(const string &prompt, double lo = 0.0) {
    double value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= lo) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        clearInputError();
        cout << "  Invalid input. Please enter a valid amount (minimum " << lo << ").\n";
    }
}

string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

string peso(double amount) {
    ostringstream oss;
    oss << fixed << setprecision(2) << amount;
    return "P" + oss.str();
}

// ------------------------------------------------------------
// Data Structures
// ------------------------------------------------------------

struct User {
    string username;
    string password;
    string role;
};

struct Product {
    int id;
    string category;
    string name;
    double price;
    string specs;
    int stock;
};

struct Transaction {
    int id;
    string username;
    double total;
    string method;
    string timestamp;
};

// ------------------------------------------------------------
// In-Memory Store Manager
// ------------------------------------------------------------

class StoreManager {
private:
    vector<User> users;
    vector<Product> products;
    vector<Transaction> transactions;
    int nextTransactionId = 1;
    const int LOW_STOCK_THRESHOLD = 5;

    string getCurrentTime() {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
        return string(buffer);
    }

public:
    StoreManager() {
        seedDefaultProducts();
    }

    void seedDefaultProducts() {
        products.push_back({1, "CPU", "AMD Ryzen 5 5600", 6800.00, "6-core/12-thread", 4});
        products.push_back({2, "CPU", "Intel Core i5-13400F", 9500.00, "10-core, 4.6GHz", 10});
        products.push_back({3, "GPU", "NVIDIA RTX 4060", 18500.00, "8GB GDDR6", 2});
        products.push_back({4, "GPU", "AMD RX 6600", 12500.00, "8GB GDDR6", 12});
        products.push_back({5, "RAM", "Kingston Fury 16GB DDR4", 1900.00, "3200MHz", 30});
        products.push_back({6, "Storage", "Kingston NV2 1TB NVMe", 2600.00, "PCIe 4.0 SSD", 25});
    }

    bool registerUser(const string& user, const string& pass, const string& role) {
        for (const auto& u : users) {
            if (u.username == user) {
                return false; 
            }
        }
        users.push_back({user, pass, role});
        return true;
    }

    string loginUser(const string& user, const string& pass) {
        for (const auto& u : users) {
            if (u.username == user && u.password == pass) {
                return u.role;
            }
        }
        return "";
    }

    void displayCatalog() {
        cout << "\n----------------------------------------------------------------------------------\n";
        cout << left << setw(4) << "ID" << setw(12) << "Category" << setw(24) << "Product"
             << setw(11) << "Price" << setw(25) << "Specs" << "Stock\n";
        cout << "----------------------------------------------------------------------------------\n";

        for (const auto& p : products) {
            cout << left << setw(4) << p.id
                 << setw(12) << p.category
                 << setw(24) << p.name
                 << setw(11) << peso(p.price)
                 << setw(25) << p.specs
                 << p.stock << (p.stock <= LOW_STOCK_THRESHOLD ? " (LOW STOCK)" : "") << "\n";
        }
        cout << "----------------------------------------------------------------------------------\n";
    }

    bool checkLowStockAlert() {
        cout << "\n==============================================\n";
        cout << "           LOW STOCK ALERTS                   \n";
        cout << "==============================================\n";
        bool lowStockFound = false;
        for (const auto& p : products) {
            if (p.stock <= LOW_STOCK_THRESHOLD) {
                cout << "WARNING: [ID: " << p.id << "] " << setw(24) << p.name 
                     << " | Stock: " << p.stock << " (Threshold: " << LOW_STOCK_THRESHOLD << ")\n";
                lowStockFound = true;
            }
        }
        if (!lowStockFound) {
            cout << "Great! All items are currently well-stocked.\n";
        }
        cout << "==============================================\n";
        return lowStockFound;
    }

    void restockFlow() {
        displayCatalog();
        int id = readInt("Enter Product ID to Restock: ", 1, 9999);
        Product* p = getProduct(id);
        if (p != nullptr) {
            int qty = readInt("Enter Quantity to Add: ", 1, 1000);
            p->stock += qty;
            cout << "\nSUCCESS: Restocked " << p->name << ". New stock level: " << p->stock << "\n";
        } else {
            cout << "\nFailed. Invalid Product ID.\n";
        }
    }

    bool addStock(int id, int qty) {
        for (auto& p : products) {
            if (p.id == id) {
                p.stock += qty;
                return true;
            }
        }
        return false;
    }

    bool purchaseProduct(int id, int qty, double& outTotal) {
        for (auto& p : products) {
            if (p.id == id) {
                if (p.stock >= qty) {
                    p.stock -= qty;
                    outTotal = p.price * qty;
                    return true;
                }
                return false; 
            }
        }
        return false; 
    }

    Product* getProduct(int id) {
        for (auto& p : products) {
            if (p.id == id) {
                return &p;
            }
        }
        return nullptr;
    }

    void recommendProducts(double budget, const string& targetCategory) {
        cout << "\n--- Recommendations for '" << targetCategory << "' under " << peso(budget) << " ---\n";
        bool found = false;
        string lowerTarget = toLower(targetCategory);
        
        for (const auto& p : products) {
            if (toLower(p.category) == lowerTarget && p.price <= budget && p.stock > 0) {
                cout << left << "ID: " << setw(4) << p.id 
                     << "| " << setw(24) << p.name 
                     << "| Price: " << setw(11) << peso(p.price) 
                     << "| Specs: " << p.specs << "\n";
                found = true;
            }
        }
        if (!found) {
            cout << "No items found matching your budget and category, or items are out of stock.\n";
        }
        cout << "----------------------------------------------------------------------\n";
    }

    // Records the transaction and automatically creates a text receipt file
    void recordTransactionAndCreateFile(const string& user, double total, const string& method, const string& productName, int qty, double cash, double change) {
        int tId = nextTransactionId++;
        string timeStr = getCurrentTime();
        transactions.push_back({tId, user, total, method, timeStr});

        // Generate text file receipt
        string filename = "receipt_" + to_string(tId) + ".txt";
        ofstream outFile(filename);
        if (outFile.is_open()) {
            outFile << "==============================================\n";
            outFile << "              EzPC POS RECEIPT                \n";
            outFile << "==============================================\n";
            outFile << "Transaction ID: " << tId << "\n";
            outFile << "Timestamp:      " << timeStr << "\n";
            outFile << "Customer User:  " << user << "\n";
            outFile << "----------------------------------------------\n";
            outFile << "Item Purchased: " << productName << " x" << qty << "\n";
            outFile << "Total Due:      " << peso(total) << "\n";
            outFile << "Cash Tendered:  " << peso(cash) << "\n";
            outFile << "Change:         " << peso(change) << "\n";
            outFile << "Payment Method: " << method << "\n";
            outFile << "==============================================\n";
            outFile << "     Thank you for shopping at EzPC!          \n";
            outFile << "==============================================\n";
            outFile.close();
            cout << "  [Receipt saved to file: " << filename << "]\n";
        } else {
            cout << "  [Warning: Could not create receipt text file.]\n";
        }
    }

    void displayTransactions(const string& targetUser = "") {
        cout << "\n----------------------------------------------------------------------------------\n";
        cout << left << setw(4) << "ID" << setw(15) << "User" << setw(15) << "Total"
             << setw(15) << "Method" << "Timestamp\n";
        cout << "----------------------------------------------------------------------------------\n";

        bool found = false;
        for (const auto& t : transactions) {
            if (targetUser.empty() || t.username == targetUser) {
                cout << left << setw(4) << t.id
                     << setw(15) << t.username
                     << setw(15) << peso(t.total)
                     << setw(15) << t.method
                     << t.timestamp << "\n";
                found = true;
            }
        }
        if (!found) {
            cout << "No transactions found.\n";
        }
        cout << "----------------------------------------------------------------------------------\n";
    }
};

// ------------------------------------------------------------
// Main Flow
// ------------------------------------------------------------

int main() {
    StoreManager store;
    string currentUser = "", currentRole = "";
    bool appRunning = true;

    cout << "==============================================\n";
    cout << "   Welcome to EzPC POS (In-Memory Edition)\n";
    cout << "==============================================\n";

    while (appRunning) {
        while (true) {
            cout << "\n1. Log In\n2. Create Account\n3. Exit App\n";
            int choice = readInt("Select an option: ", 1, 3);
            if (choice == 3) {
                appRunning = false;
                break;
            }

            string user, pass;
            cout << "Username: "; getline(cin, user);
            cout << "Password: "; getline(cin, pass);

            if (choice == 1) {
                currentRole = store.loginUser(user, pass);
                if (!currentRole.empty()) {
                    currentUser = user;
                    cout << "\nLogin successful! Welcome " << user << " (" << currentRole << ")\n";
                    if (currentRole == "administrator") {
                        store.checkLowStockAlert();
                    }
                    break;
                }
                cout << "\nInvalid credentials! Please try again.\n";
            } else {
                cout << "Role (1 = Buyer, 2 = Admin): ";
                int rChoice = readInt("", 1, 2);
                string role = (rChoice == 1) ? "buyer" : "administrator";
                if (store.registerUser(user, pass, role)) {
                    currentUser = user;
                    currentRole = role;
                    cout << "\nAccount created and logged in!\n";
                    if (currentRole == "administrator") {
                        store.checkLowStockAlert();
                    }
                    break;
                }
                cout << "\nUsername already taken. Try another.\n";
            }
        }

        if (!appRunning) break;

        bool loggedIn = true;
        while (loggedIn) {
            cout << "\n============== MENU (" << currentRole << ") ==============\n";
            cout << "1. View Catalog\n";
            
            if (currentRole == "administrator") {
                cout << "2. Restock Product\n";
                cout << "3. View Low Stock Alerts & Quick Restock\n";
                cout << "4. View Transaction History\n";
                cout << "5. Logout\n";
                int choice = readInt("Choice: ", 1, 5);
                
                if (choice == 1) {
                    store.displayCatalog();
                } else if (choice == 2) {
                    store.restockFlow();
                } else if (choice == 3) {
                    bool hasLowStock = store.checkLowStockAlert();
                    if (hasLowStock) {
                        cout << "Would you like to restock an item now? (1 = Yes, 2 = No): ";
                        int doRestock = readInt("", 1, 2);
                        if (doRestock == 1) {
                            int id = readInt("Enter Product ID: ", 1, 9999);
                            int qty = readInt("Quantity to Add: ", 1, 1000);
                            if (store.addStock(id, qty)) {
                                cout << "\nStock successfully updated!\n";
                            } else {
                                cout << "\nFailed. Invalid Product ID.\n";
                            }
                        }
                    }
                } else if (choice == 4) {
                    store.displayTransactions();
                } else {
                    cout << "Logging out...\n";
                    loggedIn = false;
                }
            } else {
                cout << "2. Purchase Item\n";
                cout << "3. Get Budget Recommendation\n";
                cout << "4. View My Transactions\n";
                cout << "5. Logout\n";
                int choice = readInt("Choice: ", 1, 5);
                
                if (choice == 1) {
                    store.displayCatalog();
                } else if (choice == 2) {
                    store.displayCatalog();
                    int id = readInt("Enter Product ID: ", 1, 9999);
                    int qty = readInt("Quantity: ", 1, 100);
                    
                    Product* p = store.getProduct(id);
                    if (p != nullptr && p->stock >= qty) {
                        double totalDue = p->price * qty;
                        cout << "\nTotal amount due: " << peso(totalDue) << "\n";
                        
                        double cash = readDouble("Enter cash amount: P", totalDue);
                        double change = cash - totalDue;
                        
                        double dummyTotal = 0.0;
                        store.purchaseProduct(id, qty, dummyTotal);
                        
                        // Records transaction and automatically exports text receipt file
                        store.recordTransactionAndCreateFile(currentUser, totalDue, "Cash", p->name, qty, cash, change);
                        
                        cout << "\n==============================================\n";
                        cout << "              RECEIPT                 \n";
                        cout << "==============================================\n";
                        cout << "Item:      " << p->name << " x" << qty << "\n";
                        cout << "Total:     " << peso(totalDue) << "\n";
                        cout << "Cash:      " << peso(cash) << "\n";
                        cout << "Change:    " << peso(change) << "\n";
                        cout << "==============================================\n";
                    } else {
                        cout << "\nPurchase failed. Either out of stock or invalid ID.\n";
                    }
                } else if (choice == 3) {
                    double budget = readDouble("\nEnter your maximum budget: P", 0);
                    cout << "Enter product category (e.g., CPU, GPU, RAM, Storage): ";
                    string category;
                    getline(cin, category);
                    store.recommendProducts(budget, category);
                } else if (choice == 4) {
                    store.displayTransactions(currentUser);
                } else {
                    cout << "Logging out...\n";
                    loggedIn = false;
                }
            }
        }
    }

    cout << "Thank you for using EzPC POS.\n";
    return 0;
}