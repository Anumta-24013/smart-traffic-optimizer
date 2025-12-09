#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <vector>
#include <list>
#include <string>
using namespace std;

struct Junction 
{
    int id;
    string name;
    double lat;
    double lng;
    
    Junction() : id(0), name(""), lat(0.0), lng(0.0) {}

    Junction(int i, string n, double la, double lo) 
        : id(i), name(n), lat(la), lng(lo) {}
};

class HashTable 
{

private:
    static const int TABLE_SIZE = 100;
    vector<list<Junction>> table;
    int elementCount;
    
    int hashFunction(int key) 
    {
        return key % TABLE_SIZE;
    }
    
public:
    HashTable() : table(TABLE_SIZE), elementCount(0) {}
    
    void insert(const Junction& junction) 
    {
        int index = hashFunction(junction.id);
        
        for (auto& j : table[index]) 
        {
            if (j.id == junction.id) 
            {
                j = junction;
                cout << "[HashTable] Updated: ID " << junction.id << endl;
                return;
            }
        }
        
        table[index].push_back(junction);
        elementCount++;

        cout << "[HashTable] Inserted: " << junction.name 
             << " at bucket " << index << endl;
    }
    
    Junction* search(int id) 
    {
        int index = hashFunction(id);
        
        for (auto& j : table[index]) 
        {
            if (j.id == id) 
            {
                cout << "[HashTable] Found: " << j.name << endl;
                return &j;
            }
        }
        
        cout << "[HashTable] Not found: ID " << id << endl;

        return nullptr;
    }
    
    void display() 
    {
        cout << "\n======== HASH TABLE CONTENTS ========" << endl;
        cout << "Total Elements: " << elementCount << endl;
        cout << "Load Factor: " << (double)elementCount / TABLE_SIZE << endl;
        cout << "-------------------------------------" << endl;
        
        int usedBuckets = 0;
    
        for (int i = 0; i < TABLE_SIZE; i++) 
        {
            if (!table[i].empty()) 
            {
                usedBuckets++;
                cout << "Bucket " << i << " (" << table[i].size() << " items): ";
            
                for (auto& j : table[i]) 
                {
                    cout << "[" << j.name << "] ";
                }
                
                cout << endl;
            }
        }
        
        cout << "Used Buckets: " << usedBuckets << " / " << TABLE_SIZE << endl;
        cout << "====================================\n" << endl;
    }
    
    int size() const 
    {
        return elementCount;
    }
};

#endif
