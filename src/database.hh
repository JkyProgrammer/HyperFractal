// #include <string>
// #include <vector>
// #include "../lib/qtl/qtl_mysql.hpp"

// #ifndef DATABASE_H
// #define DATABASE_H

// struct HFractalConfigProfile {
//     long profile_id;

//     long double x_offset;
//     long double y_offset;
//     long double zoom;
//     int iterations;
//     std::string equation;
//     std::string name;
//     std::string preview_filename;
//     long user_id;

//     HFractalConfigProfile () { memset (this, 0, sizeof(HFractalConfigProfile)); }
// };

// struct HFractalUserProfile {
//     long user_id;

//     std::string user_name;

//     HFractalUserProfile () { memset (this, 0, sizeof(HFractalUserProfile)); }
// }

// namespace qtl {
//     template <> inline void bind_record<qtl::mysql::statement, HFractalConfigProfile>(qtl::mysql::statement& command, HFractalConfigProfile&& profile) {
// 		qtl::bind_fields(command, profile.profile_id, profile.x_offset, profile.y_offset, profile.zoom, profile.iterations, profile.equation, profile.name, profile.preview_filename, profile.user_id);
// 	}

//     template <> inline void bind_record<qtl::mysql::statement, HFractalUserProfile>(qtl::mysql::statement& command, HFractalUserProfile&& profile) {
// 		qtl::bind_fields(command, profile.user_id, profile.user_name);
// 	}
// }

// class HFractalDatabase {
// private:
//     std::string db_path;
// public:
//     HFractalDatabase (std::string);

//     int countRecords (T record_type);
//     std::vector<std::pair<long, std::string>> getConfigDescriptions ();
//     HFractalConfigProfile* readConfig (long);
//     HFractalUserProfile* readUser (long);
//     bool writeConfig (long, HFractalConfigProfile*);
//     bool writeUser (long, HFractalUserProfile*);
// }

// #endif