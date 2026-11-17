# College Bus Tracking System

A console-based bus tracking and route-planning system for a college campus, written in **C++ without the STL**. Every data structure (linked list, stack, queue, hash table, graph) and every algorithm (searching, sorting, shortest path) is implemented by hand.

Built by **Team ByteMe** as a university Project Based Learning (PBL) course project.

---

## Features

| # | Feature | Data structure / algorithm |
|---|---------|----------------------------|
| 1 | Search a station (with "Did you mean" suggestions) | Hash table, prefix search |
| 2 | Search a bus (details, status, current and next stop, seats) | Binary search |
| 3 | Search a route (full route of a bus, or direct buses between two stations) | Arrays, linked lists |
| 4 | Live arrival board for a station | Insertion sort + circular queue |
| 5 | Recommend the best bus for a station | Scoring function |
| 6 | Shortest travel time between two stations | Dijkstra |
| 7 | Fewest stops between two stations | BFS |
| 8 | List every station reachable from a station | DFS |
| 9 | Sort the bus fleet by number, departure or free seats | Merge sort |
| 10 | System dashboard (status counts, active buses, hub stations) | Bubble sort |
| 11 | Simulated clock with undo | Stack |
| 12 | Save the arrival board to `report.txt` | File I/O |

---

## Project structure

```
.
├── main.cpp            # console menu and all feature functions
├── Models.cpp          # constants, data classes, BusSystem (core logic)
├── Graph.cpp           # adjacency-list graph, Dijkstra, BFS, DFS
├── CsvLoader.cpp       # CSV parsing and data loading
├── DataStructures.cpp  # IntList, IntStack, IntQueue, HashTable
├── Algo.cpp            # merge sort
└── data/
    ├── stations.csv
    ├── buses.csv
    └── edges.csv
```

Include chain: `main.cpp` -> `CsvLoader.cpp` -> `Graph.cpp` -> `Models.cpp` -> `DataStructures.cpp`.
`Algo.cpp` is included directly by `main.cpp`.

---

## Build and run

Requires a C++11 (or newer) compiler. No build tools are needed.

```bash
g++ *.cpp -o main
./main
```

On Windows:

```bash
g++ *.cpp -o main.exe
main.exe
```

By default the program looks for the CSV files in `data/`. If that folder does not exist it falls back to the current directory. You can also pass a folder explicitly:

```bash
./main path/to/csv/folder
```

If `stations.csv` or `buses.csv` cannot be loaded, the program exits with an error.

---

## Data files

### `stations.csv`
```
station_id,station_name
1,Prince Chowk
2,Saharanpur Chowk
...
```
Station names are matched **case-insensitively**. The `station_id` is kept for display only.

### `buses.csv`
```
bus_no,registration,departure,driver,type,stops
2,UK07S2788,07:00,Shankar Thapa,S,Prince Chowk;Saharanpur Chowk;Matawala Bagh;ISBT;Geu
```
- `departure` is `HH:MM` (24-hour).
- `type` is `S` (standard, 50 seats) or `M` (mini, 32 seats).
- `stops` is a `;`-separated list of station names in travel order. Every name must exist in `stations.csv`, otherwise the bus is skipped.

### `edges.csv` (optional overrides)
```
from_station,to_station,minutes
ISBT,Geu,20
GMS Road,Geu,15
```
By default every hop between consecutive stops takes **4 minutes**. This file overrides the time for specific station pairs (up to 50 overrides).

Invalid lines in any file are skipped rather than crashing the loader.

---

## How it works

### Loading (`loadAllData`)

```
loadStations()          -> fills stations[] and the hash table
loadBuses()             -> fills buses[] (stop names -> indexes via the hash table)
loadEdges()             -> fills overrides[] (custom travel times)
sortBusesByNumber()     -> buses[] sorted, enabling binary search
buildStationBusLists()  -> for every station, the list of buses that stop there
buildGraphFromRoutes()  -> builds the adjacency list from bus routes
```

