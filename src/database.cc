#include "database.hh"

#include <vector>
#include <string>

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
    db_path = path;

    if (!read()) 
        if (!commit ()) throw new std::runtime_error ("unable to create database");
}

bool HFractalDatabase::commit () {
    string db_path_configs = db_path + "_configs.csv";
    string db_path_users = db_path + "_users.csv";
    FILE *db_file_configs;
    FILE *db_file_users;
    db_file_configs = fopen(db_path_configs.c_str(),"wb");
    db_file_users = fopen(db_path_users.c_str(),"wb");

    fprintf(db_file_configs,"profile_id,x_offset,y_offset,zoom,iterations,equation,name,preview_file_address,user_id");
    for (HFractalConfigProfile* config : configs) {
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
        fprintf(db_file_configs,line.c_str());
    }

    fprintf(db_file_users,"user_id,user_name");
    for (HFractalUserProfile* user : users) {
        string line = "";
        line += forCSV (user->user_id) + ",";
        line += forCSV (user->user_name);
        fprintf(db_file_users,line.c_str());
    }

    fclose(db_file_configs);
    fclose(db_file_users);
}

bool HFractalDatabase::read () {
    
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


