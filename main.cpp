#include "httplib.h"
#include "nlohmann/json.hpp"
#include "uuid.h"
#include <iostream>
#include <map>
#include <stdexcept>

const std::string HOST = "0.0.0.0";

const int PORT = 3000;

struct User {
    std::string id;
    std::string name;

    void print() {
        std::cerr << "User: {id: " << id << ", " << "name: " << name << "}\n";
    }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(User, id, name);

std::map<std::string, User> users = {{
    {"1", {"1", "Chris"}},
    {"2", {"2", "Toni"}},
    {"3", {"3", "Ani"}},
}};

User parseUser(const nlohmann::json& json) {
    const std::string name = json.at("name");

    if (name.empty()) {
        throw std::invalid_argument("name should not be empty");
    }
    const std::string id = uuidv7String();

    return {id, name};
}

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

    s.Post("/users", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto json = nlohmann::json::parse(req.body);

            User user = parseUser(json);
            user.print();
            users.insert({user.id, user});

            res.status = 201;
            res.set_content("Chris Server", "text/plain");
        } catch (nlohmann::json::parse_error& e) {
            std::cerr << "Everything was wrong!";

            res.status = 400;
            std::cerr << "parse error at byte " << e.byte << std::endl;
        }
    });

    s.Put("/users/:id", [](const httplib::Request& req, httplib::Response& res) {
        const auto json = nlohmann::json::parse(req.body);

        const auto name = json.at("name").get<std::string>();
        const auto& id = req.path_params.at("id");

        if (id.empty() || name.empty()) {
            throw std::invalid_argument("name should not be empty");
        }

        const auto it = users.find(id);

        if (it == users.end()) {
            res.status = 404;
            res.set_content(R"({"message":"User not found"})", "application/json");
            std::cerr << "User " << id << " not found\n";
            return;
        }

        it->second.name = name;
        std::cerr << "User updated successfully!";

        nlohmann::json resJson = it->second;

        res.status = 200;
        res.set_content(resJson.dump(), "application/json");
    });

    s.Delete("/users/:id", [](const httplib::Request& req, httplib::Response& res) {
        const auto& id = req.path_params.at("id");

        std::cerr << "The id is: " << id;

        const auto it = users.find(id);
        if (it == users.end()) {
            res.status = 404;
            res.set_content(R"({"message":"User not found"})", "application/json");
            std::cerr << "User " << id << " not found\n";
            return;
        }

        users.erase(id);
        res.status = 200;
        res.set_content("User got deleted", "text/plain");
    });

    if (!s.listen(HOST, PORT)) {
        std::cerr << "[SERVER] Failed to start: The port is already in use " << PORT << "\n";
        return 1;
    }

    return 0;
}