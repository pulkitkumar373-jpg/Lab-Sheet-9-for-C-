
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <ctime>
using namespace std;

struct Expense {
    int id;
    string date;
    string category;
    double amount;
    string note;
};

vector<Expense> expenses;
double monthlyBudget = 5000;
int nextId = 1;

string todayDate() {
    time_t now = time(nullptr);
    tm *t = localtime(&now);

    char buffer[11];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", t);
    return string(buffer);
}

string currentMonth() {
    return todayDate().substr(0, 7);
}

void saveData() {
    ofstream file("expenses.txt");

    file << monthlyBudget << '\n';

    for (const auto &e : expenses) {
        file << e.id << '|'
             << e.date << '|'
             << e.category << '|'
             << e.amount << '|'
             << e.note << '\n';
    }

    file.close();
}

void loadData() {
    ifstream file("expenses.txt");
    if (!file) return;

    file >> monthlyBudget;
    file.ignore(1000, '\n');

    string line;

    while (getline(file, line)) {
        size_t p1 = line.find('|');
        size_t p2 = line.find('|', p1 + 1);
        size_t p3 = line.find('|', p2 + 1);
        size_t p4 = line.find('|', p3 + 1);

        if (p1 == string::npos || p2 == string::npos ||
            p3 == string::npos || p4 == string::npos)
            continue;

        try {
            Expense e;
            e.id = stoi(line.substr(0, p1));
            e.date = line.substr(p1 + 1, p2 - p1 - 1);
            e.category = line.substr(p2 + 1, p3 - p2 - 1);
            e.amount = stod(line.substr(p3 + 1, p4 - p3 - 1));
            e.note = line.substr(p4 + 1);

            expenses.push_back(e);
            nextId = max(nextId, e.id + 1);
        } catch (...) {
            continue;
        }
    }

    file.close();
}

double monthlyTotal() {
    double total = 0;

    for (const auto &e : expenses) {
        if (e.date.substr(0, 7) == currentMonth())
            total += e.amount;
    }

    return total;
}

void addExpense() {
    Expense e;
    e.id = nextId++;
    e.date = todayDate();

    cout << "\nEnter category (Food/Travel/Books/Other): ";
    getline(cin, e.category);

    cout << "Enter amount (Rs): ";
    if (!(cin >> e.amount) || e.amount <= 0) {
        cout << "Invalid amount!\n";
        cin.clear();
        cin.ignore(10000, '\n');
        return;
    }
    cin.ignore(10000, '\n');

    cout << "Enter note: ";
    getline(cin, e.note);

    expenses.push_back(e);
    saveData();

    cout << "\nExpense added successfully!\n";
}

void viewExpenses() {
    cout << "\n========== EXPENSE HISTORY ==========\n";

    if (expenses.empty()) {
        cout << "No expenses recorded.\n";
        return;
    }

    cout << left << setw(6) << "ID"
         << setw(13) << "Date"
         << setw(15) << "Category"
         << setw(12) << "Amount"
         << "Note\n";

    for (const auto &e : expenses) {
        cout << left << setw(6) << e.id
             << setw(13) << e.date
             << setw(15) << e.category
             << fixed << setprecision(2)
             << setw(12) << e.amount
             << e.note << '\n';
    }
}

void categoryReport() {
    vector<string> categories;
    vector<double> totals;

    for (const auto &e : expenses) {
        if (e.date.substr(0, 7) != currentMonth())
            continue;

        auto it = find(categories.begin(), categories.end(), e.category);

        if (it == categories.end()) {
            categories.push_back(e.category);
            totals.push_back(e.amount);
        } else {
            int index = int(it - categories.begin());
            totals[index] += e.amount;
        }
    }

    cout << "\n======= CATEGORY REPORT =======\n";

    for (size_t i = 0; i < categories.size(); i++) {
        cout << categories[i] << ": Rs "
             << fixed << setprecision(2)
             << totals[i] << '\n';
    }

    if (categories.empty())
        cout << "No expenses for this month.\n";
}

