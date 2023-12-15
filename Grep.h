//
// Created by vadim on 12.12.23.
//

#ifndef MTFIND_GREP_H
#define MTFIND_GREP_H

#include <list>
#include <string>
#include <thread>
#include <utility>

#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>
#include <boost/leaf/result.hpp>

#include <boost/leaf.hpp>

namespace Grep {
    struct Match final {
        Match(const std::size_t lineCount, const std::size_t lineOffset, std::string text)
            : lineCount(lineCount), lineOffset(lineOffset), text(std::move(text)) {}

        const std::size_t lineCount;
        const std::size_t lineOffset;
        const std::string text;
    };

    struct ErrorOpenFile {
        const std::string text;
    };

    struct ErrorExceededMaxFileSize {
        const std::string text;
    };

    struct ErrorExceededSearchTextSize {
        const std::string text;
    };

    boost::leaf::result<std::list<Match>>
    findAllMatches(const std::string &searchText, const std::string &filePath,
                   std::size_t threadCount = std::thread::hardware_concurrency()) noexcept;
}; // namespace Grep

#endif // MTFIND_GREP_H
