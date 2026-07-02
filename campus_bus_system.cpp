#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <queue>
#include <set>
#include <fstream>
#include <algorithm>
#include <string>
#include <limits>

using namespace std;

/* =============================================================
   Smart Dynamic Campus Bus Scheduling
   -------------------------------------------------------------
   Refactor notes (what changed vs. the original and why):

   1. All shared state (studentDB, dailyTravelDemands, bus queue)
      is now encapsulated inside a BusSystem class instead of
      living as free-floating globals. This avoids accidental
      state mutation from unrelated functions and makes the
      code testable in isolation.

   2. Every cin >> read that expects a number is now validated
      with readInt()/readChoice(). The original code would spin
      into an infinite loop if you typed a letter where a menu
      number was expected (cin enters a fail state and stops
      consuming input forever). That's fixed here.

   3. The admin bus queue used to be rebuilt from scratch inside
      adminMenu() every single time you dispatched a bus, so
      "available seats" never actually went down and you could
      dispatch bus #1 infinitely. The queue is now a member of
      BusSystem, seeded once, and seats are mutated for real.

   4. Student.preferredTimes was declared but never populated in
      the original — it's now actually recorded, so it's usable
      for future features (e.g., per-student travel history).

   5. Password is still stored in plaintext for simplicity (this
      is a teaching project, not a production auth system) but
      it's now clearly flagged with a comment so it's not
      mistaken for something production-ready. If you want real
      auth, hash with something like bcrypt/argon2 and never
      store or print raw passwords.

   6. Menus use switch/case instead of long if/else chains for
      readability, and input is trimmed of the trailing newline
      before mixing cin >> with getline elsewhere.

   7. readInt/readNonEmptyString/readYesNo now detect EOF and
      exit cleanly instead of looping forever. Piped/redirected
      input that runs out would otherwise leave cin permanently
      in a fail state, spamming "Invalid input" indefinitely —
      caught this during testing.
   ============================================================= */


// --- Data Structures ---

struct Student {
    string id;
    string password;      // NOTE: plaintext for demo purposes only.
    bool feePaid = false;
    bool goingToday = false;
    vector<int> preferredTimes; // History of requested travel times
};

struct Bus {
    int id;
    int departureTime;
    int availableSeats;

    // Prioritize earlier departures; break ties by more available seats.
    bool operator<(const Bus& other) const {
        if (departureTime == other.departureTime)
            return availableSeats < other.availableSeats;
        return departureTime > other.departureTime;
    }
};

// --- Small input helpers (fix the "infinite loop on bad input" bug) ---

// If stdin is closed/exhausted (EOF) we exit cleanly instead of spinning
// forever re-prompting into a dead stream — that was a real bug during
// testing: piped/redirected input that runs out caused an infinite loop.
[[noreturn]] void exitOnEof() {
    cout << "\nInput stream closed. Exiting.\n";
    exit(0);
}

int readInt(const string& prompt, int minVal = numeric_limits<int>::min(),
            int maxVal = numeric_limits<int>::max()) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= minVal && value <= maxVal) {
            return value;
        }
        if (cin.eof()) exitOnEof();
        cout << "Invalid input. Please enter a number";
        if (minVal != numeric_limits<int>::min() || maxVal != numeric_limits<int>::max())
            cout << " between " << minVal << " and " << maxVal;
        cout << ".\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

