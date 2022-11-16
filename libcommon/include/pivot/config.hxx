#include <filesystem>
#include <stdexcept>

#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/filesystem.hpp>

namespace pivot
{

class Config
{
public:
    static std::filesystem::path find_data_folder(std::string folder)
    {
        namespace fs = std::filesystem;

        fs::path PIVOT_INSTALLED_DATAROOT = "/usr/share/pivot";
        fs::path dataroot_directory = PIVOT_INSTALLED_DATAROOT / folder;

        if (fs::exists(dataroot_directory) && fs::is_directory(dataroot_directory)) { return dataroot_directory; }

        auto program_path = fs::path(boost::dll::program_location().string());

        auto search_directory = program_path.parent_path();
        while (search_directory.parent_path() != search_directory) {
            auto folder_path = search_directory / folder;
            if (fs::exists(folder_path) && fs::is_directory(folder_path)) { return folder_path; }
            search_directory = search_directory.parent_path();
        }
        throw std::runtime_error(std::string("Could not find data folder " + folder));
    }

    static std::filesystem::path find_assets_folder() { return find_data_folder("assets"); }
    static std::filesystem::path find_shaders_folder() { return find_data_folder("shaders"); }
};
}    // namespace pivot