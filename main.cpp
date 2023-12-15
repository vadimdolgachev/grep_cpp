#include <cstdlib>
#include <iostream>

#include <boost/leaf/error.hpp>
#include <boost/program_options.hpp>

#include "Grep.h"

namespace po = boost::program_options;

struct ErrorParseCmdLineArgs {
    const std::string text;
};

boost::leaf::result<std::tuple<std::string, std::string>>
parseCommandLineArgs(const int argc, char *const argv[]) {
    po::options_description desc("text finder");
    desc.add_options()("help", "--file file_path --text 'search text'")(
            "file", po::value<std::string>()->default_value(""),
            "input filePath")("text", po::value<std::string>()->default_value(""), "search text");

    po::variables_map optionsVarsMap;
    po::store(po::parse_command_line(argc, argv, desc), optionsVarsMap);
    po::notify(optionsVarsMap);
    const auto &filePath = optionsVarsMap["file"].as<std::string>();
    const auto &searchText = optionsVarsMap["text"].as<std::string>();
    if (optionsVarsMap.count("help") > 0 || filePath.empty() || searchText.empty()) {
        return boost::leaf::new_error(ErrorParseCmdLineArgs{(std::stringstream() << desc).str()});
    }
    return std::make_tuple(searchText, filePath);
}

int
main(const int argc, char *const argv[]) {
    return boost::leaf::try_handle_all(
            [&]() -> boost::leaf::result<int> {
                BOOST_LEAF_AUTO(const args, parseCommandLineArgs(argc, argv));
                const auto [searchText, filePath] = args;
                BOOST_LEAF_AUTO(const matches, Grep::findAllMatches(searchText, filePath));
                std::cout << matches.size() << "\n";
                for (const auto &match: matches) {
                    std::cout << match.lineCount + 1 << " " << match.lineOffset + 1 << " "
                              << match.text << "\n";
                }
                return EXIT_SUCCESS;
            },

            [](const ErrorParseCmdLineArgs &e) {
                std::cerr << e.text << "\n";
                return EXIT_SUCCESS;
            },

            [](const Grep::ErrorExceededMaxFileSize &e) {
                std::cerr << e.text << "\n";
                return EXIT_SUCCESS;
            },

            [](const Grep::ErrorExceededSearchTextSize &e) {
                std::cerr << e.text << "\n";
                return EXIT_SUCCESS;
            },

            [](const Grep::ErrorOpenFile &e) {
                std::cerr << e.text << "\n";
                return EXIT_FAILURE;

            },

            [](boost::leaf::error_info const &) {
                std::cerr << "Unknown failure detected\n";
                return EXIT_FAILURE;
            });
}
