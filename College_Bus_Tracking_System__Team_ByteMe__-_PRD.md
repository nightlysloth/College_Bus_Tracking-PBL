# College Bus Tracking System: Product Requirements Document

**Team:** ByteMe | **Language:** C++ (C++11 or later) | **Interface:** Terminal (CLI) | **Target size:** 1000 to 1200 lines of code

---

## 1. Overview

The College Bus Tracking System is an offline, single-machine, terminal-based C++ application for students of Graphic Era (Deemed to be University / Hill University). A student can search a station, a bus or a route and instantly see which bus arrives first, its ETA, how many seats are free, and the full route.

There is no real GPS. Live tracking is **simulated** on top of the real college bus chart (52 buses, all ending at Geu/Gehu). A simulated clock moves buses along their real stop sequences, so ETA and seat availability change as time moves.

The project also serves as a demonstration of DSA (hand-built Stack, Queue, Linked List, Hash Table; BFS, DFS, Dijkstra; sorting) and OOP (classes, inheritance, polymorphism, operator overloading, templates, file handling, exception handling) in one working system.

## 2. Problem Statement

- Students do not know when their bus will reach their stop, whether it has free seats, or which of several buses at a stop is best.
- This information exists only in drivers' heads or in a printed/spreadsheet chart.
- At peak hours (7:00 to 8:10 AM) several routes converge on shared stops such as Kargi Chowk, Rispana, ISBT and GMS Road, which makes the confusion worse.

## 3. Goals and Non-Goals

**Goals**

1. Answer "which bus should I catch from my stop right now?" in under 3 menu steps.
2. Use the real route chart data, loaded from a text file, not hard-coded.
3. Use hand-built data structures and algorithms (no STL containers in the core).
4. Stay within roughly 1100 lines so every member can explain every line.

**Non-Goals**

- No GUI, no database, no network, no real GPS, no login/authentication, no multithreading.
- No booking or payments. Seat availability is informational only.

## 4. Users and User Stories

The only user type is the **student** (no admin role).

| ID  | As a student I want to...                                           | So that...                        |
| --- | ------------------------------------------------------------------- | --------------------------------- |
| US1 | search my station by name (even partially)                          | I can find it quickly             |
| US2 | see all buses passing my station in arrival order                   | I know what comes next            |
| US3 | see ETA and free seats of each bus                                  | I can pick one with space         |
| US4 | get one recommended bus                                             | I do not have to compare manually |
| US5 | look up a bus number and see where it is now                        | I can check my usual bus          |
| US6 | see the full route of a bus                                         | I know all its stops              |
| US7 | find the fastest way from a station to campus (or between stations) | I can plan alternatives           |
| US8 | advance the simulated clock                                         | I can see buses moving            |

## 5. Functional Requirements

| ID   | Requirement                                                                                                 | Priority |
| ---- | ----------------------------------------------------------------------------------------------------------- | -------- |
| FR1  | Load buses and routes from `buses.txt` and edge-time overrides from `edges.txt` at startup                  | Must     |
| FR2  | Station search: exact (via Hash Table) and prefix suggestions if not found                                  | Must     |
| FR3  | Bus search by bus number: show registration, driver, departure time, status, current stop, next stop, seats | Must     |
| FR4  | Route search: by bus number or by origin and destination station                                            | Must     |
| FR5  | Arrival order at a station: buses sorted by ETA, shown through a Queue                                      | Must     |
| FR6  | ETA computed dynamically from the simulated clock                                                           | Must     |
| FR7  | Seat availability: capacity, occupied, free, and a status (Plenty / Filling / Full)                         | Must     |
| FR8  | Bus recommendation using the scoring rule in section 12                                                     | Must     |
| FR9  | Simulated live tracking: clock advance by +1, +5, +10 minutes or set time                                   | Must     |
| FR10 | Shortest-time path between two stations (Dijkstra)                                                          | Must     |
| FR11 | Minimum number of stops between two stations (BFS)                                                          | Must     |
| FR12 | Connectivity check, and list of stations reachable from a station (DFS)                                     | Should   |
| FR13 | Sorting demo: sort the bus list by ETA, bus number or free seats with a chosen algorithm                    | Should   |
| FR14 | Dashboard: current time, buses running / not started / reached, busiest stations                            | Should   |
| FR15 | Save a text report of a station's arrival board to `report.txt` (file output)                               | Could    |
| FR16 | Friendly errors for bad input and bad data (exceptions), never a crash                                      | Must     |

