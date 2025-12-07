#include <iostream>
#include <fstream>
#include <iomanip>
#include "src/btree.h"
#include "src/hashtable.h"
#include "src/graph.h"
#include "include/json.hpp"

using namespace std;
using json = nlohmann::json;

// Load junctions from JSON file
void loadJunctions(BTree& btree, HashTable& hashtable) {
    ifstream file("data/junctions.json");
    
    if (!file.is_open()) {
        cout << "[ERROR] Could not open junctions.json!" << endl;
        return;
    }
    
    json data;
    file >> data;
    
    for (auto& j : data["junctions"]) {
        int id = j["id"];
        string name = j["name"];
        double lat = j["lat"];
        double lng = j["lng"];
        
        Junction junction(id, name, lat, lng);
        btree.insert(name, id);
        hashtable.insert(junction);
    }
    
    file.close();
    cout << "[OK] Loaded " << data["junctions"].size() << " junctions from JSON" << endl;
}

// Load roads from JSON file
void loadRoads(Graph& graph) {
    ifstream file("data/roads.json");
    
    if (!file.is_open()) {
        cout << "[ERROR] Could not open roads.json!" << endl;
        return;
    }
    
    json data;
    file >> data;
    
    for (auto& r : data["roads"]) {
        int from = r["from"];
        int to = r["to"];
        double distance = r["distance"];
        double time = r["base_time"];
        
        graph.addEdge(from, to, distance, time);
    }
    
    file.close();
    cout << "[OK] Loaded " << data["roads"].size() << " roads from JSON" << endl;
}

// Display menu
void displayMenu() {
    cout << "\n========================================" << endl;
    cout << "   SMART TRAFFIC ROUTE OPTIMIZER       " << endl;
    cout << "========================================" << endl;
    cout << "  1. Search Junction by Name           " << endl;
    cout << "  2. Get Junction Details by ID        " << endl;
    cout << "  3. Find Shortest Path                " << endl;
    cout << "  4. Display All Data Structures       " << endl;
    cout << "  5. Update Traffic Condition          " << endl;
    cout << "  6. Exit                              " << endl;
    cout << "========================================" << endl;
    cout << "Enter choice: ";
}

int main() {
    // Initialize data structures
    BTree btree;
    HashTable hashtable;
    Graph graph;
    
    cout << "\n========================================" << endl;
    cout << "  INITIALIZING TRAFFIC OPTIMIZER...    " << endl;
    cout << "========================================\n" << endl;
    
    // Load data from JSON files
    cout << "Loading junctions from JSON..." << endl;
    loadJunctions(btree, hashtable);
    
    cout << "\nLoading roads from JSON..." << endl;
    loadRoads(graph);
    
    cout << "\n[OK] System Ready!" << endl;
    
    // Main loop
    int choice;
    do {
        displayMenu();
        cin >> choice;
        cin.ignore(); // Clear newline
        
        switch (choice) {
            case 1: {
                // Search by name
                string name;
                cout << "\nEnter junction name: ";
                getline(cin, name);
                
                int id = btree.search(name);
                if (id != -1) {
                    Junction* j = hashtable.search(id);
                    if (j) {
                        cout << "\n>>> Junction Details:" << endl;
                        cout << "  ID: " << j->id << endl;
                        cout << "  Name: " << j->name << endl;
                        cout << "  Coordinates: " << j->lat << " N, " 
                             << j->lng << " E" << endl;
                    }
                }
                break;
            }
            
            case 2: {
                // Get details by ID
                int id;
                cout << "\nEnter junction ID: ";
                cin >> id;
                
                Junction* j = hashtable.search(id);
                if (j) {
                    cout << "\n>>> Junction Details:" << endl;
                    cout << "  ID: " << j->id << endl;
                    cout << "  Name: " << j->name << endl;
                    cout << "  Coordinates: " << j->lat << " N, " 
                         << j->lng << " E" << endl;
                }
                break;
            }
            
            case 3: {
                // Find shortest path
                int source, dest;
                cout << "\nEnter source junction ID: ";
                cin >> source;
                cout << "Enter destination junction ID: ";
                cin >> dest;
                
                auto [path, time] = graph.dijkstra(source, dest);
                
                if (!path.empty()) {
                    cout << "\n>>> Shortest Route Found!" << endl;
                    cout << "========================================" << endl;
                    cout << "Path: ";
                    
                    for (size_t i = 0; i < path.size(); i++) {
                        Junction* j = hashtable.search(path[i]);
                        if (j) {
                            cout << j->name;
                        } else {
                            cout << path[i];
                        }
                        
                        if (i < path.size() - 1) {
                            cout << " -> ";
                        }
                    }
                    
                    cout << "\n\nTotal Time: " << fixed << setprecision(1) 
                         << time << " minutes" << endl;
                    cout << "Estimated Distance: " << (time * 0.5) 
                         << " km" << endl;
                    cout << "========================================" << endl;
                } else {
                    cout << "\n[ERROR] No path found between these junctions!" << endl;
                }
                break;
            }
            
            case 4: {
                // Display all structures
                btree.display();
                hashtable.display();
                graph.display();
                break;
            }
            
            case 5: {
                // Update traffic
                int from, to;
                double multiplier;
                
                cout << "\nEnter road (from junction ID): ";
                cin >> from;
                cout << "Enter road (to junction ID): ";
                cin >> to;
                cout << "Enter traffic multiplier (1.0=clear, 2.0=moderate, 3.0=heavy): ";
                cin >> multiplier;
                
                graph.updateTraffic(from, to, multiplier);
                cout << "\n[OK] Traffic updated successfully!" << endl;
                break;
            }
            
            case 6:
                cout << "\n========================================" << endl;
                cout << "  Thank you for using Traffic Optimizer!" << endl;
                cout << "========================================" << endl;
                break;
                
            default:
                cout << "\n[ERROR] Invalid choice! Please try again." << endl;
        }
        
    } while (choice != 6);
    
    return 0;
}