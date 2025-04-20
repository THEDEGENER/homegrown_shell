// simple http server

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <regex.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <format>

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(int client_fd);
void set_mime_type(const std::string &filename, std::string& mime_type);
void build_http_response(std::string& mime_type, std::string &response, std::string filename);

int listen_for_exit() {
    std::string line;
    std::getline(std::cin, line);

    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;
    while (iss >> token) {  // tokenizes by whitespace by default
        tokens.push_back(token);
    }

    if (tokens[0] == "exit") {
        exit(1);
    } else {
        std::cout << "unkown command" << std::endl;
        return 0;
    }
}

int main(int argc, char* argv[])
{
    struct sockaddr_in server_addr;

    // create server socket
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // sets configuration for the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // bind socket
    if (bind(server, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }


    // listen on port
    if (listen(server, 1) < 0) {
        perror("failed to listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1)
    {
        listen_for_exit();

        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int client_fd = accept(server, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_fd < 0) {
            perror("failed to accept");
            continue;
        }
        handle_client(client_fd);
    }
    close(server);

    return 0;
}

void handle_client(int client_fd)
{
    std::vector<char> buffer(BUFFER_SIZE);
    ssize_t bytes_received = recv(client_fd, buffer.data(), BUFFER_SIZE, 0);

    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';

        std::cout << buffer.data() << std::endl;

        regex_t regex;
        regcomp(&regex, "GET /([^ ]*) HTTP/1", REG_EXTENDED);
        // an array of size 2 where 0 is the whole string and 1 is the matched substring
        regmatch_t matches[2];

        if (regexec(&regex, buffer.data(), 2, matches, 0) == 0) {
            // Construct string from the match range
            std::string url_encoded_filename(buffer.data() + matches[1].rm_so,
                                             matches[1].rm_eo - matches[1].rm_so);
            std::cout << "Filename: " << url_encoded_filename << std::endl;

            std::string mime_type;
            std::string response;

            // build the http response
            set_mime_type(url_encoded_filename, mime_type);

            build_http_response(mime_type, response, url_encoded_filename);


            // send the response
            send(client_fd, response.c_str(), response.size(), 0);

        } else {
            std::cout << "Regex did not match the request." << std::endl;
        }
        regfree(&regex);
    }
    // send response

    close(client_fd);
    return;
}

void set_mime_type(const std::string &filename, std::string& mime_type) {
    std::size_t dot = filename.find(".");
    if (dot != std::string::npos) {
        std::string extension = filename.substr(dot);
        if (extension == ".html") {
            mime_type = "text/html";
        } else if (extension == ".css") {
            mime_type = "text/css";
        } else if (extension == ".txt") {
            mime_type = "text/plain";
        } else {
            // If there's no extension, you might want to return a default value
            mime_type = "text/plain";
        }
    } else {
        mime_type = "text/plain";
    }
}

void build_http_response(std::string& mime_type, std::string &response, std::string filename) {
    std::string header = std::format("HTTP/1.1 200 OK\r\n Content-Type: {} \r\n\r\n", mime_type);
    response += header;
    std::ifstream file(filename);
    if (!file) {
        std::cout << "Unable to open file" << std::endl;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    response += buffer.str();
}