Campus Bus Route Optimizer

A console-based C++ system for managing a campus bus network — student registration and attendance, route optimization, peak-hour demand analysis, and dynamic bus dispatch with seat allocation.

Features


Student registration & login with fee-status gating (students with pending fees can't book a bus)
Daily attendance tracking — students mark whether they're traveling and their preferred time, feeding into demand analytics
Route optimization using Dijkstra's algorithm to compute shortest travel time between campus checkpoints
Peak-hour analysis using a prefix-sum over hourly travel demand to identify the busiest hour of the day
Dynamic bus dispatch via a priority queue (earliest departure first, tie-broken by available seats), with seats that persist and deplete across dispatches rather than resetting
Seat allocation visualization — prints a realistic 3+2 bus seating layout showing occupied vs. empty seats
Route connectivity checks using a Disjoint Set Union (DSU) structure to confirm all checkpoints are reachable
Nearest-bus lookup via binary search over the daily schedule, respecting a no-return-before-1PM constraint
Persistent student database saved to and loaded from a local text file


Tech Stack


Language: C++17
Data structures used: unordered_map, priority_queue, DSU (Union-Find), adjacency list graph
Algorithms used: Dijkstra's shortest path, prefix sum, binary search (lower_bound)
No external dependencies — standard library only


Getting Started

Prerequisites

A C++17-compatible compiler (e.g. g++ 7+, MSVC, or Clang).

Build

bashg++ -std=c++17 -Wall -Wextra -o campus_bus_system campus_bus_system.cpp

Run

bash./campus_bus_system

On Windows (PowerShell):

powershellg++ -std=c++17 -Wall -Wextra -o campus_bus_system.exe campus_bus_system.cpp
.\campus_bus_system.exe

Usage

On launch you'll see the main menu:

1. Register Student
2. Student Login
3. Admin Login
4. Exit

As a student, register with an ID, password, and fee status. Once logged in (fee must be marked paid), you can:


Mark today's attendance and preferred travel time
Check the nearest scheduled bus for a given time


As an admin (default demo password: admin123), you get access to:


Route optimization (Dijkstra)
Peak-hour demand analysis
Bus dispatch with live seat allocation
Route connectivity check (DSU)


Student data is saved to student_db.txt in the working directory and reloaded automatically on the next run.

Project Structure

.
├── campus_bus_system.cpp   # Full application source
├── student_db.txt          # Auto-generated student database (created at runtime)
└── README.md

Notes


The admin password and student passwords are stored in plaintext — this is a class/demo project, not a production auth system. For real deployment, passwords should be hashed (e.g. bcrypt/argon2) and never stored or logged in plaintext.
The bus schedule, checkpoints, and graph weights are hardcoded sample data meant to demonstrate the underlying algorithms; swapping in real route/schedule data would be a natural next step.


License

This project is open source and available for educational use.
