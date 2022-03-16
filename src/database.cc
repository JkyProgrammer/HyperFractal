// src/database.cc

#include "database.hh"

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

/**
 * @brief Modify a string so that it can be written to CSV properly. This means replacing double quotes with a pair of consecutive double quotes
 * 
 * @param s Input string
 * @return A ready-to-write result string
 */
std::string HFractalDatabase::forCSVInner (std::string s) {
    string fixed_quotes = "";
    for (char c : s) {
        fixed_quotes += c;
        if (c == '\"') fixed_quotes += '\"';
    }
    fixed_quotes = "\"" + fixed_quotes + "\"";
    return fixed_quotes;
}

/**
 * @brief Break a record from a CSV file into a sequence of raw string fields
 * 
 * @param line Line from CSV file to process
 * @return std::vector of raw string fields (require additional processing)
 */
std::vector<std::string> HFractalDatabase::componentify (std::string line) {
    vector<string> ret_value;
    string current_component;
    int start_index = line.find ("\"");
    int end_index = -1;
    while (start_index < line.length()) {
        end_index = line.find ("\",\"", start_index);
        if (end_index == string::npos) end_index = line.find ("\"", start_index+1);
        current_component = line.substr (start_index+1, end_index-(start_index+1));
        current_component = fixDoubleQuote (current_component);
        ret_value.push_back (current_component);
        start_index = end_index+2;
    }
    return ret_value;
}

/**
 * @brief Remove double-double quotes from a string. Effectively the reverse of forCSVInner
 * 
 * @param s String field to process
 * @return Cleaned-up field string
 */
std::string HFractalDatabase::fixDoubleQuote (std::string s) {
    string result = "";
    char current_char = '\0';
    char next_char = '\0';
    for (int i = 0; i < s.length(); i++) {
        current_char = s[i];
        next_char = (i+1 < s.length()) ? s[i+1] : '\0';
        result.push_back (current_char);
        if (current_char == '\"' && next_char == '\"') i++;
    }
    return result;
}

/**
 * @brief Construct a database instance using a base path to CSV files
 * 
 * @param path Base path to the target CSV database. Individual tables must be stored as separate CSV files, so the base path is modified provide paths to the invidiual CSV files
 */
HFractalDatabase::HFractalDatabase (std::string path) {
    // Generate and assign the base path
    int cutoff = path.find(".csv");
    db_path = path.substr (0, cutoff);
    
    // Try to read the database, failing that write a new one, failing that, error out
    if (!read()) if (!commit ()) {
        cout << "fuc" << endl;
        throw new std::runtime_error ("unable to create database");
    }
}

HFractalDatabase::HFractalDatabase () {}

/**
 * @brief Get a list of config profile descriptions paired with their IDs. Allows the GUI to easily grab a profile summary without having to fetch all the data one-by-one
 * 
 * @return std::vector of pairs of ID and string values
 */
std::vector<std::pair<long, std::string>> HFractalDatabase::getConfigDescriptions () {
    std::vector<std::pair<long, std::string>> ret_val;
    for (auto conf : configs) {
        std::pair<long, std::string> desc_pair;
        desc_pair.first = conf.first;
        desc_pair.second = (
            conf.second->name
            + " ("
            + conf.second->equation
            + ")");
        ret_val.push_back (desc_pair);
    }
    return ret_val;
}

/**
 * @brief Function to get a configuration profile by its ID
 * 
 * @param id The ID of the profile
 * @return A pointer to the configuration profile
 */
HFractalConfigProfile* HFractalDatabase::getConfig (long id) {
    HFractalConfigProfile *ret = NULL;
    if (configs.count(id) != 0) ret = configs[id];
    return ret;
}

/**
 * @brief Function to get a user profile by its ID
 * 
 * @param id The ID of the profile
 * @return A pointer to the user profile
 */
HFractalUserProfile* HFractalDatabase::getUser (long id) {
    HFractalUserProfile *ret = NULL;
    if (users.count(id) != 0) ret = users[id];
    return ret;
}

/**
 * @brief Insert a configuration profile into the database. Automatically generates and assigns a unique ID
 * 
 * @param c Pointer to the config profile being inserted
 * @return Generated ID of the profile
 */
long HFractalDatabase::insertConfig (HFractalConfigProfile* c) {
    long max_id = -1;
    for (pair<long, HFractalConfigProfile*> p : configs)
        if (p.first > max_id) max_id = p.first;
    
    c->profile_id = max_id+1; 
    configs.emplace (c->profile_id, c);
    return c->profile_id;
}

/**
 * @brief Insert a user profile into the database. Automatically generates and assigns a unique ID
 * 
 * @param u Pointer to the user profile being inserted
 * @return Generated ID of the profile
 */
long HFractalDatabase::insertUser (HFractalUserProfile* u) {
    long max_id = -1;
    for (pair<long, HFractalUserProfile*> p : users)
        if (p.first > max_id) max_id = p.first;
    
    u->user_id = max_id+1;
    users.emplace (u->user_id, u);
    return u->user_id;
}

