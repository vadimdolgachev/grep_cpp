//
// Created by vadim on 12.12.23.
//

#ifndef MTFIND_GREP_H
#define MTFIND_GREP_H

#include <thread>
#include <string>
#include <list>
#include <utility>

#include <boost/format.hpp>
#include <boost/algorithm/string/replace.hpp>

namespace Grep {
    struct Match final {
        Match(const std::size_t lineCount,
              const std::size_t lineOffset,
              std::string text) :
                lineCount(lineCount),
                lineOffset(lineOffset),
                text(std::move(text)) {

        }

        const std::size_t lineCount;
        const std::size_t lineOffset;
        const std::string text;
    };

    std::list<Match> findAllMatches(const std::string& searchText,
                                    const std::string& filePath,
                                    std::size_t threadCount = std::thread::hardware_concurrency());
};


#endif //MTFIND_GREP_H