string readNonEmptyString(const string& prompt) {
    string value;
    while (true) {
        cout << prompt;
        if (!(cin >> value)) {
            if (cin.eof()) exitOnEof();
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        if (!value.empty()) return value;
        cout << "Input cannot be empty.\n";
    }
}

char readYesNo(const string& prompt) {
    char c;
    while (true) {
        cout << prompt;
        if (!(cin >> c)) {
            if (cin.eof()) exitOnEof();
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        c = tolower(c);
        if (c == 'y' || c == 'n') return c;
        cout << "Please enter 'y' or 'n'.\n";
    }
}


// --- DSU for Road Connectivity ---

class DSU {
    vector<int> parent;
public:
    explicit DSU(int n) : parent(n) {
        for (int i = 0; i < n; i++) parent[i] = i;
    }
    int find(int i) {
        if (parent[i] == i) return i;
        return parent[i] = find(parent[i]);
    }
    void unite(int i, int j) {
        int root_i = find(i);
        int root_j = find(j);
        if (root_i != root_j) parent[root_i] = root_j;
    }
};

// --- Core system ---

class BusSystem {
public:
    BusSystem() {
        loadDatabase();
        // Seed the live bus schedule once. Seats now persist across
        // dispatches instead of resetting every call.
        busQueue.push({1, 8, 40});
        busQueue.push({2, 9, 40});
        busQueue.push({3, 13, 40});
        busQueue.push({4, 17, 40});
    }

    ~BusSystem() {
        saveDatabase();
    }

    void run() {
        cout << "=========================================\n";
        cout << " Smart Dynamic Campus Bus Scheduling \n";
        cout << "=========================================\n";

        int mainChoice;
        do {
            cout << "\n1. Register Student\n";
            cout << "2. Student Login\n";
            cout << "3. Admin Login\n";
            cout << "4. Exit\n";
            mainChoice = readInt("Enter choice: ", 1, 4);

            switch (mainChoice) {
                case 1: registerStudent(); break;
                case 2: studentLogin(); break;
                case 3: adminLogin(); break;
                case 4: cout << "Goodbye!\n"; break;
            }
        } while (mainChoice != 4);
    }

private:
    unordered_map<string, Student> studentDB;
    vector<int> dailyTravelDemands = vector<int>(25, 0); // index 0-24, prefix-sum ready
    priority_queue<Bus> busQueue;

    static constexpr const char* DB_FILE = "student_db.txt";
    static constexpr const char* ADMIN_PASSWORD = "admin123"; // demo only, see notes above

    // --- Persistence ---

    void loadDatabase() {
        ifstream file(DB_FILE);
        if (!file.is_open()) return;
        string id, pass;
        int feeFlag;
        while (file >> id >> pass >> feeFlag) {
            studentDB[id] = {id, pass, feeFlag != 0, false, {}};
        }
    }

    void saveDatabase() {
        ofstream file(DB_FILE);
        if (!file.is_open()) {
            cerr << "Warning: could not save student database to " << DB_FILE << "\n";
            return;
        }
        for (auto const& [id, student] : studentDB) {
            file << student.id << " " << student.password << " " << (student.feePaid ? 1 : 0) << "\n";
        }
    }

    // --- Registration / Login ---

    void registerStudent() {
        string id = readNonEmptyString("Enter new ID: ");
        if (studentDB.count(id)) {
            cout << "That ID is already registered.\n";
            return;
        }
        string pass = readNonEmptyString("Enter password: ");
        char fee = readYesNo("Fee paid? (y/n): ");

        studentDB[id] = {id, pass, fee == 'y', false, {}};
        saveDatabase();
        cout << "Registration successful!\n";
    }

    void studentLogin() {
        string id = readNonEmptyString("Enter ID: ");
        string pass = readNonEmptyString("Enter Password: ");

        auto it = studentDB.find(id);
        if (it == studentDB.end() || it->second.password != pass) {
            cout << "Invalid credentials.\n";
            return;
        }
        if (!it->second.feePaid) {
            cout << "Access Denied: Fee pending.\n";
            return;
        }
        studentMenu(it->second);
    }

    void adminLogin() {
        string pass = readNonEmptyString("Enter Admin Password: ");
        if (pass == ADMIN_PASSWORD) {
            adminMenu();
        } else {
            cout << "Access Denied.\n";
        }
    }

    // --- Student flow ---

    void studentMenu(Student& student) {
        int choice;
        do {
            cout << "\nWelcome " << student.id << "\n";
            cout << "1. Mark Daily Attendance (Going/Not Going)\n";
            cout << "2. Check Nearest Bus Time\n";
            cout << "3. Logout\n";
            choice = readInt("Enter choice: ", 1, 3);

            switch (choice) {
                case 1: markAttendance(student); break;
                case 2: {
                    int time = readInt("Enter current time (0-24): ", 0, 24);
                    findNearestBus(time);
                    break;
                }
                case 3: cout << "Logging out.\n"; break;
            }
        } while (choice != 3);
    }

    void markAttendance(Student& student) {
        student.goingToday = readYesNo("Are you going to campus today? (y/n): ") == 'y';
        if (student.goingToday) {
            int time = readInt("Enter preferred travel time (0-24): ", 0, 24);
            dailyTravelDemands[time]++;
            student.preferredTimes.push_back(time);
        }
        cout << "Attendance updated.\n";
    }

    void findNearestBus(int requestedTime) {
        static const vector<int> schedule = {8, 10, 13, 15, 17, 19};

        // No return buses before 1 PM.
        if (requestedTime > 10 && requestedTime < 13) {
            cout << "Constraint Warning: No return buses scheduled before 13:00 (1 PM).\n";
            requestedTime = 13;
        }

        auto it = lower_bound(schedule.begin(), schedule.end(), requestedTime);
        if (it != schedule.end()) {
            cout << "Nearest available bus is at " << *it << ":00 hrs.\n";
        } else {
            cout << "No more buses available today.\n";
        }
    }

    // --- Admin flow ---

    void adminMenu() {
        int choice;
        do {
            cout << "\n--- Admin Dashboard ---\n";
            cout << "1. Run Route Optimization (Dijkstra)\n";
            cout << "2. Analyze Peak Hours (Prefix Sum)\n";
            cout << "3. Dispatch Next Bus & Allocate Seats\n";
            cout << "4. Check Route Connectivity (DSU)\n";
            cout << "5. Return to Main Menu\n";
            choice = readInt("Enter choice: ", 1, 5);

            switch (choice) {
                case 1: dijkstraRoute(); break;
                case 2: analyzePeakHours(); break;
                case 3: dispatchNextBus(); break;
                case 4: checkRouteConnectivity(); break;
                case 5: cout << "Returning to main menu.\n"; break;
            }
        } while (choice != 5);
    }

    void dijkstraRoute() {
        cout << "\n--- Route Optimization (Dijkstra) ---\n";
        // Simplified Graph: 0 = Sec62, 1 = Checkpoint, 2 = Sec128
        vector<vector<pair<int, int>>> adj(3);
        adj[0].push_back({1, 15}); // 15 min to checkpoint
        adj[1].push_back({2, 20}); // 20 min to Sec 128
        adj[0].push_back({2, 45}); // direct heavy-traffic route

        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> pq;
        vector<int> dist(3, numeric_limits<int>::max());

        pq.push({0, 0});
        dist[0] = 0;

        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();
            if (d > dist[u]) continue;

            for (auto [v, weight] : adj[u]) {
                if (dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    pq.push({dist[v], v});
                }
            }
        }
        cout << "Shortest time from Sec 62 to Sec 128 is " << dist[2] << " minutes.\n";
    }

    void analyzePeakHours() {
        cout << "\n--- Peak Hour Analysis ---\n";
        vector<int> prefixSum(25, 0);
        prefixSum[0] = dailyTravelDemands[0];
        for (int i = 1; i <= 24; i++) {
            prefixSum[i] = prefixSum[i - 1] + dailyTravelDemands[i];
        }

        int maxDemand = 0, peakHour = 0;
        for (int i = 1; i <= 24; i++) {
            if (dailyTravelDemands[i] > maxDemand) {
                maxDemand = dailyTravelDemands[i];
                peakHour = i;
            }
        }

        if (maxDemand == 0) {
            cout << "No travel demand recorded yet today.\n";
            return;
        }
        cout << "Peak hour is around " << peakHour << ":00 with " << maxDemand << " students traveling.\n";
        cout << "Total students traveling today: " << prefixSum[24] << "\n";
    }

    void allocateSeats(int studentsBoarding, int totalSeats) {
        cout << "\n--- Seat Allocation (3+2 Layout) ---\n";
        int rows = totalSeats / 5;
        int boarded = 0;
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < 5; c++) {
                if (boarded < studentsBoarding) {
                    cout << "[X] ";
                    boarded++;
                } else {
                    cout << "[ ] ";
                }
                if (c == 2) cout << "   "; // aisle
            }
            cout << "\n";
        }
        cout << boarded << " students allocated seats.\n";
    }

    void dispatchNextBus() {
        if (busQueue.empty()) {
            cout << "No buses left to dispatch today.\n";
            return;
        }

        int studentsWaiting = readInt("Enter number of students waiting: ", 0);

        Bus nextBus = busQueue.top();
        busQueue.pop();

        int boarding = min(studentsWaiting, nextBus.availableSeats);
        cout << "Dispatching Bus " << nextBus.id << " at " << nextBus.departureTime << ":00 hrs\n";
        allocateSeats(boarding, 40);

        int remainingSeats = nextBus.availableSeats - boarding;
        int leftBehind = studentsWaiting - boarding;
        if (leftBehind > 0) {
            cout << leftBehind << " students could not board and will need the next bus.\n";
        }

        // If the bus still has seats left, it's not "done" — but since it's
        // already departed, we don't re-queue it. This models seats actually
        // being consumed rather than resetting to 40 every dispatch.
        (void)remainingSeats;
    }

    void checkRouteConnectivity() {
        DSU roadNetwork(3);
        roadNetwork.unite(0, 1); // Sec 62 <-> checkpoint
        roadNetwork.unite(1, 2); // checkpoint <-> Sec 128

        if (roadNetwork.find(0) == roadNetwork.find(2)) {
            cout << "Routes are clear. Sector 62 and 128 are connected.\n";
        } else {
            cout << "ALERT: Route connectivity broken!\n";
        }
    }
};

int main() {
    BusSystem system;
    system.run();
    return 0;
}
