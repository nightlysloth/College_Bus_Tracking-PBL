#pragma once
#include "Models.cpp"

struct GraphEdge {
    int toStation;
    int weight;
    GraphEdge* next;
};

struct StationGraph {
    GraphEdge* adj[MAX_STATIONS];
    int vertexCount;

    StationGraph() {
        vertexCount = 0;
        for (int i = 0; i < MAX_STATIONS; i++) adj[i] = NULL;
    }

    ~StationGraph() {
        for (int i = 0; i < MAX_STATIONS; i++) {
            while (adj[i]) {
                GraphEdge* next = adj[i]->next;
                delete adj[i];
                adj[i] = next;
            }
        }
    }
};

inline StationGraph& getGraph() {
    static StationGraph instance;
    return instance;
}
#define graph (getGraph())

inline void initGraph() {
    graph.vertexCount = sys.stationCount;
    for (int i = 0; i < MAX_STATIONS; i++) {
        while (graph.adj[i]) {
            GraphEdge* next = graph.adj[i]->next;
            delete graph.adj[i];
            graph.adj[i] = next;
        }
    }
}

inline void addGraphEdge(int fromStation, int toStation, int weight) {
    if (fromStation < 0 || fromStation >= MAX_STATIONS || toStation < 0 || toStation >= MAX_STATIONS) return;
    for (GraphEdge* e = graph.adj[fromStation]; e != NULL; e = e->next) {
        if (e->toStation == toStation) {
            if (weight < e->weight) e->weight = weight;
            return;
        }
    }
    graph.adj[fromStation] = new GraphEdge{toStation, weight, graph.adj[fromStation]};
}

inline void buildGraphFromRoutes() {
    initGraph();
    for (int b = 0; b < sys.busCount; b++) {
        for (int i = 0; i < sys.buses[b].stopCount - 1; i++) {
            int u = sys.buses[b].stops[i];
            int v = sys.buses[b].stops[i + 1];
            addGraphEdge(u, v, sys.edgeMinutes(u, v));
        }
    }
}

inline bool dijkstraShortestPath(int src, int dst, int& totalMinutes, int path[], int& pathLength) {
    int dist[MAX_STATIONS], parent[MAX_STATIONS];
    bool visited[MAX_STATIONS];
    const int INF = 999999;

    for (int i = 0; i < sys.stationCount; i++) {
        dist[i] = INF; visited[i] = false; parent[i] = -1;
    }
    dist[src] = 0;

    for (int step = 0; step < sys.stationCount; step++) {
        int u = -1, minDist = INF;
        for (int i = 0; i < sys.stationCount; i++) {
            if (!visited[i] && dist[i] < minDist) { minDist = dist[i]; u = i; }
        }
        if (u == -1 || minDist == INF || u == dst) break;
        visited[u] = true;

        for (GraphEdge* e = graph.adj[u]; e != NULL; e = e->next) {
            int v = e->toStation, w = e->weight;
            if (!visited[v] && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                parent[v] = u;
            }
        }
    }

    if (dist[dst] >= INF) return false;
    totalMinutes = dist[dst];

    IntStack s;
    for (int curr = dst; curr != -1; curr = parent[curr]) s.push(curr);
    pathLength = 0;
    while (!s.isEmpty()) path[pathLength++] = s.pop();
    return true;
}

inline bool bfsFewestStops(int src, int dst, int& stopCount, int path[], int& pathLength) {
    bool visited[MAX_STATIONS] = {false};
    int parent[MAX_STATIONS];
    for (int i = 0; i < sys.stationCount; i++) parent[i] = -1;

    IntQueue q;
    visited[src] = true;
    q.enqueue(src);
    bool reached = false;

    while (!q.isEmpty()) {
        int u = q.dequeue();
        if (u == dst) { reached = true; break; }

        for (GraphEdge* e = graph.adj[u]; e != NULL; e = e->next) {
            int v = e->toStation;
            if (!visited[v]) {
                visited[v] = true;
                parent[v] = u;
                q.enqueue(v);
            }
        }
    }

    if (!reached && src != dst) return false;

    IntStack s;
    for (int curr = dst; curr != -1; curr = parent[curr]) s.push(curr);
    pathLength = 0;
    while (!s.isEmpty()) path[pathLength++] = s.pop();
    stopCount = pathLength - 1;
    return true;
}

inline int dfsReachableStations(int src, int reachableList[], int maxStations) {
    bool visited[MAX_STATIONS] = {false};
    IntStack s;
    s.push(src);
    visited[src] = true;
    int count = 0;

    while (!s.isEmpty()) {
        int u = s.pop();
        if (count < maxStations) reachableList[count++] = u;

        for (GraphEdge* e = graph.adj[u]; e != NULL; e = e->next) {
            int v = e->toStation;
            if (!visited[v]) {
                visited[v] = true;
                s.push(v);
            }
        }
    }
    return count;
}
