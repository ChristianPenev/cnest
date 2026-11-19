#include "httplib.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <map>

const std::string HOST = "0.0.0.0";

const int PORT = 3000;

struct User {
    std::string id;
    std::string name;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(User, id, name);

std::map<std::string, User> users = {{
    {"1", {"1", "Chris"}},
    {"2", {"2", "Toni"}},
    {"3", {"3", "Ani"}},
}};

int main() {
    httplib::Server s;

    s.Get("/", [](const auto&, httplib::Response& res) {
        res.set_content("Chris Server", "text/plain");
        std::cerr << "Wow we got a message\n";
    });

    s.Get("/users", [](const auto&, httplib::Response& res) {
        res.status = 200;
        nlohmann::json out = nlohmann::json::array();
        for (const auto& [id, user] : users) {
            out.push_back(user);
        }
        res.set_content(out.dump(), "application/json");
        std::cerr << "Returned the users!";
    });

    if (!s.listen(HOST, PORT)) {
        std::cerr << "[SERVER] Failed to start: The port is already in use " << PORT << "\n";
        return 1;
    }

    return 0;
}