## 6. Data

### 6.1 Source and files

The bus route chart images provided by the college (52 buses, numbered 1 to 56 with some numbers unused; departure times 7:00, 7:10, 7:35, 7:50, 8:00, 8:10 and 8:30 AM). Every route ends at **Geu** (the chart writes some as `Geu/Gehu`; the data uses `Geu`). Driver phone numbers are personal data and are **not** copied into the program; keep only driver names.

The chart is stored as CSV files in a `data/` folder next to the executable. The program reads them at startup.

| File | Rows | Purpose |
|------|------|---------|
| `buses.csv` | 52 | One row per bus: details plus its full ordered route (main file the loader needs) |
| `stations.csv` | 93 | One row per unique station, with its id and how many buses serve it |
| `route_stops.csv` | 370 | The same routes, one stop per row (optional alternative to the `stops` column) |
| `edges.csv` | 2 | Optional travel-time overrides between two stations, in minutes |

All files have a header row, use `,` as the field separator, and contain no quoted fields (no station or driver name contains a comma).

### 6.2 File `buses.csv`

```
bus_no,registration,departure,driver,type,stop_count,stops
```

| Column | Meaning |
|--------|---------|
| `bus_no` | Bus number from the chart (unique) |
| `registration` | Registration number without spaces, for example `UK07PA1688` |
| `departure` | Departure from the first stop, `HH:MM` (24-hour) |
| `driver` | Driver name only |
| `type` | `S` = standard bus (50 seats), `M` = mini bus (32 seats). Not on the chart: assigned `M` when the route has 4 or fewer stations, otherwise `S`. Editable |
| `stop_count` | Number of stations in the route, including `Geu` (used to validate the row) |
| `stops` | Stations in travel order separated by `;`, always ending with `Geu` |

Sample rows (taken from the actual file):

```
2,UK07S2788,08:00,Shankar Thapa,S,5,Prince Chowk;Saharanpur Chowk;Matawala Bagh;ISBT;Geu
18,UK07PA1696,08:00,Krishan Kumar,M,4,Saharanpur Chowk;Matawala Bagh;ISBT;Geu
20,UK07PA2190,08:00,Sunil Kumar,S,8,Garhi Cantt;Vijay Coloney;Chir Bagh;CM House;ONGC Chowk;Ballupur Chowk;GMS Road;Geu
17,UK07PA1694,08:00,Rama,M,3,Ballupur Chowk;GMS Road;Geu
51,UK07PA4119,07:00,Manjeet,M,3,Kulhal;ISBT;Geu
47,UK07PA3848,07:10,Rohit Singh,S,14,Vikas Nagar;Tehsil;Harbatpur Chowk;Langa Road;Shaspur;Selaqui Chowk;Sudhowala;Nanda Ki Chowki;Premnagar;Panditwari;Vasant Vihar;Baliwala Chowk;GMS Road;Geu
6,UK07PA2341,07:35,Kamlesh Kumar,S,11,Ranipokhri;Doiwala;Lachiwala;Kuanwala;Harawala;Miawala;Mokampur;Jogiwala;Rispana;ISBT;Geu
40,UK07PA3345,07:50,Vishesh Tyagi,S,9,Gujuraunwala;Ranjawala;Raipur Chowk;6 No. Pulia;Ring Road;Fountain Chowk;Rispana;ISBT;Geu
36,UK07PA2621,08:00,Jyoti Tiwari,S,7,Fountain Chowk;Nehru Coloney;Chanchal Dairy;Rispana;Kargi Chowk;ISBT;Geu
48,UK07PA3849,08:00,Sukhbir Singh,M,4,Rispana;Kargi Chowk;ISBT;Geu
```

