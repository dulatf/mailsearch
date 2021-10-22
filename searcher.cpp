#include <iostream>
#include "util.h"

std::vector<int> read_node_index(const std::string &filename)
{
    std::ifstream infile(filename, std::ios::binary);
    int num_nodes = 0;
    infile.read(reinterpret_cast<char *>(&num_nodes), sizeof(int));
    std::vector<int> node_offsets;
    node_offsets.reserve(num_nodes);
    for (int i = 0; i < num_nodes; ++i)
    {
        int offset = 0;
        infile.read(reinterpret_cast<char *>(&offset), sizeof(int));
        node_offsets.push_back(offset);
    }
    return node_offsets;
}

std::vector<std::string> read_file_index(const std::string &filename)
{
    std::vector<std::string> files;
    std::ifstream infile(filename);
    std::string buffer;
    while (std::getline(infile, buffer))
    {
        files.push_back(buffer);
    }
    return files;
}

class StreamedTrie
{
public:
    explicit StreamedTrie(const std::string base_path)
    {
        data_path_ = base_path + ".data";
        const std::string index_path = base_path + ".index";
        node_offsets_ = read_node_index(index_path);
        loaded_node_index_ = -1;
    }
    // Loads the given node from disk.
    bool load_node(int node_index)
    {
        if (node_index >= node_offsets_.size())
        {
            loaded_node_index_ = -1;
            return false;
        }
        if (loaded_node_index_ == node_index)
        {
            // Already loaded.
            return true;
        }
        loaded_node_index_ = node_index;
        std::ifstream infile(data_path_, std::ios::binary);
        infile.seekg(node_offsets_[node_index]);
        loaded_node_ = Node::deserialize(infile);
        return true;
    }
    const Node &get_loaded_node() const
    {
        return loaded_node_;
    }
    int get_loaded_node_index() const
    {
        return loaded_node_index_;
    }
    int find_string(const std::string &word)
    {
        if (!load_node(0))
        {
            return -1;
        }
        for (const char &c : word)
        {
            if (loaded_node_.children.find(c) != loaded_node_.children.end())
            {
                if (!load_node(loaded_node_.children.at(c)))
                {
                    return -1;
                }
            }
            else
            {
                return -1;
            }
        }
        return loaded_node_index_;
    }
    std::vector<int> all_files_containing_prefix(const std::string &prefix)
    {
        const int pref_idx = find_string(prefix);
        if (pref_idx < 0)
        {
            return {};
        }
        std::unordered_set<int> files;
        accumulate_files(pref_idx, files);
        return std::vector<int>(files.begin(), files.end());
    }
    void accumulate_files(int index, std::unordered_set<int> &files)
    {
        if (!load_node(index))
        {
            return;
        }
        if (loaded_node_.files.size() > 0)
        {
            files.insert(loaded_node_.files.begin(), loaded_node_.files.end());
        }
        // TODO: This should be optimized
        Node node_copy = loaded_node_;
        for (const auto &[c, child_index] : node_copy.children)
        {
            accumulate_files(child_index, files);
        }
    }

private:
    std::string data_path_;
    std::vector<int> node_offsets_;

    int loaded_node_index_ = 0;
    Node loaded_node_;
};

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cout << "Usage: searcher database prefix" << std::endl;
        return 0;
    }
    const std::string database_path = std::string(argv[1]);
    const std::string query = std::string(argv[2]);
    std::cerr << "Searching for " << query << std::endl;
    StreamedTrie trie(database_path);
    const std::vector<std::string> file_index = read_file_index(database_path + ".files");
    std::vector<int> file_indices = trie.all_files_containing_prefix(query);
    for (const auto &x : file_indices)
    {
        std::cout << file_index[x] << std::endl;
    }
}