void setBudget() {
    double amount;

    cout << "Enter monthly budget (Rs): ";

    if (!(cin >> amount) || amount <= 0) {
        cout << "Invalid budget!\n";
        cin.clear();
        cin.ignore(10000, '\n');
        return;
    }

    cin.ignore(10000, '\n');
    monthlyBudget = amount;
    saveData();

    cout << "Budget saved successfully!\n";
}

void budgetReport() {
    double spent = monthlyTotal();
    double remaining = monthlyBudget - spent;

    time_t now = time(nullptr);
    tm *t = localtime(&now);
    int day = t->tm_mday;

    int daysInMonth[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    int month = t->tm_mon;
    int days = daysInMonth[month];

    int year = t->tm_year + 1900;
    bool leap = (year % 400 == 0) ||
                (year % 4 == 0 && year % 100 != 0);

    if (month == 1 && leap)
        days = 29;

    double prediction = spent * days / day;

    cout << "\n======= SMART BUDGET REPORT =======\n";
    cout << fixed << setprecision(2);
    cout << "Monthly budget : Rs " << monthlyBudget << '\n';
    cout << "Total spent    : Rs " << spent << '\n';
    cout << "Budget left    : Rs " << remaining << '\n';
    cout << "Month-end estimate: Rs " << prediction << '\n';

    if (spent >= monthlyBudget)
        cout << "WARNING: Monthly budget reached or exceeded!\n";
    else if (spent >= monthlyBudget * 0.90)
        cout << "WARNING: You have used at least 90% of budget.\n";
    else if (prediction > monthlyBudget)
        cout << "ALERT: You may exceed your monthly budget.\n";
    else
        cout << "Status: You are currently within budget.\n";

    cout << "Suggested daily limit for remaining days: Rs "
         << max(0.0, remaining) / max(1, days - day)
         << '\n';
}

void searchExpense() {
    string keyword;

    cout << "Enter category or note to search: ";
    getline(cin, keyword);

    bool found = false;

    for (const auto &e : expenses) {
        if (e.category.find(keyword) != string::npos ||
            e.note.find(keyword) != string::npos) {
            cout << "ID: " << e.id
                 << " | Date: " << e.date
                 << " | Category: " << e.category
                 << " | Amount: Rs " << e.amount
                 << " | Note: " << e.note << '\n';
            found = true;
        }
    }

    if (!found)
        cout << "No matching expense found.\n";
}

void deleteExpense() {
    int id;
    cout << "Enter expense ID to delete: ";

    if (!(cin >> id)) {
        cout << "Invalid ID!\n";
        cin.clear();
        cin.ignore(10000, '\n');
        return;
    }

    cin.ignore(10000, '\n');

    auto it = find_if(expenses.begin(), expenses.end(),
        [id](const Expense &e) {
            return e.id == id;
        });

    if (it == expenses.end()) {
        cout << "Expense not found.\n";
        return;
    }

    expenses.erase(it);
    saveData();

    cout << "Expense deleted successfully!\n";
}

int main() {
    loadData();

    int choice;

    do {
        cout << "\n====================================\n";
        cout << "          SMARTSPEND AI\n";
        cout << "     Smart Digital Expense Tracker\n";
        cout << "====================================\n";
        cout << "1. Add Daily Expense\n";
        cout << "2. View Expense History\n";
        cout << "3. Category-wise Report\n";
        cout << "4. Set Monthly Budget\n";
        cout << "5. Budget Prediction\n";
        cout << "6. Search Expense\n";
        cout << "7. Delete Expense\n";
        cout << "8. Exit\n";
        cout << "Enter your choice: ";

        if (!(cin >> choice)) {
            cout << "Please enter a valid number.\n";
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        cin.ignore(10000, '\n');

        switch (choice) {
            case 1: addExpense(); break;
            case 2: viewExpenses(); break;
            case 3: categoryReport(); break;
            case 4: setBudget(); break;
            case 5: budgetReport(); break;
            case 6: searchExpense(); break;
            case 7: deleteExpense(); break;
            case 8:
                saveData();
                cout << "Thank you for using SmartSpend AI!\n";
                break;
            default:
                cout << "Invalid choice. Try again.\n";
        }

    } while (choice != 8);

    return 0;
}
