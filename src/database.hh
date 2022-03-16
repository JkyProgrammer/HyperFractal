// src/database.hh

#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <cstring>

// Struct describing the Config Profile record type
struct HFractalConfigProfile {
    long profile_id; // Primary key

    long double x_offset;
    long double y_offset;
    long double zoom;
    int iterations;
    std::string equation;
    std::string name;
    int palette;
    long user_id; // Foreign key of HFractalUserProfile

    HFractalConfigProfile () { memset (this, 0, sizeof(HFractalConfigProfile)); }
};

// Struct describing the User Profile record type
struct HFractalUserProfile {
    long user_id; // Primary key

    std::string user_name;

    HFractalUserProfile () { memset (this, 0, sizeof(HFractalUserProfile)); }
};

// Class for managing the database of users and configurations and providing access to data for the GUI
class HFractalDatabase {
private:
    std::string db_path; // Base path to the database
    std::unordered_map<long, HFractalConfigProfile*> configs; // Map of config profiles against their IDs
    std::unordered_map<long, HFractalUserProfile*> users; // Map of user profiles against their IDs
    static std::string forCSVInner (std::string); // Static function to convert a string into a form CSVs will read/write properly
    std::vector<std::string> componentify (std::string); // Break a line of CSV into fields
    std::string fixDoubleQuote (std::string); // Remove double quotes in strings read from CSV file

    static std::string forCSV (std::string); // Generate a string which can be written to a CSV file as a field of a record
    static std::string forCSV (long); // Generate a string which can be written to a CSV file as a field of a record
    static std::string forCSV (int); // Generate a string which can be written to a CSV file as a field of a record
    static std::string forCSV (long double); // Generate a string which can be written to a CSV file as a field of a record
public:
    HFractalDatabase (std::string); // Initialise the database from a given base path
    HFractalDatabase (); // Dead initialiser for implicit instantiation

    std::vector<std::pair<long, std::string>> getConfigDescriptions (); // Generate a list of ID and description pairs for the GUI to display
    HFractalConfigProfile* getConfig (long); // Get a pointer to the config profile with a given ID
    HFractalUserProfile* getUser (long); // Get a pointer to the user profile with a given ID

    long insertConfig (HFractalConfigProfile*); // Insert a new config record and return its ID
    long insertUser (HFractalUserProfile*); // Insert a new user record and return its ID

    bool removeConfig (long); // Remove a config record by ID
    bool removeUser (long); // Remove a user record by ID
    
    bool commit (); // Write the contents of the cached database out to CSV files
    bool read (); // Read the contents of a CSV file into the database cache
};

#endif