#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <map>
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
using namespace std;

class Graph {
public:
    struct Edge {
        int to;
        double distance;
        double time;
        
        Edge(int t, double d, double tm) : to(t), distance(d), time(tm) {}
    };
    
private:
    map<int, vector<Edge>> adjList;
    int edgeCount;
    
public:
    Graph() : edgeCount(0) {}
    
    void addEdge(int from, int to, double distance, double time) {
        adjList[from].push_back(Edge(to, distance, time));
        adjList[to].push_back(Edge(from, distance, time));
        edgeCount++;
        
        cout << "[Graph] Added edge: " << from << " <-> " << to 
             << " (" << distance << "km, " << time << "min)" << endl;
    }
    
    void updateTraffic(int from, int to, double trafficMultiplier) {
        for (auto& edge : adjList[from]) {
            if (edge.to == to) {
                edge.time *= trafficMultiplier;
            }
        }
        
        for (auto& edge : adjList[to]) {
            if (edge.to == from) {
                edge.time *= trafficMultiplier;
            }
        }
        
        cout << "[Graph] Updated traffic: " << from << " <-> " << to 
             << " (x" << trafficMultiplier << ")" << endl;
    }
    
    pair<vector<int>, double> dijkstra(int source, int dest) {
        cout << "\n[Dijkstra] Finding shortest path: " 
             << source << " -> " << dest << endl;
        
        map<int, double> dist;
        map<int, int> parent;
        priority_queue<pair<double, int>, 
                       vector<pair<double, int>>, 
                       greater<pair<double, int>>> pq;
        
        for (auto& [node, edges] : adjList) {
            dist[node] = numeric_limits<double>::infinity();
        }
        
        dist[source] = 0;
        pq.push({0, source});
        
        while (!pq.empty()) {
            auto [currentDist, u] = pq.top();
            pq.pop();
            
            if (u == dest) {
                break;
            }
            
            if (currentDist > dist[u]) {
                continue;
            }
            
            for (auto& edge : adjList[u]) {
                int v = edge.to;
                double weight = edge.time;
                
                if (dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    parent[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }
        
        if (dist[dest] == numeric_limits<double>::infinity()) {
            cout << "[Dijkstra] No path found!" << endl;
            return {vector<int>(), -1};
        }
        
        vector<int> path;
        int current = dest;
        
        while (current != source) {
            path.push_back(current);
            current = parent[current];
        }
        path.push_back(source);
        reverse(path.begin(), path.end());
        
        cout << "[Dijkstra] Path found! Total time: " 
             << dist[dest] << " minutes" << endl;
        
        return {path, dist[dest]};
    }
    
    void display() {
        cout << "\n========= GRAPH STRUCTURE ==========" << endl;
        cout << "Total Junctions: " << adjList.size() << endl;
        cout << "Total Edges: " << edgeCount << endl;
        cout << "-----------------------------------" << endl;
        
        for (auto& [junction, edges] : adjList) {
            cout << "Junction " << junction << " connects to: ";
            for (auto& edge : edges) {
                cout << "[" << edge.to << ": " << edge.distance 
                     << "km, " << edge.time << "min] ";
            }
            cout << endl;
        }
        
        cout << "===================================\n" << endl;
    }
};

#endif