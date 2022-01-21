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

HFractalDatabase::HFractalDatabase (std::string path) {
    int cutoff = path.find(".csv");
    db_path = path.substr (0, cutoff);

    if (!read()) if (!commit ()) throw new std::runtime_error ("unable to create database");
}

long HFractalDatabase::insertConfig (HFractalConfigProfile* c) {
    if (configs.size() > 0)
        c->profile_id = configs.at (configs.size()-1)->profile_id + 1;
    else
        c->profile_id = 0;
    configs.emplace (c->profile_id, c);
    return c->profile_id;
}

long HFractalDatabase::insertUser (HFractalUserProfile* u) {
    if (users.size() > 0)
        u->user_id = users.at (users.size()-1)->user_id + 1;
    else
        u->user_id = 0;
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
        db_file_configs << "profile_id,x_offset,y_offset,zoom,iterations,equation,name,preview_file_address,user_id" << endl;
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

        db_file_users << "user_id,user_name" << endl;
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
        // TODO: read aaaaaaaaa

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