Loading rule: split each line on `,` into 7 fields, then split the last field on `;`. A row is skipped with a message if it does not have 7 fields, the time is not `HH:MM`, `type` is not `S` or `M`, or `stop_count` does not match the number of stops.

### 6.3 File `stations.csv`

```
station_id,station_name,buses_serving
```

`station_id` runs from 1 to 93 in order of first appearance in `buses.csv`, and is the id used by the `HashTable`, the `Graph` and `Route`. `buses_serving` is the number of buses that stop there and is shown on the dashboard. The first rows of the file:

```
1,Prince Chowk,1
2,Saharanpur Chowk,4
3,Matawala Bagh,3
4,ISBT,39
5,Geu,52
6,Sabji Mandi,1
```

Station names are unique and are the key for the hash table lookup (case-insensitive). The loader can either read this file to build the station list first, or build the list while reading `buses.csv`; both give the same ids if the files are used as supplied.

### 6.4 File `route_stops.csv` (optional)

```
bus_no,seq,station_id,station_name
```

Gives every stop of every bus as its own row (`seq` starts at 1 and ends at `Geu`). Use either this file or the `stops` column of `buses.csv`, not both. It is handy for checking the data in a spreadsheet.

### 6.5 File `edges.csv` (optional travel-time overrides, in minutes)

```
from_station,to_station,minutes
ISBT,Geu,20
GMS Road,Geu,15
```

Every other consecutive stop pair uses the default of **4 minutes**. The same pair of stops always has the same time, even if many routes use it. More rows can be added without changing the code.

### 6.6 Data cleaning applied to the chart

- Spelling variants were merged into one station: `Balliwala` to `Baliwala Chowk`, `Dharampur` to `Dharampur Chowk`, `Selaqui` to `Selaqui Chowk`, `Prem Nagar` to `Premnagar`, `Raipur` to `Raipur Chowk`.
- Bus 20 lists `Garhi Cantt` twice on the chart; only the first is kept, because a route must not contain the same station twice.
- `Rajpur` (bus 35) and `Raipur Chowk` are different places and stay separate. `SGRR Mathurawala` (bus 8) and `Mathurawala` (bus 13) are also kept as separate stations.
- Bus 32's route text is garbled on the chart; it is stored as `Rispana;Kargi Chowk;ISBT;Geu`.
- Registration numbers have spaces removed and obvious typos fixed (for example `UK0 7PA` to `UK07PA`).

### 6.7 Important shared stations (the graph hubs)

Number of buses serving each hub, from `stations.csv`:

| Station | Buses |
|---------|-------|
| `Geu` | 52 |
| `ISBT` | 39 |
| `Rispana` | 23 |
| `Kargi Chowk` | 17 |
| `GMS Road` | 13 |
| `Jogiwala` | 12 |
| `Mokampur` | 8 |
| `Fountain Chowk` | 7 |
| `6 No. Pulia` | 7 |
| `Ballupur Chowk` | 7 |
| `Baliwala Chowk` | 7 |
| `Saharanpur Chowk` | 4 |
| `Mata Mandir` | 3 |
| `Bangali Kothi` | 3 |

Two main corridors exist: the **ISBT corridor** and the **GMS Road corridor**; this is what makes Dijkstra and BFS meaningful.

## 7. Architecture and File Layout

The program is split into a few small files so that each member owns one part. All code lives in `namespace cbt`.

| File          | Contents                                                                          | Approx. lines  |
| ------------- | --------------------------------------------------------------------------------- | -------------- |
| `DS.h`        | Template `LinkedList<T>`, `Stack<T>`, `Queue<T>`, `HashTable` (string key to int) | 230            |
| `Time.h`      | `Time` class with operator overloading                                            | 60             |
| `Models.h`    | `Station`, `Vehicle` (abstract), `Bus`, `MiniBus`, `Route`, `Schedule`            | 220            |
| `Graph.h`     | `Graph` (adjacency list) with BFS, DFS, Dijkstra                                  | 150            |
| `Algo.h`      | Sorting algorithms (bubble, insertion, merge) as templates                        | 70             |
| `Simulator.h` | `BusSimulator`: clock, positions, ETA, occupancy, recommendation                  | 150            |
| `UI.h`        | `Dashboard`, `MainMenu`, input helpers                                            | 190            |
| `Loader.h`    | File loading, report saving, custom exceptions                                    | 80             |
| `main.cpp`    | Wiring and program start                                                          | 30             |
| **Total**     |                                                                                   | **about 1180** |

