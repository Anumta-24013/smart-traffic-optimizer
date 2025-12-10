#include "include/httplib.h"
#include "include/json.hpp"
#include "src/btree.h"
#include "src/graph.h"
#include "src/hashtable.h"
#include <fstream>
#include <iostream>

using json = nlohmann::json;
using namespace httplib;

// Global data structures
BTree btree;
Graph graph;
HashTable hashtable;

// ⭐ CORS Headers Function
void enableCORS(Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
}

// Load data from JSON files
void loadData() {
    // Load junctions
    std::ifstream jFile("data/junctions.json");
    if (jFile.is_open()) {
        json jData;
        jFile >> jData;
        
        for (auto& j : jData["junctions"]) {
            int id = j["id"];
            std::string name = j["name"];
            double lat = j["lat"];
            double lng = j["lng"];
            
            Junction junction(id, name, lat, lng);
            btree.insert(name, id);
            hashtable.insert(junction);
        }
        jFile.close();
        std::cout << "[OK] Loaded " << jData["junctions"].size() << " junctions" << std::endl;
    }
    
    // Load roads
    std::ifstream rFile("data/roads.json");
    if (rFile.is_open()) {
        json rData;
        rFile >> rData;
        
        for (auto& r : rData["roads"]) {
            int from = r["from"];
            int to = r["to"];
            double distance = r["distance"];
            double time = r["base_time"];
            
            graph.addEdge(from, to, distance, time);
        }
        rFile.close();
        std::cout << "[OK] Loaded " << rData["roads"].size() << " roads" << std::endl;
    }
    
    std::cout << "[OK] Data loaded successfully!" << std::endl;
}

int main() {
    Server svr;
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "  SMART TRAFFIC API SERVER STARTING..." << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::cout << "Loading data..." << std::endl;
    loadData();
    
    // ⭐ Handle CORS preflight requests
    svr.Options(".*", [](const Request& req, Response& res) {
        enableCORS(res);
        res.status = 200;
    });
    
    // ⭐ Health check endpoint
    svr.Get("/api/health", [&](const Request& req, Response& res) {
        enableCORS(res);  // ⭐ IMPORTANT
        
        json response = {
            {"status", "OK"},
            {"message", "Server is running"},
            {"junctions", btree.size()},
            {"timestamp", std::time(0)}
        };
        
        res.set_content(response.dump(), "application/json");
        std::cout << "[API] GET /api/health - Server healthy" << std::endl;
    });
    
    // ⭐ Get all junctions
    svr.Get("/api/junctions", [&](const Request& req, Response& res) {
        enableCORS(res);  // ⭐ IMPORTANT
        
        json response;
        response["junctions"] = json::array();
        
        std::ifstream jFile("data/junctions.json");
        if (jFile.is_open()) {
            json jData;
            jFile >> jData;
            response["junctions"] = jData["junctions"];
            jFile.close();
        }
        
        res.set_content(response.dump(), "application/json");
        std::cout << "[API] GET /api/junctions - Returned " 
                  << response["junctions"].size() << " junctions" << std::endl;
    });
    
    // ⭐ Find shortest path
    svr.Post("/api/path", [&](const Request& req, Response& res) {
        enableCORS(res);  // ⭐ IMPORTANT
        
        try {
            auto body = json::parse(req.body);
            int source = body["source"];
            int destination = body["destination"];
            
            std::cout << "[API] POST /api/path - Finding path: " 
                      << source << " -> " << destination << std::endl;
            
            auto [path, totalTime] = graph.dijkstra(source, destination);
            
            if (path.empty()) {
                json errorResponse = {
                    {"success", false},
                    {"message", "No path found"}
                };
                res.set_content(errorResponse.dump(), "application/json");
                return;
            }
            
            json response;
            response["success"] = true;
            response["path"] = json::array();
            response["totalTime"] = totalTime;
            response["estimatedDistance"] = totalTime * 0.5;
            
            for (int id : path) {
                Junction* j = hashtable.search(id);
                if (j) {
                    response["path"].push_back({
                        {"id", j->id},
                        {"name", j->name},
                        {"lat", j->lat},
                        {"lng", j->lng}
                    });
                }
            }
            
            res.set_content(response.dump(), "application/json");
            std::cout << "[Dijkstra] Path found! Total time: " 
                      << totalTime << " minutes" << std::endl;
            
        } catch (const std::exception& e) {
            json errorResponse = {
                {"success", false},
                {"message", std::string("Error: ") + e.what()}
            };
            res.set_content(errorResponse.dump(), "application/json");
        }
    });
    
    // ⭐ Update traffic
    svr.Post("/api/traffic", [&](const Request& req, Response& res) {
        enableCORS(res);  // ⭐ IMPORTANT
        
        try {
            auto body = json::parse(req.body);
            int from = body["from"];
            int to = body["to"];
            double multiplier = body["multiplier"];
            
            std::cout << "[API] POST /api/traffic - Updating: " 
                      << from << " <-> " << to << " (x" << multiplier << ")" << std::endl;
            
            graph.updateTraffic(from, to, multiplier);
            
            json response = {
                {"success", true},
                {"message", "Traffic updated successfully"}
            };
            
            res.set_content(response.dump(), "application/json");
            
        } catch (const std::exception& e) {
            json errorResponse = {
                {"success", false},
                {"message", std::string("Error: ") + e.what()}
            };
            res.set_content(errorResponse.dump(), "application/json");
        }
    });
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "  SERVER RUNNING ON http://0.0.0.0:8080" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Available Endpoints:" << std::endl;
    std::cout << "  GET  /api/junctions        - Get all junctions" << std::endl;
    std::cout << "  POST /api/path             - Find shortest path" << std::endl;
    std::cout << "  POST /api/traffic          - Update traffic" << std::endl;
    std::cout << "  GET  /api/health           - Health check" << std::endl;
    std::cout << "Press Ctrl+C to stop server..." << std::endl;
    
    svr.listen("0.0.0.0", 8080);
    
    return 0;
}