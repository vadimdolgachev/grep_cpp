#include <cstdlib>
#include <iostream>

#include <boost/leaf/error.hpp>

#include "Grep.h"

struct ErrorParseCmdLineArgs final {
    const std::string text;
};

boost::leaf::result<std::tuple<std::string, std::string>>
parseCommandLineArgs(const int argc, const char *const argv[]) {
    if (argc != 3) {
        return boost::leaf::new_error(ErrorParseCmdLineArgs{"error parse input args"});
    }
    return std::make_tuple(argv[2], argv[1]);
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
