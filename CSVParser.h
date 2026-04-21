#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <string>
#include <vector>
#include <unordered_set>

class CSVParser {
public:
    static void peek(const std::string& filename);
    static std::vector<std::string> parseLine(const std::string& line);
    static bool validateAndParse(const std::string& filename, const std::string& primary_col, std::vector<std::vector<std::string>>& valid_records, int& pk_index);
};

#endif // CSV_PARSER_H