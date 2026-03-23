#pragma once

#include <functional>
#include <memory>
#include <string_view>
#include <vector>

class Application;

struct AppDescriptor {
    std::string_view                             name;
    std::string_view                             description;
    std::function<std::unique_ptr<Application>()> create;
};

const std::vector<AppDescriptor>& getAppRegistry();
std::unique_ptr<Application>      createAppByName(std::string_view name);
