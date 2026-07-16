// todo-cli — a simple command-line to-do list manager.
//
// Single-file C++ program, standard library only.
// Build (MinGW / g++):  g++ -std=c++11 -Wall -O2 -o todo main.cpp
//
// Tasks are kept in memory while the program runs, loaded from tasks.txt on
// startup and written back on quit. Each line in the file is "doneFlag|description",
// e.g.  0|Buy milk   or   1|Walk the dog

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// A single to-do item.
struct Task {
    std::string description;
    bool done;
};

static const std::string DATA_FILE = "tasks.txt";

// ---------------------------------------------------------------------------
// Small input helpers
// ---------------------------------------------------------------------------

// Trim leading/trailing whitespace.
static std::string trim(const std::string& s) {
    const std::string ws = " \t\r\n";
    std::size_t start = s.find_first_not_of(ws);
    if (start == std::string::npos) return "";
    std::size_t end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

// Read one full line from stdin (whole-line input keeps things simple and
// avoids the classic cin >> / getline newline pitfalls).
static std::string readLine() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

// Parse a whole integer from a string. Returns false if it isn't a clean number.
static bool parseInt(const std::string& s, int& out) {
    std::string t = trim(s);
    if (t.empty()) return false;
    std::istringstream iss(t);
    int value;
    iss >> value;
    if (iss.fail()) return false;
    char leftover;
    if (iss >> leftover) return false; // trailing junk -> not a clean integer
    out = value;
    return true;
}

// Pause so the user can read the result before the menu redraws.
// (Not named "pause" because POSIX <unistd.h> already declares pause().)
static void waitForEnter() {
    std::cout << "\n  (press Enter to continue) ";
    std::string discard;
    std::getline(std::cin, discard);
}

// ---------------------------------------------------------------------------
// Persistence
// ---------------------------------------------------------------------------

// Load tasks from DATA_FILE. A missing or empty file simply yields an empty
// list — that's the normal first-run case, not an error.
static std::vector<Task> loadTasks() {
    std::vector<Task> tasks;
    std::ifstream in(DATA_FILE.c_str());
    if (!in) return tasks; // no file yet

    std::string line;
    while (std::getline(in, line)) {
        if (trim(line).empty()) continue;          // skip blank lines

        std::size_t sep = line.find('|');
        if (sep == std::string::npos) continue;    // skip malformed lines

        std::string flag = line.substr(0, sep);
        std::string desc = line.substr(sep + 1);   // keeps any '|' in the text
        if (desc.empty()) continue;

        Task t;
        t.done = (trim(flag) == "1");
        t.description = desc;
        tasks.push_back(t);
    }
    return tasks;
}

// Save all tasks back to DATA_FILE, overwriting it.
static bool saveTasks(const std::vector<Task>& tasks) {
    std::ofstream out(DATA_FILE.c_str(), std::ios::trunc);
    if (!out) return false;
    for (std::size_t i = 0; i < tasks.size(); ++i) {
        out << (tasks[i].done ? "1" : "0") << "|" << tasks[i].description << "\n";
    }
    return true;
}

// ---------------------------------------------------------------------------
// UI
// ---------------------------------------------------------------------------

static void printBanner() {
    std::cout << "\n";
    std::cout << "  ============================================\n";
    std::cout << "                MY TO-DO LIST\n";
    std::cout << "  ============================================\n";
}

static void printMenu() {
    std::cout << "\n";
    std::cout << "  --------------------------------------------\n";
    std::cout << "    1.  Add task\n";
    std::cout << "    2.  List tasks\n";
    std::cout << "    3.  Mark task complete\n";
    std::cout << "    4.  Delete task\n";
    std::cout << "    5.  Quit\n";
    std::cout << "  --------------------------------------------\n";
    std::cout << "  Choose an option (1-5): ";
}

// Print the numbered task list with [ ] / [x] markers and a small summary.
static void listTasks(const std::vector<Task>& tasks) {
    std::cout << "\n";
    if (tasks.empty()) {
        std::cout << "  Your list is empty - add a task to get started!\n";
        return;
    }

    int doneCount = 0;
    std::cout << "  Your tasks:\n";
    std::cout << "  --------------------------------------------\n";
    for (std::size_t i = 0; i < tasks.size(); ++i) {
        const Task& t = tasks[i];
        if (t.done) ++doneCount;
        std::cout << "   " << (i + 1) << ". [" << (t.done ? 'x' : ' ') << "] "
                  << t.description << "\n";
    }
    std::cout << "  --------------------------------------------\n";
    std::cout << "  Total: " << tasks.size()
              << "  |  Done: " << doneCount
              << "  |  Pending: " << (tasks.size() - doneCount) << "\n";
}

// ---------------------------------------------------------------------------
// Actions
// ---------------------------------------------------------------------------

static void addTask(std::vector<Task>& tasks) {
    std::cout << "\n  Enter task description: ";
    std::string desc = trim(readLine());
    if (desc.empty()) {
        std::cout << "  ! Empty description - task not added.\n";
        return;
    }
    Task t;
    t.description = desc;
    t.done = false;
    tasks.push_back(t);
    std::cout << "  + Added: \"" << desc << "\"\n";
}

// Shared helper for the "mark complete" and "delete" flows: show the list and
// ask the user to pick a task number (0 cancels). Returns a 0-based index, or
// -1 if the user cancelled / entered something invalid.
static int pickTask(const std::vector<Task>& tasks, const std::string& verb) {
    if (tasks.empty()) {
        std::cout << "\n  There are no tasks to " << verb << " yet.\n";
        return -1;
    }
    listTasks(tasks);
    std::cout << "\n  Enter the task number to " << verb << " (0 to cancel): ";

    int n;
    if (!parseInt(readLine(), n)) {
        std::cout << "  ! That isn't a valid number.\n";
        return -1;
    }
    if (n == 0) {
        std::cout << "  Cancelled.\n";
        return -1;
    }
    if (n < 1 || n > static_cast<int>(tasks.size())) {
        std::cout << "  ! No task with that number.\n";
        return -1;
    }
    return n - 1;
}

static void markComplete(std::vector<Task>& tasks) {
    int i = pickTask(tasks, "mark complete");
    if (i < 0) return;
    if (tasks[i].done) {
        std::cout << "  \"" << tasks[i].description << "\" is already complete.\n";
    } else {
        tasks[i].done = true;
        std::cout << "  [x] Marked complete: \"" << tasks[i].description << "\"\n";
    }
}

static void deleteTask(std::vector<Task>& tasks) {
    int i = pickTask(tasks, "delete");
    if (i < 0) return;
    std::string removed = tasks[i].description;
    tasks.erase(tasks.begin() + i);
    std::cout << "  - Deleted: \"" << removed << "\"\n";
}

// ---------------------------------------------------------------------------
// Main loop
// ---------------------------------------------------------------------------

int main() {
    std::vector<Task> tasks = loadTasks();

    printBanner();
    std::cout << "  Loaded " << tasks.size() << " task(s) from " << DATA_FILE << ".\n";

    bool running = true;
    while (running) {
        printMenu();

        std::string choiceLine = readLine();
        if (std::cin.eof()) {           // input stream closed (e.g. piped input)
            std::cout << "\n";
            break;
        }

        int choice;
        if (!parseInt(choiceLine, choice)) {
            std::cout << "\n  ! Please enter a number from 1 to 5.\n";
            waitForEnter();
            continue;
        }

        switch (choice) {
            case 1: addTask(tasks);     waitForEnter(); break;
            case 2: listTasks(tasks);   waitForEnter(); break;
            case 3: markComplete(tasks); waitForEnter(); break;
            case 4: deleteTask(tasks);  waitForEnter(); break;
            case 5: running = false;    break;
            default:
                std::cout << "\n  ! Please choose a number from 1 to 5.\n";
                waitForEnter();
        }
    }

    if (saveTasks(tasks)) {
        std::cout << "  Saved " << tasks.size() << " task(s) to " << DATA_FILE << ".\n";
    } else {
        std::cout << "  ! Warning: could not write to " << DATA_FILE << ".\n";
    }
    std::cout << "  Goodbye!\n\n";
    return 0;
}
