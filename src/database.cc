#include "database.hh"

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

std::string HFractalDatabase::forCSVInner (std::string s) {
    string fixed_quotes = "";
    for (char c : s) {
        fixed_quotes += c;
        if (c == '\"') fixed_quotes += '\"';
    }
    fixed_quotes = "\"" + fixed_quotes + "\"";
    return fixed_quotes;
}

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

HFractalDatabase::HFractalDatabase (std::string path) {
    int cutoff = path.find(".csv");
    db_path = path.substr (0, cutoff);

    if (!read()) if (!commit ()) throw new std::runtime_error ("unable to create database");
}

long HFractalDatabase::insertConfig (HFractalConfigProfile* c) {
    long max_id = -1;
    for (pair<long, HFractalConfigProfile*> p : configs)
        if (p.first > max_id) max_id = p.first;
    
    c->profile_id = max_id+1; 
    configs.emplace (c->profile_id, c);
    return c->profile_id;
}

long HFractalDatabase::insertUser (HFractalUserProfile* u) {
    long max_id = -1;
    for (pair<long, HFractalUserProfile*> p : users)
        if (p.first > max_id) max_id = p.first;
    
    u->user_id = max_id+1;
    users.emplace (u->user_id, u);
    return u->user_id;
}

bool HFractalDatabase::removeConfig (long id) {
    return configs.erase (id);
}

bool HFractalDatabase::removeUser (long id) {
    return users.erase (id);
}

bool HFractalDatabase::commit () {
    string db_path_configs = db_path + "_configs.csv";
    string db_path_users = db_path + "_users.csv";
    ofstream db_file_configs (db_path_configs.c_str());
    ofstream db_file_users (db_path_users.c_str());

    if (db_file_configs.is_open() && db_file_users.is_open()) {
        //db_file_configs << "profile_id,x_offset,y_offset,zoom,iterations,equation,name,preview_file_address,user_id" << endl;
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
            line += forCSV (config->preview_file_address) + ",";
            line += forCSV (config->user_id);
            db_file_configs << line.c_str() << endl;
        }

        //db_file_users << "user_id,user_name" << endl;
        for (auto upair : users) {
            HFractalUserProfile* user = upair.second;
            string line = "";
            line += forCSV (user->user_id) + ",";
            line += forCSV (user->user_name);
            db_file_users << line.c_str() << endl;
        }

        db_file_configs.close();
        db_file_users.close();
        return true;
    } else return false;
}

bool HFractalDatabase::read () {
    string db_path_configs = db_path + "_configs.csv";
    string db_path_users = db_path + "_users.csv";
    ifstream db_file_configs (db_path_configs.c_str());
    ifstream db_file_users (db_path_users.c_str());

    if (db_file_configs.is_open() && db_file_users.is_open()) {
        string line;

        int line_number = 0;

        configs.clear();
        HFractalConfigProfile* config;
        while (getline (db_file_configs, line)) {
            try {
                vector<string> components = componentify (line);
                config = new HFractalConfigProfile ();
                cout << components[0] << endl;
                config->profile_id = stol(components[0]);
                config->x_offset = stold(components[1]);
                config->y_offset = stold(components[2]);
                config->zoom = stold(components[3]);
                config->iterations = stoi(components[4]);
                config->equation = components[5];
                config->name = components[6];
                config->preview_file_address = components[7];
                config->user_id = stol(components[8]);
                configs.emplace (config->profile_id, config);
            } catch (std::invalid_argument e) {
                cout << "Failed to read config profile on line " << line_number << endl;
            }
            line_number++;
        }

        line_number = 0;

        users.clear();
        HFractalUserProfile* user;
        while (getline (db_file_users, line)) {
            try {
                vector<string> components = componentify (line);
                user = new HFractalUserProfile ();
                user->user_id = stol(components[0]);
                user->user_name = components[1];
                users.emplace (user->user_id, user);
            } catch (std::invalid_argument e) {
                cout << "Failed to read user profile on line " << line_number << endl;
            }
            line_number++;
        }
        return true;
    } else return false;
}

std::string HFractalDatabase::forCSV (std::string s) {
    return forCSVInner (s);
}

std::string HFractalDatabase::forCSV (long l) {
    return forCSVInner (to_string(l));
}

std::string HFractalDatabase::forCSV (long double ld) {
    return forCSVInner (to_string(ld));
}

std::string HFractalDatabase::forCSV (int i) {
    return forCSVInner (to_string(i));
}