Header-only classes are used to avoid linker problems. Build command:

```
g++ -std=c++11 -Wall -o busapp main.cpp
```

## 8. Class Design

### 8.1 `Time` (Time.h)

- Stores minutes since midnight in an `int`.
- Constructor `Time(int h=0,int m=0)`; `toString()` gives `HH:MM`.
- Overloaded operators: `+` and `-` (minutes), `<`, `>`, `==`, pre-increment `++t`, post-increment `t++`, `<<` and `>>` (stream I/O, friend functions).

### 8.2 `Station`

- Fields: `int id`, `string name`, `int busCount` (static counter of stations: `Station::total`).
- Methods: `getName()`, `matchesPrefix(prefix)`, copy constructor, destructor, `display()`.

### 8.3 `Vehicle` (abstract base) and its children

```
class Vehicle {            // abstract
  virtual int capacity() const = 0;
  virtual string typeName() const = 0;
  virtual void display() const;
  virtual ~Vehicle() {}
};
class Bus     : public Vehicle  // capacity 50, "Standard Bus"
class MiniBus : public Vehicle  // capacity 32, "Mini Bus"
```

The `type` column in `buses.txt` decides which child is created with `new` (dynamic objects). Buses are kept as `Vehicle*` so calls like `capacity()` and `display()` are resolved at run time (runtime polymorphism).

`Bus` / `Vehicle` fields: `busNo`, `registration`, `driver`, `departure` (`Time`), `Route* route`. `static int totalBuses` counts created vehicles.

### 8.4 `Route`

- Holds an ordered array of station ids (`int* stops`, `int n`) allocated dynamically.
- Has a **copy constructor** (deep copy) and **destructor**.
- Methods: `stopAt(i)`, `indexOf(stationId)`, `cumulativeTime(i, graph)`, `printRecursive(i)` (recursive printing of stops).
- Overloaded `<<` as a friend to print the route as `A -> B -> C`.

### 8.5 `Schedule`

- Owns the array of `Vehicle*` (array of objects/pointers), the `HashTable` of stations and the station list.
- Methods: `addBus(...)`, `findBus(busNo)`, `busesAtStation(stationId)` (returns a `LinkedList<Vehicle*>`).

### 8.6 `Graph`

- Vertices are station ids; edges come from consecutive stops of each route (directed, weight in minutes).
- Adjacency list built with the hand-made `LinkedList<Edge>`.
- Methods: `addEdge`, `bfsStops(src,dst)`, `dfsReach(src)`, `dijkstra(src,dst)`, `weight(a,b)`.

### 8.7 `BusSimulator`

- Owns the current `Time now`, a pointer to `Schedule` and `Graph`.
- Methods: `advance(min)`, `setTime(t)`, `status(bus)`, `currentStopIndex(bus)`, `etaAt(bus, station)`, `occupied(bus)`, `freeSeats(bus)`, `arrivalBoard(station)`, `recommend(station)`.

### 8.8 `Dashboard` and `MainMenu`

- `Dashboard::show()` prints the summary screen.
- `MainMenu::run()` loops, reads the choice, and dispatches to the right feature. Menu code never contains algorithm logic.

## 9. Data Structures (hand-built)

| Structure                          | Used for                                                                               | Key operations                                                                                       |
| ---------------------------------- | -------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------- |
| `LinkedList<T>` (singly, template) | Adjacency lists, station list, buses-at-station results, hash buckets                  | `pushBack`, `pushFront`, `find`, `size`, iterate by node pointer                                     |
| `Stack<T>` (linked, template)      | Iterative DFS, rebuilding Dijkstra/BFS paths in forward order, undo of last clock move | `push`, `pop`, `top`, `isEmpty`                                                                      |
| `Queue<T>` (linked, template)      | BFS frontier, arrival-order board (buses enqueued in ETA order)                        | `enqueue`, `dequeue`, `front`, `isEmpty`                                                             |
| `HashTable` (chaining)             | Station name to station id lookup in O(1) average                                      | `insert(name,id)`, `get(name)`; hash = polynomial rolling hash on the lowercase name, table size 101 |

