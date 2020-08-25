#include "handler.h"

#include <filesystem>

namespace {

std::string to_native_name(const std::string& path) {
    const auto fs_path = std::filesystem::path(path);
    const auto filename = dynalo::to_native_name(fs_path.filename().string());

    return (fs_path.parent_path() / filename).string();
}

template< typename T >
void load_function(dynalo::library& lib, std::function<T>& func, const char* name) {
    func = lib.get_function<T>(name);
}

}

Handler::Handler(const std::string& path) 
    : library_(to_native_name(path))
{
    load_function(library_, open, "asa_handler_open");
    load_function(library_, close, "asa_handler_close");
    load_function(library_, push, "asa_handler_push");
    load_function(library_, get_type, "asa_handler_get_type");
    load_function(library_, download, "asa_handler_download");
    load_function(library_, upload, "asa_handler_upload");
    load_function(library_, free, "asa_handler_free");
}
