#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;

class BTree 
{

private:
    map<string, int> data;
    
public:
    
    void insert(const string& name, int id) 
    {
        data[name] = id;
        cout << "[B-Tree] Inserted: " << name << " -> ID: " << id << endl;
    }
    
    int search(const string& name) 
    {
        auto it = data.find(name);
    
        if (it != data.end()) 
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
    
        for (auto& [name, id] : data) 
        {
            if (name.substr(0, prefix.length()) == prefix) 
            {
                results.push_back(name);
            }
        }
        return results;
    }
    
    void display() 
    {
        cout << "\n========== B-TREE CONTENTS ==========" << endl;
        cout << "Total Junctions: " << data.size() << endl;
        cout << "-------------------------------------" << endl;
    
        for (auto& [name, id] : data) 
        {
            cout << "  " << name << " -> ID: " << id << endl;
        }
        cout << "====================================\n" << endl;
    }
    
    int size() const 
    {
        return data.size();
    }
};

#endif