The order matters: overrides must exist before the graph is built so the edge weights are correct.

### Indexes, not strings

Station and bus names are converted to **integer indexes** once, when read. Everything internal (graph, queues, stacks, routes) works on indexes. Indexes are turned back into names only when printing.

- **Station index** = position in `stations[]` (0-based, in file order). This is different from the CSV `station_id`.
- **Bus index** = position in the sorted `buses[]` array. This is different from the bus number.

```
name --(HashTable)--> index --(algorithms)--> index path --(stations[])--> name
```

### The graph

An **adjacency list** of linked lists, one list per station:

```cpp
class GraphEdge {            // one node of an adjacency list
    int toStation;           // destination station index
    int weight;              // travel time in minutes
    GraphEdge* next;         // next edge leaving the same station
};

class StationGraph {         // the whole graph
    GraphEdge* adj[MAX_STATIONS];
};
```

- It is **directed**: a bus going A -> B does not create B -> A.
- Edges come from consecutive stops of every bus. Duplicate connections are merged, keeping the smaller weight.
- It is built once at startup and only read afterwards.

With the sample data: 93 stations, 52 buses, 107 edges. Geu is the final stop of every bus, so it has no outgoing edges.

Example of one slot:

```
adj[10] Garhi Cantt -> ONGC Chowk(14) 4 min -> Vijay Coloney(11) 4 min -> NULL
```

### Route finding

| Feature | Algorithm | Minimises | Complexity |
|---------|-----------|-----------|------------|
| Shortest time | Dijkstra (array version) | total minutes | O(V^2 + E) |
| Fewest stops | BFS | number of hops | O(V + E) |
| Reachability | DFS (iterative, stack) | n/a | O(V + E) |

After a route is found, the program checks whether a single bus follows that exact path and lists it.

Example from the sample data (Ring Road to ISBT, option 6):

```
Shortest Travel Time : 8 minutes
Path: Ring Road -> Jogiwala -> ISBT
```

Route times are ride time only. Waiting time and bus changes are not modelled.

### Arrival board and recommendation

- The **arrival board** collects the buses that have not yet passed a station, sorts them by ETA with insertion sort, and puts them in a circular queue that is dequeued in arrival order.
- The **recommendation** scores each eligible bus and picks the lowest:

```
score = wait + 0.5 * remaining_ride + (5 if seats are "Filling")
```

  Buses that have already passed, or are full, are excluded. The recommended bus is not always the first one on the board, because ride length and seat availability are also weighed.

### Seats and status

- Status is **Not Started**, **Running** or **Reached**, derived from the simulated clock and the bus's cumulative route time.
- Seat occupancy is simulated from the bus number and the number of stops covered. Seat status is **Plenty** (more than 40% free), **Filling** (10% to 40%) or **Full**.

### Simulated clock

The system clock starts at 07:00 and can move forward by 1, 5 or 10 minutes, be set to a given time, or be undone. Every change pushes the previous time on a stack, which is what makes undo possible.

---

## Data structures used

| Structure | Where | Stores |
|-----------|-------|--------|
| Array | `stations[]`, `buses[]`, `overrides[]`, `stops[]` | core data and routes |
| Hash table (211 slots, linear probing) | `stationIndex` | station name -> station index |
| Singly linked list (`IntList`) | `busesAtStation[]` | bus indexes serving each station |
| Adjacency list | `StationGraph` | edges between stations |
| Stack (`IntStack`) | clock history, path reversal, DFS | ints |
| Circular queue (`IntQueue`) | arrival board, BFS | ints |

### Algorithms used

| Algorithm | Where |
|-----------|-------|
| Binary search | `findBus` |
| Prefix search | `suggestStations` |
| Insertion sort | arrival board, `sortBusesByNumber` |
| Merge sort | fleet sorting (option 9) |
| Bubble sort | hub ranking in the dashboard |
| Dijkstra, BFS, DFS | options 6, 7, 8 |

---

## Team

**Team ByteMe**