Rule: Don’t use any STL. Every Data Structure is to be user def ined.

## 10. Algorithms

| Algorithm                       | Feature                                                                       | Details                                                                                       |
| ------------------------------- | ----------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------- |
| BFS                             | Minimum stops between two stations (FR11)                                     | Queue-based, treats every edge as 1, returns the stop count and path                          |
| DFS                             | Connectivity and reachable stations (FR12)                                    | Stack-based iterative version plus a short recursive version for explanation                  |
| Dijkstra                        | Shortest travel time between two stations (FR10); also used inside ETA to Geu | Array-based O(V squared); about 150 stations, so no heap is needed. Path rebuilt with a Stack |
| Bubble / Insertion / Merge sort | Sorting demo (FR13); Merge or Insertion is used for the arrival board         | Template functions taking a comparison function pointer                                       |
| Linear scan                     | Prefix station suggestions                                                    | Walk the station list, compare lowercase prefix                                               |

Graph note: because all routes drain toward Geu, many shortest paths pass through ISBT or GMS Road. BFS and Dijkstra may give different answers (fewest stops versus least time), which is a good point to show during evaluation.

## 11. Simulation Model

1. **Clock:** `now` starts at 07:00 by default. Controls: +1, +5, +10 minutes, or set a time.
2. **Bus start:** a bus is at its first stop at its departure time.
3. **Status:** before departure = `Not Started`; after reaching Geu = `Reached`; otherwise `Running`.
4. **Position:** `elapsed = now - departure`. The current stop index is the largest `i` where `cumulativeTime(i) <= elapsed`. Cumulative time sums the edge weights (default 4 min, overrides from `edges.txt`).
5. **ETA at station S:** `etaAt = departure + cumulativeTime(indexOf(S))`; ETA to the student is `etaAt - now`. If the bus has already passed S, it is shown as `Passed` and excluded from the board.
6. **Occupancy (deterministic, no random numbers needed):** `occupied = min(capacity, 5 + (busNo * 13) % 10 + 3 * stopsTravelled)` so a bus fills up as it moves along its route. `free = capacity - occupied`.
7. **Seat status:** free > 40% of capacity = `Plenty`; 10% to 40% = `Filling`; below 10% = `Full`.

## 12. Recommendation Rule

For a chosen station, consider every bus that has not yet passed it and is not `Full`:

```
score = waitMinutes + 0.5 * rideMinutesToGeu + seatPenalty
seatPenalty = 0 (Plenty), 5 (Filling)
```

- `waitMinutes` = ETA at the station minus now.
- `rideMinutesToGeu` = cumulative time from the station to the end of that bus's route.
- The **lowest score wins**. The screen also prints the second-best bus and the reason (for example "arrives in 4 min, 22 free seats"). If all buses are Full or none are coming, a clear message is shown.

## 13. CLI Design

### 13.1 Main menu

```
=========== COLLEGE BUS TRACKER (Team ByteMe) ===========
 Time: 07:35   Running: 18   Not started: 30   Reached: 4
---------------------------------------------------------
 1. Search station            6. Shortest time (Dijkstra)
 2. Search bus                7. Fewest stops (BFS)
 3. Search route              8. Connectivity (DFS)
 4. Arrival board             9. Sorting demo
 5. Recommend a bus          10. Dashboard
11. Advance clock            12. Save report
 0. Exit
Choice:
```

### 13.2 Arrival board (feature 4)

```
Station: Rispana                    Time: 07:35
#  Bus  Type      ETA     In      Seats(free/cap)  Status
1  6    Standard  07:41   6 min   31/50            Plenty
2  36   Standard  08:09   34 min  27/50            Plenty
3  48   Standard  08:00   25 min  9/50             Filling
Recommended: Bus 6 (arrives in 6 min, 31 free seats)
```

### 13.3 Input rules

