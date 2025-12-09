#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <ctime>
using namespace std;

class BTree 
{
private:
    map<string, int> nameToId;  // Junction name → ID mapping
    
    Traffic history storage
    struct TrafficLog 
    {
        string timestamp;
        double trafficLevel;
        double timeInMinutes;
        
        TrafficLog(string ts, double level, double time) 
            : timestamp(ts), trafficLevel(level), timeInMinutes(time) {}
    };
    
    // Key: "2-3" (road between junctions 2 and 3)
    // Value: vector of traffic logs (chronologically sorted)
    map<string, vector<TrafficLog>> trafficHistory;
    
    // Helper function to get current timestamp
    string getCurrentTimestamp() 
    {
        time_t now = time(0);
        char buf[80];
        strftime(buf, sizeof(buf), "%Y-%m-%d %I:%M:%S%p", localtime(&now));
        return string(buf);
    }
    
    // Helper to create road key (always sorted: smaller-larger)
    string makeRoadKey(int from, int to) 
    {
        int a = min(from, to);
        int b = max(from, to);
        return to_string(a) + "-" + to_string(b);
    }
    
public:
    
    void insert(const string& name, int id) 
    {
        nameToId[name] = id;
        cout << "[B-Tree] Inserted: " << name << " -> ID: " << id << endl;
    }
    
    int search(const string& name) 
    {
        auto it = nameToId.find(name);
    
        if (it != nameToId.end()) 
        {
            cout << "[B-Tree] Found: " << name << " -> ID: " << it->second << endl;
            return it->second;
        }
        
        cout << "[B-Tree] Not found: " << name << endl;
        return -1;
    }
    
    vector<string> searchByPrefix(const string& prefix) 
    {
        vector<string> results;
    
        for (auto& [name, id] : nameToId) 
        {
            if (name.substr(0, prefix.length()) == prefix) 
            {
                results.push_back(name);
            }
        }
        return results;
    }
    
    // Log traffic update
    void logTrafficUpdate(int from, int to, double trafficLevel, double timeInMinutes) 
    {
        string roadKey = makeRoadKey(from, to);
        string timestamp = getCurrentTimestamp();
        
        trafficHistory[roadKey].push_back(TrafficLog(timestamp, trafficLevel, timeInMinutes));
        
        cout << "[B-Tree] Logged traffic: Road " << roadKey 
             << " → " << trafficLevel << "x (" << timeInMinutes << " min)" << endl;
    }
    
    // Show traffic history for a specific road
    void showRoadHistory(int from, int to) 
    {
        string roadKey = makeRoadKey(from, to);
        
        cout << "\n========== Traffic History: Road " << roadKey << " ==========" << endl;
        
        if (trafficHistory.find(roadKey) == trafficHistory.end() || 
            trafficHistory[roadKey].empty()) 
        {
            cout << "No traffic history available for this road." << endl;
            cout << "=================================================\n" << endl;
            return;
        }
        
        auto& logs = trafficHistory[roadKey];
        cout << "Total updates logged: " << logs.size() << endl;
        cout << "---------------------------------------------------" << endl;
        
        // Show last 10 entries (or all if less than 10)
        int start = max(0, (int)logs.size() - 10);
        
        for (int i = start; i < logs.size(); i++) 
        {
            cout << "  [" << (i+1) << "] " << logs[i].timestamp 
                 << " → Traffic: " << logs[i].trafficLevel << "x"
                 << ", Time: " << logs[i].timeInMinutes << " min" << endl;
        }
        
        // Calculate average traffic
        double avgTraffic = 0;
        for (auto& log : logs) 
        {
            avgTraffic += log.trafficLevel;
        }
        avgTraffic /= logs.size();
        
        cout << "---------------------------------------------------" << endl;
        cout << "Average traffic level: " << fixed << setprecision(2) 
             << avgTraffic << "x" << endl;
        cout << "=================================================\n" << endl;
    }
    
    // Show all roads with severe traffic (>3x)
    void showSevereTrafficRoads() 
    {
        cout << "\n========== Roads with Severe Traffic (>3.0x) ==========" << endl;
        
        bool foundAny = false;
        
        for (auto& [roadKey, logs] : trafficHistory) 
        {
            if (!logs.empty()) 
            {
                // Get most recent log
                TrafficLog& latest = logs.back();
                
                if (latest.trafficLevel >= 3.0) 
                {
                    cout << "  Road " << roadKey 
                         << " → " << latest.trafficLevel << "x"
                         << " (Current time: " << latest.timeInMinutes << " min)"
                         << " [Updated: " << latest.timestamp << "]" << endl;
                    foundAny = true;
                }
            }
        }
        
        if (!foundAny) 
        {
            cout << "  ✓ No severe traffic currently!" << endl;
        }
        
        cout << "====================================================\n" << endl;
    }
    
    // Show traffic analytics summary
    void showTrafficAnalytics() 
    {
        cout << "\n========== Traffic Analytics Summary ==========" << endl;
        cout << "Total roads monitored: " << trafficHistory.size() << endl;
        
        int totalLogs = 0;
        for (auto& [roadKey, logs] : trafficHistory) 
        {
            totalLogs += logs.size();
        }
        
        cout << "Total traffic updates logged: " << totalLogs << endl;
        cout << "Average updates per road: " 
             << (trafficHistory.empty() ? 0 : totalLogs / trafficHistory.size()) << endl;
        
        cout << "---------------------------------------------" << endl;
        cout << "Roads ranked by activity (most updated):" << endl;
        
        // Create vector of roads with their update counts
        vector<pair<string, int>> roadActivity;
        for (auto& [roadKey, logs] : trafficHistory) 
        {
            roadActivity.push_back({roadKey, logs.size()});
        }
        
        // Sort by activity (descending)
        sort(roadActivity.begin(), roadActivity.end(), 
             [](auto& a, auto& b) { return a.second > b.second; });
        
        // Show top 5
        for (int i = 0; i < min(5, (int)roadActivity.size()); i++) 
        {
            cout << "  " << (i+1) << ". Road " << roadActivity[i].first 
                 << " → " << roadActivity[i].second << " updates" << endl;
        }
        
        cout << "=============================================\n" << endl;
    }
    
    void display() 
    {
        cout << "\n========== B-TREE CONTENTS ==========" << endl;
        cout << "Total Junctions: " << nameToId.size() << endl;
        cout << "Total Roads Logged: " << trafficHistory.size() << endl;
        cout << "-------------------------------------" << endl;
    
        cout << "\nJunction Name → ID Mapping:" << endl;
        for (auto& [name, id] : nameToId) 
        {
            cout << "  " << name << " → ID: " << id << endl;
        }
        
        cout << "\nTraffic Monitoring Status:" << endl;
        if (trafficHistory.empty()) 
        {
            cout << "  No traffic data logged yet." << endl;
        } 
        else 
        {
            for (auto& [roadKey, logs] : trafficHistory) 
            {
                cout << "  Road " << roadKey << " → " << logs.size() 
                     << " updates logged" << endl;
            }
        }
        
        cout << "====================================\n" << endl;
    }
    
    int size() const 
    {
        return nameToId.size();
    }
    
    // Get total logged roads
    int getLoggedRoadsCount() const 
    {
        return trafficHistory.size();
    }
};

#endif

