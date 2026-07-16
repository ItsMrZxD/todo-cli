// Unit tests for the helpers in main.cpp. No framework, standard library only,
// matching the spirit of the project.
//
// Build & run (from the repo root):
//   g++ -std=c++11 -Wall -Wextra -O2 -o run_tests tests/test_main.cpp
//   ./run_tests
//
// main.cpp is deliberately a single file, so the tests #include it directly
// and rename its main() out of the way.
#define main todo_cli_main
#include "../main.cpp"
#undef main

#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

static int checks = 0;
static int failures = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        ++checks;                                                          \
        if (!(cond)) {                                                     \
            ++failures;                                                    \
            std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond);    \
        }                                                                  \
    } while (0)

// loadTasks/saveTasks read and write "tasks.txt" in the current directory,
// so move into a scratch directory to avoid touching anyone's real list.
static void enterScratchDir() {
#ifdef _WIN32
    _mkdir("test_scratch");
    if (_chdir("test_scratch") != 0) {
#else
    mkdir("test_scratch", 0777);
    if (chdir("test_scratch") != 0) {
#endif
        std::printf("FATAL: could not enter test_scratch directory\n");
        std::exit(1);
    }
}

static void testTrim() {
    CHECK(trim("  hi  ") == "hi");
    CHECK(trim("\t\r\nx\t") == "x");
    CHECK(trim("") == "");
    CHECK(trim(" \t ") == "");
    CHECK(trim("a  b") == "a  b"); // inner whitespace survives
}

static void testParseInt() {
    int v = 0;
    CHECK(parseInt("42", v) && v == 42);
    CHECK(parseInt("  7  ", v) && v == 7);
    CHECK(parseInt("-3", v) && v == -3);
    CHECK(parseInt("0", v) && v == 0);
    CHECK(!parseInt("", v));
    CHECK(!parseInt("   ", v));
    CHECK(!parseInt("abc", v));
    CHECK(!parseInt("12abc", v));
    CHECK(!parseInt("1 2", v));
    CHECK(!parseInt("4.5", v));
}

static void testMissingFileGivesEmptyList() {
    std::remove(DATA_FILE.c_str());
    CHECK(loadTasks().empty());
}

static void testSaveLoadRoundTrip() {
    std::vector<Task> tasks;
    Task a; a.description = "Buy milk";        a.done = false; tasks.push_back(a);
    Task b; b.description = "Walk the dog";    b.done = true;  tasks.push_back(b);
    Task c; c.description = "Ship v1|then v2"; c.done = false; tasks.push_back(c);

    CHECK(saveTasks(tasks));

    std::vector<Task> loaded = loadTasks();
    CHECK(loaded.size() == 3);
    if (loaded.size() == 3) {
        CHECK(loaded[0].description == "Buy milk" && !loaded[0].done);
        CHECK(loaded[1].description == "Walk the dog" && loaded[1].done);
        // Only the first '|' separates the flag; the rest is description.
        CHECK(loaded[2].description == "Ship v1|then v2" && !loaded[2].done);
    }
}

static void testLoadSkipsMalformedLines() {
    std::ofstream out(DATA_FILE.c_str(), std::ios::trunc);
    out << "\n";                    // blank line
    out << "   \t\n";               // whitespace-only line
    out << "no separator here\n";   // no '|'
    out << "1|\n";                  // empty description
    out << "0|the only good line\n";
    out << " 1 |flag gets trimmed\n";
    out.close();

    std::vector<Task> loaded = loadTasks();
    CHECK(loaded.size() == 2);
    if (loaded.size() == 2) {
        CHECK(loaded[0].description == "the only good line" && !loaded[0].done);
        CHECK(loaded[1].description == "flag gets trimmed" && loaded[1].done);
    }
}

int main() {
    testTrim();
    testParseInt();

    enterScratchDir();
    testMissingFileGivesEmptyList();
    testSaveLoadRoundTrip();
    testLoadSkipsMalformedLines();

    std::printf("%d checks, %d failure(s)\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
