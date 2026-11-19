#include "httplib.h"
#include <iostream>

std::string HOST = "0.0.0.0";

const int PORT = 3000;
int main() {
    httplib::Server s;

    s.Get("/", [](const auto& req, auto& res) {
        res.set_content("Hello, World!", "text/plain");
        std::cerr << "Wow we got a message";
    });

    s.listen(HOST, PORT);
    return 0;
}