#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

#include "Grep.h"

namespace po = boost::program_options;

int main(const int argc, char *const argv[]) {
    try {
        po::options_description desc("text finder");
        desc.add_options()
                ("help", "--file file_path --text 'search text'")
                ("file", po::value<std::string>()->default_value(""), "input filePath")
                ("text", po::value<std::string>()->default_value(""), "search text");

        po::variables_map optionsVarsMap;
        po::store(po::parse_command_line(argc, argv, desc), optionsVarsMap);
        po::notify(optionsVarsMap);
        const auto &filePath = optionsVarsMap["file"].as<std::string>();
        const auto &searchText = optionsVarsMap["text"].as<std::string>();
        if (optionsVarsMap.count("help") > 0 || filePath.empty() || searchText.empty()) {
            std::cout << desc << "\n";
            return 0;
        }

        const auto &matches = Grep::findAllMatches(searchText, filePath);
        std::cout << matches.size() << "\n";
        for (const auto &match : matches) {
            std::cout << match.lineNumber << " "
                      << match.lineOffset << " "
                      << match.text << "\n";
        }
    } catch (const std::ifstream::failure &e) {
        std::cerr << "error: opening/reading/closing file" << "\n";
        return EXIT_FAILURE;
    } catch (std::exception &e) {
        std::cerr << "error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}