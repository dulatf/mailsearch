#include <iostream>
#include "util.h"
#include <unordered_map>
#include <filesystem>

class Trie
{

public:
    Trie()
    {
        // Insert a root node.
        new_node();
    }
    void insert(const std::string &word, int file_index)
    {
        int current_idx = 0; // start at root.
        for (const char &c : word)
        {
            if (node_buffer_[current_idx].children.find(c) == node_buffer_[current_idx].children.end())
            {
                int new_idx = new_node();
                node_buffer_[current_idx].children.insert({c, new_idx});
            }
            current_idx = node_buffer_[current_idx].children[c];
        }
        node_buffer_[current_idx].files.insert(file_index);
    }
    bool contains(const std::string &word) const
    {
        const int index = find_string(word);
        if (index >= 0)
        {
            return node_buffer_[index].files.size() > 0;
        }
        return false;
    }
    std::vector<int> files_for_word(const std::string &word) const
    {
        const int index = find_string(word);
        if (index < 0)
        {
            return {};
        }
        if (node_buffer_[index].files.size() == 0)
        { // not a word
            return {};
        }
        return {node_buffer_[index].files.begin(), node_buffer_[index].files.end()};
    }
    std::vector<std::string> all_words_with_prefix(const std::string &prefix) const
    {
        int prefix_idx = find_string(prefix);
        if (prefix_idx < 0)
        {
            return {};
        }
        std::vector<std::string> words;
        accumulate_words(prefix_idx, prefix, words);
        return words;
    }
    void accumulate_words(int index, std::string prefix,
                          std::vector<std::string> &words) const
    {
        if (node_buffer_[index].files.size() > 0)
        {
            words.push_back(prefix);
        }
        for (const auto &[c, child_idx] : node_buffer_[index].children)
        {
            std::string child_word = prefix;
            child_word.push_back(c);
            accumulate_words(child_idx, child_word, words);
        }
    }
    void print() const
    {
        for (int idx = 0; idx < node_buffer_.size(); ++idx)
        {
            std::cout << idx << ": ";
            for (const auto &[c, i] : node_buffer_[idx].children)
            {
                std::cout << "(" << c << ", " << i << ") ";
            }
            if (node_buffer_[idx].files.size() > 0)
            {
                std::cout << " $ ";
                for (const auto &f : node_buffer_[idx].files)
                {
                    std::cout << f << " ";
                }
            }
            std::cout << std::endl;
        }
    }

    void serialize(const std::string base_path) const
    {
        const std::string index_path = base_path + ".index";
        const std::string data_path = base_path + ".data";
        serialize_node_index(index_path);
        serialize_nodes(data_path);
    }

private:
    int new_node()
    {
        node_buffer_.push_back(Node());
        return node_buffer_.size() - 1;
    }
    // Index serialization:
    // num nodes
    // node 0 start in bytes
    // node 1 start in bytes
    void serialize_node_index(const std::string &filename) const
    {
        std::ofstream outfile(filename, std::ios::binary);
        const int nodes = node_buffer_.size();
        outfile.write(reinterpret_cast<const char *>(&nodes), sizeof(int));
        int position = 0;
        for (int i = 0; i < nodes; ++i)
        {
            outfile.write(reinterpret_cast<const char *>(&position), sizeof(int));
            position += node_buffer_[i].serialized_size_in_bytes();
        }
    }
    void serialize_nodes(const std::string &filename) const
    {
        std::ofstream outfile(filename, std::ios::binary);
        for (const auto &node : node_buffer_)
        {
            node.serialize(outfile);
        }
    }

public:
    int find_string(const std::string &word) const
    {
        int cur = 0;
        for (const char &c : word)
        {
            if (node_buffer_[cur].children.find(c) != node_buffer_[cur].children.end())
            { // found
                cur = node_buffer_.at(cur).children.at(c);
            }
            else
            { // not found
                return -1;
            }
        }
        return cur;
    }
    std::vector<Node> node_buffer_;
};

using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cout << "Usage: indexer mailbox_path database\n";
        return 1;
    }

    const std::string mailbox_path = std::string(argv[1]); //"./data/skilling-j";
    const std::string output_path = std::string(argv[2]);  //"./test_db";
    std::vector<std::string> file_index;
    Trie trie;
    for (const auto &entry : recursive_directory_iterator(mailbox_path))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }
        file_index.push_back(entry.path());
        const auto &words = read_words_from_file(entry.path());
        if (!words)
        {
            std::cerr << "Failed to read words from " << entry.path() << std::endl;
            return 1;
        }
        for (const auto &word : words.value())
        {
            trie.insert(word, file_index.size() - 1);
        }
    }
    // Save file index.
    std::ofstream outfile(output_path + ".files");
    for (const auto &file : file_index)
    {
        outfile << file << std::endl;
    }
    // Save lookup database.
    trie.serialize(output_path);

    return 0;
}