/**
 * @brief Delete a config profile record from the database
 * 
 * @param id ID of the profile to be deleted
 * @return True if the delete succeeded, False if the record did not exist
 */
bool HFractalDatabase::removeConfig (long id) {
    return configs.erase (id);
}

/**
 * @brief Delete a user profile record from the database
 * 
 * @param id ID of the profile to be deleted
 * @return True if the delete succeeded, False if the record did not exist
 */
bool HFractalDatabase::removeUser (long id) {
    return users.erase (id);
}

/**
 * @brief Write out the contents of the cached database to CSV files. 
 * 
 * @return True if the write succeeded, False if it failed
 */
bool HFractalDatabase::commit () {
    // Create path strings and file streams
    string db_path_configs = db_path + "_configs.csv";
    string db_path_users = db_path + "_users.csv";
    ofstream db_file_configs (db_path_configs.c_str());
    ofstream db_file_users (db_path_users.c_str());

    // If the file streams are open, write data
    if (db_file_configs.is_open() && db_file_users.is_open()) {
        // Iterate over config files
        for (auto copair : configs) {
            HFractalConfigProfile* config = copair.second;
            string line = "";
            line += forCSV (config->profile_id) + ",";
            line += forCSV (config->x_offset) + ",";
            line += forCSV (config->y_offset) + ",";
            line += forCSV (config->zoom) + ",";
            line += forCSV (config->iterations) + ",";
            line += forCSV (config->equation) + ",";
            line += forCSV (config->name) + ",";
            line += forCSV (config->palette) + ",";
            line += forCSV (config->user_id);
            db_file_configs << line.c_str() << endl;
        }

        // Iterate over user files
        for (auto upair : users) {
            HFractalUserProfile* user = upair.second;
            string line = "";
            line += forCSV (user->user_id) + ",";
            line += forCSV (user->user_name);
            db_file_users << line.c_str() << endl;
        }

        // Close the files and return success
        db_file_configs.close();
        db_file_users.close();
        return true;
    } else return false; // Return failure
}

/**
 * @brief Read the contents of CSV files into the cached database
 * 
 * @return True if the read succeeded, False if it failed
 */
bool HFractalDatabase::read () {
    // Create paths and file streams
    string db_path_configs = db_path + "_configs.csv";
    string db_path_users = db_path + "_users.csv";
    ifstream db_file_configs (db_path_configs.c_str());
    ifstream db_file_users (db_path_users.c_str());

    // If the file streams are open, read data
    if (db_file_configs.is_open() && db_file_users.is_open()) {
        string line;
        int line_number = 0;

        configs.clear();
        // Read config profiles
        HFractalConfigProfile* config;
        while (getline (db_file_configs, line)) {
            try {
                // Attempt to convert the record to a config profile
                vector<string> components = componentify (line);
                config = new HFractalConfigProfile ();
                config->profile_id = stol(components[0]);
                config->x_offset = stold(components[1]);
                config->y_offset = stold(components[2]);
                config->zoom = stold(components[3]);
                config->iterations = stoi(components[4]);
                config->equation = components[5];
                config->name = components[6];
                config->palette = stoi(components[7]);
                config->user_id = stol(components[8]);
                configs.emplace (config->profile_id, config);
            } catch (std::invalid_argument e) {
                // Print a console error if a line could not be read
                cout << "Failed to read config profile on line " << line_number << endl;
            }
            line_number++;
        }

        line_number = 0;

        // Read user profiles
        users.clear();
        HFractalUserProfile* user;
        while (getline (db_file_users, line)) {
            try {
                // Attempt to convert the record to a user profile
                vector<string> components = componentify (line);
                user = new HFractalUserProfile ();
                user->user_id = stol(components[0]);
                user->user_name = components[1];
                users.emplace (user->user_id, user);
            } catch (std::invalid_argument e) {
                // Print a console error if a line could not be read
                cout << "Failed to read user profile on line " << line_number << endl;
            }
            line_number++;
        }
        // Return success
        return true;
    } else return false; // Return failure
}

/**
 * @brief Generate a CSV-happy string from a given input
 * 
 * @param s String input
 * @return CSV-writeable string
 */
std::string HFractalDatabase::forCSV (std::string s) {
    return forCSVInner (s);
}

/**
 * @brief Generate a CSV-happy string from a given input
 * 
 * @param l Long integer input
 * @return CSV-writeable string
 */
std::string HFractalDatabase::forCSV (long l) {
    return forCSVInner (to_string(l));
}

/**
 * @brief Generate a CSV-happy string from a given input
 * 
 * @param ld Long double input
 * @return CSV-writeable string
 */
std::string HFractalDatabase::forCSV (long double ld) {
    return forCSVInner (to_string(ld));
}

/**
 * @brief Generate a CSV-happy string from a given input
 * 
 * @param i Integer input
 * @return CSV-writeable string
 */
std::string HFractalDatabase::forCSV (int i) {
    return forCSVInner (to_string(i));
}


