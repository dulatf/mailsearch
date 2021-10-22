#ifndef __UTIL_H__
#define __UTIL_H__
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <optional>
#include <regex>
#include <unordered_set>

// Split a string into words, omitting certain punctuations.
std::vector<std::string> split(const std::string &input);

// Helper function to read text file into string.
std::optional<std::string> read_file_to_string(const std::string &filename);

// Read file and return all unique words.
std::optional<std::unordered_set<std::string>> read_words_from_file(const std::string &filename);

struct Node
{
    std::unordered_map<char, int> children;
    std::unordered_set<int> files;
    // Serialization:
    // children.size()
    // files.size()
    // children[0].char, children[0].int
    // ...
    // children[n]....
    // files[0]
    // ...
    int serialized_size_in_bytes() const
    {
        return 2 * sizeof(int) + (sizeof(int) + sizeof(char)) * children.size() + sizeof(int) * files.size();
    }
    void serialize(std::ofstream &outfile) const
    {
        const int children_size = children.size();
        const int files_size = files.size();
        outfile.write(reinterpret_cast<const char *>(&children_size), sizeof(int));
        outfile.write(reinterpret_cast<const char *>(&files_size), sizeof(int));
        for (const auto &[character, index] : children)
        {
            outfile.write(reinterpret_cast<const char *>(&character), sizeof(char));
            outfile.write(reinterpret_cast<const char *>(&index), sizeof(int));
        }
        for (const auto &file_index : files)
        {
            outfile.write(reinterpret_cast<const char *>(&file_index), sizeof(int));
        }
    }
    static Node deserialize(std::ifstream &infile)
    {
        int children_size = 0;
        int files_size = 0;
        infile.read(reinterpret_cast<char *>(&children_size), sizeof(int));
        infile.read(reinterpret_cast<char *>(&files_size), sizeof(int));
        Node node;
        for (int i = 0; i < children_size; ++i)
        {
            char character = 0;
            int index = 0;
            infile.read(reinterpret_cast<char *>(&character), sizeof(char));
            infile.read(reinterpret_cast<char *>(&index), sizeof(int));
            node.children.insert({character, index});
        }
        for (int i = 0; i < files_size; ++i)
        {
            int file_index = 0;
            infile.read(reinterpret_cast<char *>(&file_index), sizeof(int));
            node.files.insert(file_index);
        }
        return node;
    }
};

#endif