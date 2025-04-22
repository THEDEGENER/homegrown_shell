// simple http server

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <format>
#include <filesystem>
#include <regex>

#include "http.hpp"

#define PORT 8080
#define BUFFER_SIZE 1024

int print_asset(const std::string& filename)
{   
    const std::string default_extension = ".txt";
    const std::string current_dir = std::filesystem::current_path();
    const std::string asset_path = current_dir + "/assets/";
    std::fstream file(asset_path + filename + default_extension);
    if (!file) 
    {
        std::cout << "unable to open file" << std::endl;
        return 1;
    }
    std::string line;
    while(std::getline(file, line))
    {
        std::cout << line << std::endl;
    }

    file.close();
    return 0;
}

// if http server commands grow this can be changed to encapsulate all commands
int listen_for_command() {
    std::string line;
    std::getline(std::cin, line);

    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;
    while (iss >> token) {  // tokenizes by whitespace by default
        tokens.push_back(token);
    }

    std::cout << ">> ";

    if (tokens[0] == "exit-server") 
    {
        std::cout << "Stopping HTTP server" << std::endl;
        return 1;
    } 
    else if (tokens[0] == "help") 
    {
        print_asset("http-help");
        return 0;
    } 
    else 
    {
        std::cout << "unkown command - use exit server to stop" << std::endl;
        return 0;
    }
}

void set_mime_type(const std::string &filename, std::string& mime_type) 
{
    std::size_t dot = filename.find(".");
    if (dot != std::string::npos) 
    {
        std::string extension = filename.substr(dot);
        if (extension == ".html") 
        {
            mime_type = "text/html";
        } 
        else if (extension == ".css") 
        {
            mime_type = "text/css";
        } 
        else if (extension == ".txt") 
        {
            mime_type = "text/plain";
        } 
        else 
        {
            // If there's no extension set a default value
            mime_type = "text/plain";
        }
    } 
    else 
    {
        mime_type = "text/plain";
    }
}

void build_http_response(std::string& mime_type, std::string &response, std::string filename) 
{
    std::vector<std::string> headers{
        "HTTP/1.1 200 OK\r\n",
        "Content-Type: " + mime_type + "\r\n",
        "\r\n"
    };
    
    std::ifstream file(filename);

    if (!file) 
    {
        std::cout << "Unable to open file" << std::endl;
        response = "HTTP/1.1 404 Not Found\r\n Content-Type: text/plain\r\n\r\n 404 Not Found";
        return;
    }

    for (auto line : headers)
    {
        response += line;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    response += buffer.str();
}

void handle_client(int &client_fd)
{
    // std::vector<char> buffer(BUFFER_SIZE);
    std::string buffer(BUFFER_SIZE, '\0');
    ssize_t bytes_received = recv(client_fd, buffer.data(), BUFFER_SIZE, 0);

    if (bytes_received > 0) 
    {
        buffer.resize(bytes_received);

        std::cout << buffer << std::endl;

        std::smatch mtch;
        std::regex expr(
            R"(^\s*(GET|POST)\s+/([^ ]*)\s+HTTP/1\.[01])"
        );

        if (std::regex_search (buffer, mtch, expr))
        {
            // Construct string from the match range
            std::string url_encoded_filename(mtch[2].str());
            std::cout << "Filename: " << url_encoded_filename << std::endl;

            std::string mime_type;
            std::string response;

            // build the http response
            set_mime_type(url_encoded_filename, mime_type);

            // have a more robust console log for request and responses 
            // have more information for the start up of the server
            // have better error handeling 

            build_http_response(mime_type, response, url_encoded_filename);

            // send the response
            send(client_fd, response.c_str(), response.size(), 0);

        } 
        else 
        {
            std::cout << "Regex did not match the request." << std::endl;
        }
    }

    close(client_fd);
    return;
}

int start_http_server(const std::vector<std::string>& args)
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
    if (bind(server, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }


    // listen on port
    if (listen(server, 1) < 0) 
    {
        perror("failed to listen");
        exit(EXIT_FAILURE);
    }

    std::cout << ">> Server listening on port: " << PORT << std::endl;

    while (1)
    {
        std::cout << ">> ";
        //if (listen_for_command() == 1) 
        //{
        //    break;
        //} 
        std::cout << "ready to accept" << std::endl;

        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int client_fd = accept(server, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_fd < 0) 
        {
            perror("failed to accept");
            continue;
        }
        else 
        {
            std::cout << "accepted" << std::endl;
            handle_client(client_fd);
        }
    }
    close(server);
    return 1;
}

