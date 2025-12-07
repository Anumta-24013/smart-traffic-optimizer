#include <iostream>
#include <fstream>
#include <iomanip>
#include "src/btree.h"
#include "src/hashtable.h"
#include "src/graph.h"

using namespace std;

// Load junctions from JSON (manual parsing for simplicity)
void loadJunctions(BTree& btree, HashTable& hashtable) {
    // Hardcoded for now (you can parse JSON later)
    vector<Junction> junctions = {
        {1, "Liberty Chowk", 31.5096, 74.3442},
        {2, "Kalma Chowk", 31.5204, 74.3587},
        {3, "Mall Road", 31.5656, 74.3242},
        {4, "Jail Road", 31.5497, 74.3436},
        {5, "Township", 31.4697, 74.3973},
        {6, "Gulberg Main", 31.5203, 74.3587},
        {7, "Ferozepur Road", 31.4343, 74.2963},
        {8, "Model Town", 31.4843, 74.3154}
    };
    
    for (auto& j : junctions) {
        btree.insert(j.name, j.id);
        hashtable.insert(j);
    }
}

// Load roads from data
void loadRoads(Graph& graph) {
    // Hardcoded road connections
    graph.addEdge(1, 2, 3.5, 8);   // Liberty <-> Kalma
    graph.addEdge(2, 3, 5.2, 12);  // Kalma <-> Mall
    graph.addEdge(1, 4, 2.1, 5);   // Liberty <-> Jail
    graph.addEdge(4, 5, 8.3, 18);  // Jail <-> Township
    graph.addEdge(2, 5, 6.7, 15);  // Kalma <-> Township
    graph.addEdge(2, 6, 1.2, 3);   // Kalma <-> Gulberg
    graph.addEdge(6, 8, 4.5, 10);  // Gulberg <-> Model Town
    graph.addEdge(7, 5, 7.8, 16);  // Ferozepur <-> Township
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
    
    // Load data
    cout << "Loading junctions..." << endl;
    loadJunctions(btree, hashtable);
    
    cout << "\nLoading roads..." << endl;
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