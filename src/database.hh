#include <string>
#include <vector>
#include <unordered_map>
#include <cstring>
//#include "../lib/qtl/qtl_sqlite.hpp"

#ifndef DATABASE_H
#define DATABASE_H

struct HFractalConfigProfile {
    long profile_id; // Primary key

    long double x_offset;
    long double y_offset;
    long double zoom;
    int iterations;
    std::string equation;
    std::string name;
    std::string preview_file_address;
    long user_id; // Foreign key of HFractalUserProfile

    HFractalConfigProfile () { memset (this, 0, sizeof(HFractalConfigProfile)); }
};

struct HFractalUserProfile {
    long user_id; // Primary key

    std::string user_name;

    HFractalUserProfile () { memset (this, 0, sizeof(HFractalUserProfile)); }
};

// namespace qtl {
//     template <> inline void bind_record<qtl::sqlite::statement, HFractalConfigProfile>(qtl::sqlite::statement& command, HFractalConfigProfile&& profile) {
// 		qtl::bind_fields(command, profile.profile_id, profile.x_offset, profile.y_offset, profile.zoom, profile.iterations, profile.equation, profile.name, profile.preview_file_address, profile.user_id);
// 	}

//     template <> inline void bind_record<qtl::sqlite::statement, HFractalUserProfile>(qtl::sqlite::statement& command, HFractalUserProfile&& profile) {
// 		qtl::bind_fields(command, profile.user_id, profile.user_name);
// 	}
// }

class HFractalDatabase {
private:
    std::string db_path;
    std::unordered_map<long, HFractalConfigProfile*> configs;
    std::unordered_map<long, HFractalUserProfile*> users;
    static std::string forCSVInner (std::string);
    std::vector<std::string> componentify (std::string);
    std::string fixDoubleQuote (std::string);

    static std::string forCSV (std::string);
    static std::string forCSV (long);
    static std::string forCSV (int);
    static std::string forCSV (long double);
public:
    HFractalDatabase (std::string);

    std::vector<std::pair<long, std::string>> getConfigDescriptions ();
    HFractalConfigProfile* getConfig (long);
    HFractalUserProfile* getUser (long);

    long insertConfig (HFractalConfigProfile*);
    long insertUser (HFractalUserProfile*);

    bool removeConfig (long);
    bool removeUser (long);
    
    bool commit ();
    bool read ();
};

#endif