- Menu input is validated; any non-number returns to the menu with a message.
- Station names are case-insensitive. If not found, up to 5 prefix suggestions are shown.

## 14. OOP and Syllabus Mapping

| Unit | Syllabus topic                                                                             | Where it appears in the project                                                      |
| ---- | ------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------ |
| 1    | Need for OOP, procedural vs OOP                                                            | Section 1 and viva answers: the same problem as one big procedure versus classes     |
| 1    | Header files, namespaces                                                                   | Multiple `.h` files, everything inside `namespace cbt`                               |
| 1    | Data types, variables, operators, expressions, arrays, strings                             | Time arithmetic, arrays of stops, `std::string` station names                        |
| 1    | Pointers, user-defined functions, argument passing                                         | `Vehicle*`, `Route* route`, pass by reference for graph and lists                    |
| 1    | Inline functions                                                                           | Small getters such as `getName()`, `capacity()` are `inline`                         |
| 1    | Recursive functions                                                                        | `Route::printRecursive`, recursive DFS, recursive merge sort                         |
| 2    | Classes, objects, access specifiers, member functions, data hiding                         | All classes keep data `private` with getters                                         |
| 2    | Constructors (default, parameterized, copy), destructors                                   | `Time`, `Station`, `Route` (deep copy)                                               |
| 2    | Static data members                                                                        | `Vehicle::totalBuses`, `Station::total`                                              |
| 2    | Friend functions                                                                           | `operator<<` for `Time` and `Route`                                                  |
| 2    | Passing/returning objects, arrays of objects, dynamic objects, pointers to objects, `this` | `Schedule` stores `Vehicle*` created with `new`; chained setters return `*this`      |
| 2    | Operator overloading (unary, binary, `+`, `-`, `++` pre/post, `<<`, `>>`)                  | `Time` class                                                                         |
| 3    | Inheritance, base and derived class, public inheritance                                    | `Vehicle` to `Bus` and `MiniBus`                                                     |
| 3    | Constructors and destructors in inheritance, passing parameters to base                    | `Bus(...) : Vehicle(...)` prints creation order in debug mode                        |
| 3    | Multilevel / multiple / hybrid (covered in viva)                                           | Optional extension: `Printable` interface plus `Vehicle` shows multiple inheritance  |
| 4    | Compile-time vs run-time polymorphism, overloading, overriding                             | Overloaded `display()`, `etaAt()` variants; overridden `capacity()`                  |
| 4    | Virtual functions, pure virtual, abstract class, virtual destructor, late binding          | `Vehicle` is abstract, accessed through base pointers                                |
| 5    | Stream classes, I/O manipulators, fstream, file operations                                 | `Loader` reads `buses.txt`; `setw`, `left`, `setfill` in tables; `report.txt` output |
| 5    | Function templates                                                                         | `LinkedList<T>`, `Stack<T>`, `Queue<T>`, `sortArray<T>`                              |
| 5    | Exception handling (throw, catch, re-throw)                                                | `DataException` for bad lines; `main` re-throws unknown errors after logging         |
| 5    | STL                                                                                        | One optional small demo only; core structures are hand-made                          |

## 15. Error Handling

| Situation                           | Behaviour                                                       |
| ----------------------------------- | --------------------------------------------------------------- |
| `buses.txt` missing                 | Throw `DataException`, print message, exit cleanly with code 1  |
| Bad line (missing fields, bad time) | Skip the line, print `Line N skipped: reason`, continue loading |
| Unknown station or bus              | Print "not found" plus suggestions; no crash                    |
| Non-numeric menu input              | Clear the input stream, show the menu again                     |
| Station not on any route            | "No bus serves this station"                                    |
| No path in graph                    | "Not connected"                                                 |

## 16. Acceptance Criteria and Test Cases

