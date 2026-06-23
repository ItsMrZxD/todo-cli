# todo-cli

A small, dependency-free command-line to-do list manager written in C++.

It runs entirely in the terminal with a simple numbered menu, and remembers your
tasks between runs by saving them to a plain-text file.

## Features

- Menu-driven loop: **add**, **list**, **mark complete**, **delete**, and **quit**
- Tasks stored in memory as a list of `{ description, done }` items
- Automatic persistence — loads from `tasks.txt` on startup, saves on quit
- Clear numbered list with `[ ]` / `[x]` markers and a done/pending summary
- Robust input handling (bad input never crashes the program)
- Handles a missing or empty `tasks.txt` gracefully on first run

## Requirements

- A C++ compiler. Built and tested with **g++ (MinGW GCC 6.3.0)**.
- No external libraries — standard library only.

## Build

From the project folder:

```sh
g++ -std=c++11 -Wall -O2 -o todo main.cpp
```

This produces `todo.exe` (or `todo` on Linux/macOS).

## Run

```sh
./todo
```

You'll see a menu like this:

```
  ============================================
                MY TO-DO LIST
  ============================================
  Loaded 2 task(s) from tasks.txt.

  --------------------------------------------
    1.  Add task
    2.  List tasks
    3.  Mark task complete
    4.  Delete task
    5.  Quit
  --------------------------------------------
  Choose an option (1-5):
```

Listing tasks looks like:

```
  Your tasks:
  --------------------------------------------
   1. [ ] Buy milk
   2. [x] Walk the dog
  --------------------------------------------
  Total: 2  |  Done: 1  |  Pending: 1
```

## Data format

Tasks are stored in `tasks.txt`, one per line, as `doneFlag|description`:

```
0|Buy milk
1|Walk the dog
```

- `doneFlag` is `1` for completed tasks and `0` otherwise.
- Everything after the first `|` is the task description (so descriptions may
  themselves contain `|`).
- The file is created automatically the first time you quit, and `tasks.txt` is
  git-ignored since it's your personal data.

## Project structure

```
todo-cli/
├── main.cpp      # the entire program
├── README.md
└── .gitignore
```
