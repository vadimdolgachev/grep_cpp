//
// Created by vadim on 12.12.23.
//

#include <iostream>
#include <future>
#include <numeric>
#include <regex>
#include <fstream>

#include "Grep.h"

constexpr std::size_t MAX_FILE_SIZE_BYTES = 1024UL * 1024 * 1024;
constexpr std::size_t MAX_SEARCH_TEXT_LENGTH = 1000;

namespace {
    struct BlockSearchResult final {
        BlockSearchResult(const std::size_t lineNumbersInBlock,
                          std::list<Grep::Match> matches) :
                lineCount(lineNumbersInBlock),
                matches(std::move(matches)) {

        }

        const std::size_t lineCount;
        const std::list<Grep::Match> matches;
    };

    struct SearchRequest final {
        SearchRequest(const std::fstream::off_type startOffset,
                      const std::size_t blockLength,
                      const std::string &regExpText) :
                startOffset(startOffset),
                blockLength(blockLength),
                regExpText(makeRegExp(regExpText)) {

        }

        const std::fstream::off_type startOffset;
        const std::size_t blockLength;
        const std::string regExpText;

    private:
        static std::string makeRegExp(const std::string &text) {
            return str(boost::format("(%1%)") % boost::replace_all_copy(text, "?", "[\\w\\s]"));
        }
    };

    inline std::ifstream openFile(const std::string &name, const std::ios::openmode openMode) {
        std::ifstream file(name, openMode);
        file.exceptions(std::ios::failbit | std::ios::badbit);
        return file;
    }

    inline std::size_t getFileLength(const std::string &fileName) {
        return openFile(fileName, std::ios::binary | std::ios::ate).tellg();
    }

    std::tuple<BlockSearchResult, std::ifstream> findMatches(const SearchRequest &searchRequest, std::ifstream file) {
        file.seekg(searchRequest.startOffset, std::ios::beg);

        std::list<Grep::Match> matches;
        std::size_t lineNumber = 0;
        const std::regex re(searchRequest.regExpText);
        std::size_t readChars = 0;
        std::string line;
        while (readChars < searchRequest.blockLength && std::getline(file, line, '\n')) {
            for (auto it = std::sregex_iterator(line.begin(), line.end(), re);
                 it != std::sregex_iterator();
                 ++it) {
                const auto &match = *it;
                if (match.size() > 1) {
                    const auto &matchText = match.str(1);
                    matches.emplace_back(lineNumber,
                                         it->position(1),
                                         matchText);
                }
            }

            readChars += line.size() + 1; // length + delimiter
            lineNumber++;
        }
        return {BlockSearchResult(lineNumber, matches), std::move(file)};
    }
}  // namespace

std::list<Grep::Match> Grep::findAllMatches(const std::string &searchText,
                                            const std::string &filePath,
                                            const std::size_t threadCount) {
    auto file = openFile(filePath, std::ios::in);
    const auto fileSize = getFileLength(filePath);
    std::cout << "file size: " << fileSize << "\nfile path: '" << filePath << "'\ntext: '" << searchText << "'\n";

    if (fileSize > MAX_FILE_SIZE_BYTES) {
        throw std::invalid_argument(str(boost::format("maximum file size exceeded %1% > %2%")
                                        % fileSize % MAX_FILE_SIZE_BYTES));
    }

    if (searchText.length() > MAX_SEARCH_TEXT_LENGTH) {
        throw std::invalid_argument(str(boost::format("maximum search text size exceeded %1% > %2%")
                                        % searchText.length() % MAX_SEARCH_TEXT_LENGTH));
    }

    const std::size_t chunkSize = fileSize / threadCount;
    std::list<std::ifstream> filePool;
    std::list<BlockSearchResult> blocksSearchResult;

    std::size_t docNumberOfLines = 0;
    std::size_t docPos = 0;
    while (docPos < fileSize) {
        std::list<std::future<std::tuple<BlockSearchResult, std::ifstream>>> futures;
        for (std::size_t i = 0; i < threadCount && docPos < fileSize; ++i) {
            const auto startOffset = static_cast<std::ifstream::off_type>(docPos);
            auto endOffset = startOffset + chunkSize;
            if (endOffset > fileSize) {
                endOffset = fileSize;
            } else {
                file.seekg(static_cast<std::ifstream::off_type>(endOffset), std::ios::beg);
                // идем вперед, ищем конец строки
                while (endOffset < fileSize) {
                    if (file.peek() == '\n' && file.get() != '\n') {
                        break;
                    }
                    ++endOffset;
                }
            }
            if (filePool.empty()) {
                filePool.push_back(openFile(filePath, std::ios::in));
            }
            // first task is executed in thread function other in separate threads
            futures.emplace_back(
                    std::async(i == 0 ? std::launch::deferred : std::launch::async,
                               findMatches, SearchRequest(startOffset,
                                                          endOffset - startOffset,
                                                          searchText),
                               std::move(filePool.back())));
            filePool.pop_back();
            docPos = endOffset;
        }

        for (auto &f: futures) {
            auto [res, taskFile] = f.get();
            taskFile.clear();
            filePool.push_back(std::move(taskFile));
            blocksSearchResult.push_back(res);
        }
    }

    std::list<Match> resultMatches;
    for (const auto &result: blocksSearchResult) {
        assert(result.lineCount > 0);
        for (const auto &match: result.matches) {
            resultMatches.emplace_back(docNumberOfLines + match.lineCount, match.lineOffset, match.text);
        }
        docNumberOfLines += result.lineCount;
    }
    std::cout << "total number of lines: " << docNumberOfLines << "\n";
    return resultMatches;
}