| ID  | Test                                               | Expected                                                          |
| --- | -------------------------------------------------- | ----------------------------------------------------------------- |
| T1  | Start the app with the full chart                  | Loads 52 buses with no skipped lines                              |
| T2  | Search `rispana` (lowercase)                       | Finds `Rispana` through the hash table                            |
| T3  | Search `Ris`                                       | Suggests `Rispana`                                                |
| T4  | Search a bad name `Xyz`                            | "Not found" message, menu returns                                 |
| T5  | Arrival board at `Kargi Chowk` at 07:50            | Several buses listed in ascending ETA order                       |
| T6  | Advance clock by 10 min                            | Positions and ETAs change; passed buses leave the board           |
| T7  | Bus 48 at 08:00                                    | Status Running, current stop `Rispana`, 3 stops left              |
| T8  | Recommend at `ISBT` at the busiest minute          | One bus recommended with reason; Full buses are never recommended |
| T9  | Dijkstra `Ranipokhri` to `Geu`                     | Prints total minutes and the path                                 |
| T10 | BFS `Doiwala` to `Geu`                             | Prints stop count and the path                                    |
| T11 | DFS from `Kulhal`                                  | Lists reachable stations                                          |
| T12 | Sort buses by ETA with bubble, insertion and merge | Identical output, different step counts shown                     |
| T13 | Save report                                        | `report.txt` is created and matches the board                     |
| T14 | Enter letters at the menu                          | No crash                                                          |

The project is done when T1 to T14 pass, the code compiles with `-Wall` without warnings, and it stays under 1200 lines.

## 17. Suggested Build Order (for generating the project)

1. `DS.h` templates and a tiny test for each structure.
2. `Time.h`, then `Models.h` (`Vehicle` hierarchy, `Station`, `Route`, `Schedule`).
3. `Loader.h` and a hand-made `buses.txt` with 10 buses; print what was loaded.
4. `Graph.h`: build from routes, then BFS, DFS, Dijkstra.
5. `Simulator.h`: clock, ETA, occupancy, board, recommendation.
6. `Algo.h` sorting, then `UI.h` menus and dashboard.
7. Transcribe the remaining buses, run T1 to T14, trim code to the line budget.

**Code rules:** one class per concern, short functions (under 30 lines), comments only where logic is not obvious, no global variables except constants, consistent naming (`camelCase` methods, `PascalCase` classes).

## 18. Evaluation Preparation: Likely Questions

| Question                                       | Short answer                                                                          |
| ---------------------------------------------- | ------------------------------------------------------------------------------------- |
| Why a Hash Table for stations?                 | Constant average lookup instead of scanning about 150 names each search               |
| Why Dijkstra and not BFS for time?             | Edges have different minutes; BFS only counts stops                                   |
| Why O(V squared) Dijkstra?                     | The graph is small; it avoids writing a heap and keeps code short                     |
| Where is polymorphism?                         | `Vehicle*` calls `capacity()` and `display()` resolved at run time                    |
| Where is operator overloading?                 | `Time`: `+`, `-`, `<`, `++`, `<<`, `>>`                                               |
| Why is the copy constructor needed in `Route`? | It owns a dynamic array; a shallow copy would double-free                             |
| Why a virtual destructor?                      | So deleting a `Vehicle*` also runs the `Bus` destructor                               |
| What is simulated?                             | Bus positions, ETA and seat occupancy; the route and schedule data are real           |
| Where are Stack and Queue used?                | Stack: DFS and path rebuild. Queue: BFS and arrival board                             |
| How would real GPS be added?                   | Replace `BusSimulator::currentStopIndex` with live coordinates; the rest is unchanged |

## 19. Assumptions, Risks and Future Scope

**Assumptions**

- The chart time is the departure time from the first stop.
- Travel time between consecutive stops is 4 minutes unless overridden.
- Every bus runs every day and never breaks down or is delayed.

**Risks**

| Risk                                            | Mitigation                                                                                       |
| ----------------------------------------------- | ------------------------------------------------------------------------------------------------ |
| Typos in transcribed stop names break the graph | Normalise names while loading; print a list of stations used by only one bus for manual checking |
| Code grows past 1200 lines                      | Keep the optional features (FR13, FR15) last and cut them first                                  |
| Team members cannot explain others' code        | Each member owns a file and presents it in the viva                                              |

**Future scope:** real GPS feed, mobile or web front end, delay prediction, admin editing of routes, and a heap-based Dijkstra for larger networks.
