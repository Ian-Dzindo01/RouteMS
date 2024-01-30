#include <cstdlib>
#include <ctime>
#include <optional>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>


static std::optional<std::vector<std::byte>> ReadFile(const std::string &path) {
    std::ifstream is{path, std::ios::binary | std::ios::ate};       // ate sets file pointer to end of file

    if(!is)
        return std::nullopt;        // absence of value
    
    auto size = is.tellg();         // current position of get pointer
    std::vector<std::byte> contents(size);

    is.seekg(0);        // set pointer to beginning of stream
    is.read((char*)contents.data(), size);           // read data into contents vector

    if (contents.empty())
        return std::nullopt;
    return std::move(contents);
}

int main() {
    std::string location = "";
    std::string osm_data_file = "../data" + location + ".osm.pbf";

    std::vector<std::byte> osm_data;

    if(osm_data.empty() && !osm_data_file.empty()) {
        std::cout << "Reading OSP data from the following file: " <<  osm_data_file << std::endl;
        auto data = ReadFile(osm_data_file);

        if(!data){
            std::cout << "Failed to read." << std::endl;
        } else {
            osm_data = std::move(*data);          // move to other object without the need for a deep copy
        
        return 0;

        }
    }
}
