#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <optional>
#include <regex>
#include <unordered_set>

// Split a string into words, omitting certain punctuations.
std::vector<std::string> split(const std::string &input)
{
    std::regex re("[\\!\\?\'\t \n,.:-]");
    std::sregex_token_iterator iter(input.begin(), input.end(), re, -1);
    const std::vector<std::string> tokens(iter, std::sregex_token_iterator());
    std::vector<std::string> result;
    std::copy_if(tokens.begin(), tokens.end(), std::back_inserter(result),
                 [](const std::string &token)
                 {
                     return !token.empty();
                 });
    return result;
}

// Helper function to read text file into string.
std::optional<std::string> read_file_to_string(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file " << filename << std::endl;
        return std::nullopt;
    }
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

// Read file and return all unique words.
std::optional<std::unordered_set<std::string>> read_words_from_file(const std::string &filename)
{
    const std::optional<std::string> contents = read_file_to_string(filename);
    if (!contents)
    {
        return std::nullopt;
    }
    // TODO: Skip headers
    const std::vector<std::string> tokens = split(contents.value());
    return std::unordered_set<std::string>(tokens.begin(), tokens.end());
}
