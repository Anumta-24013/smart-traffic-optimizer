#include <iostream>
#include <fstream>
#include "include/httplib.h"
#include "include/json.hpp"
#include "src/btree.h"
#include "src/hashtable.h"
#include "src/graph.h"

using namespace std;
using json = nlohmann::json;

// Global data structures
BTree btree;
HashTable hashtable;
Graph graph;

// Load junctions from JSON
void loadJunctions() {
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
    cout << "[OK] Loaded " << data["junctions"].size() << " junctions" << endl;
}

// Load roads from JSON
void loadRoads() {
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
    cout << "[OK] Loaded " << data["roads"].size() << " roads" << endl;
}

int main() {
    cout << "\n========================================" << endl;
    cout << "  SMART TRAFFIC API SERVER STARTING... " << endl;
    cout << "========================================\n" << endl;
    
    // Load data
    cout << "Loading data..." << endl;
    loadJunctions();
    loadRoads();
    cout << "\n[OK] Data loaded successfully!\n" << endl;
    
    // Create HTTP server
    httplib::Server svr;
    
    // Enable CORS for frontend
    svr.set_base_dir("./frontend");
    
    // CORS middleware
    svr.set_pre_routing_handler([](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        return httplib::Server::HandlerResponse::Unhandled;
    });
    
    // ========================================
    // ENDPOINT 1: Get all junctions
    // ========================================
    svr.Get("/api/junctions", [](const httplib::Request &req, httplib::Response &res) {
        ifstream file("data/junctions.json");
        json data;
        file >> data;
        file.close();
        
        res.set_content(data.dump(), "application/json");
        cout << "[API] GET /api/junctions - Returned " << data["junctions"].size() << " junctions" << endl;
    });
    
    // ========================================
    // ENDPOINT 2: Search junction by name
    // ========================================
    svr.Get(R"(/api/search/(.+))", [](const httplib::Request &req, httplib::Response &res) {
        string name = req.matches[1];
        
        // URL decode (replace %20 with space)
        size_t pos = 0;
        while ((pos = name.find("%20")) != string::npos) {
            name.replace(pos, 3, " ");
        }
        
        cout << "[API] GET /api/search/" << name << endl;
        
        int id = btree.search(name);
        
        if (id != -1) {
            Junction* j = hashtable.search(id);
            if (j) {
                json response = {
                    {"success", true},
                    {"junction", {
                        {"id", j->id},
                        {"name", j->name},
                        {"lat", j->lat},
                        {"lng", j->lng}
                    }}
                };
                res.set_content(response.dump(), "application/json");
                return;
            }
        }
        
        json response = {
            {"success", false},
            {"message", "Junction not found"}
        };
        res.set_content(response.dump(), "application/json");
    });
    
    // ========================================
    // ENDPOINT 3: Get junction by ID
    // ========================================
    svr.Get(R"(/api/junction/(\d+))", [](const httplib::Request &req, httplib::Response &res) {
        int id = stoi(req.matches[1]);
        cout << "[API] GET /api/junction/" << id << endl;
        
        Junction* j = hashtable.search(id);
        
        if (j) {
            json response = {
                {"success", true},
                {"junction", {
                    {"id", j->id},
                    {"name", j->name},
                    {"lat", j->lat},
                    {"lng", j->lng}
                }}
            };
            res.set_content(response.dump(), "application/json");
        } else {
            json response = {
                {"success", false},
                {"message", "Junction not found"}
            };
            res.set_content(response.dump(), "application/json");
        }
    });
    
    // ========================================
    // ENDPOINT 4: Find shortest path
    // ========================================
    svr.Post("/api/path", [](const httplib::Request &req, httplib::Response &res) {
        json requestData = json::parse(req.body);
        
        int source = requestData["source"];
        int destination = requestData["destination"];
        
        cout << "[API] POST /api/path - Finding path: " << source << " -> " << destination << endl;
        
        auto [path, time] = graph.dijkstra(source, destination);
        
        if (!path.empty()) {
            json pathDetails = json::array();
            
            for (int junctionId : path) {
                Junction* j = hashtable.search(junctionId);
                if (j) {
                    pathDetails.push_back({
                        {"id", j->id},
                        {"name", j->name},
                        {"lat", j->lat},
                        {"lng", j->lng}
                    });
                }
            }
            
            json response = {
                {"success", true},
                {"path", pathDetails},
                {"totalTime", time},
                {"estimatedDistance", time * 0.5}
            };
            
            res.set_content(response.dump(), "application/json");
        } else {
            json response = {
                {"success", false},
                {"message", "No path found"}
            };
            res.set_content(response.dump(), "application/json");
        }
    });
    
    // ========================================
    // ENDPOINT 5: Update traffic
    // ========================================
    svr.Post("/api/traffic", [](const httplib::Request &req, httplib::Response &res) {
        json requestData = json::parse(req.body);
        
        int from = requestData["from"];
        int to = requestData["to"];
        double multiplier = requestData["multiplier"];
        
        cout << "[API] POST /api/traffic - Updating: " << from << " <-> " << to 
             << " (x" << multiplier << ")" << endl;
        
        graph.updateTraffic(from, to, multiplier);
        
        json response = {
            {"success", true},
            {"message", "Traffic updated successfully"}
        };
        
        res.set_content(response.dump(), "application/json");
    });
    
    // ========================================
    // ENDPOINT 6: Health check
    // ========================================
    svr.Get("/api/health", [](const httplib::Request &req, httplib::Response &res) {
        json response = {
            {"status", "OK"},
            {"message", "Server is running"},
            {"junctions", btree.size()},
            {"timestamp", time(nullptr)}
        };
        res.set_content(response.dump(), "application/json");
    });
    
    // ========================================
    // Start server
    // ========================================
    cout << "========================================" << endl;
    cout << "  SERVER RUNNING ON http://localhost:8080" << endl;
    cout << "========================================" << endl;
    cout << "\nAvailable Endpoints:" << endl;
    cout << "  GET  /api/junctions        - Get all junctions" << endl;
    cout << "  GET  /api/search/<name>    - Search by name" << endl;
    cout << "  GET  /api/junction/<id>    - Get by ID" << endl;
    cout << "  POST /api/path             - Find shortest path" << endl;
    cout << "  POST /api/traffic          - Update traffic" << endl;
    cout << "  GET  /api/health           - Health check" << endl;
    cout << "\nPress Ctrl+C to stop server...\n" << endl;
    
    svr.listen("0.0.0.0", 8080);
    
    return 0;
}