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

    s.Get("/", [](const auto&, auto& res) {
        res.set_content("Chris Server", "text/plain");
        std::cerr << "Wow we got a message\n";
    });

    s.Get("/users", [](const auto&, auto& res) {
        res.status = 200;

        nlohmann::json resJson = nlohmann::json::array();
        for (const auto& [id, user] : users) {
            resJson.push_back(user);
        }

        res.set_content(resJson.dump(), "application/json");
        std::cerr << "Returned the users!";
    });

    s.Get("/users/:id", [](const httplib::Request& req, httplib::Response& res) {
        const auto& id = req.path_params.at("id");

        const auto it = users.find(id);
        if (it == users.end()) {
            res.status = 404;
            res.set_content(R"({"message":"User not found"})", "application/json");
            std::cerr << "User " << id << " not found\n";
            return;
        }

        nlohmann::json resJson = it->second;
        res.set_content(resJson.dump(), "application/json");
    });

    if (!s.listen(HOST, PORT)) {
        std::cerr << "[SERVER] Failed to start: The port is already in use " << PORT << "\n";
        return 1;
    }

    return 0;
}