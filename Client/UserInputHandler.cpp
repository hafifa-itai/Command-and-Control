#include "UserInputHandler.hpp"

ControllerCommandReq UserInputHandler::CreateCommandObject()
{
    std::string param;
    std::string input;
    std::string command;
    std::vector<std::string> parameters;

    while (TRUE) {

        std::cout << "Enter Full command: ";
        std::getline(std::cin, input);

        std::istringstream iss(input);
        iss >> command;

        while (iss >> param) {
            parameters.push_back(param);
        }

        for (const auto& p : parameters) {
            std::cout << "- " << p << "\n";
        }

        if (command == "quit") {
            return ControllerCommandReq(CommandType::Quit, "", "", "");
        }
        else if (command == "close") {
            if (parameters.size() != 1) {
                std::cout << "[!] Invalid parameters for close command\n";
            }
            else {
                return ControllerCommandReq(CommandType::Close, parameters[0], "", "");
            }
        }
        else if (command == "cmd") {
            if (parameters.size() != 1) {
                std::cout << "[!] Invalid parameters for cmd command\n";
            }
            else {
                return ControllerCommandReq(CommandType::OpenCmdWindow, parameters[0], "", "");
            }
        }
        else if (command == "list") {
            return ControllerCommandReq(CommandType::List, "", "", "");
        }
        else if (command == "group-cmd") {
            if (parameters.size() > 1) {
                //return UserRunCommandOnGroup(parameters);
            }
            else {
                std::cout << "[!] Invalid parameters for group-cmd command\n";
            }
        }
        else if (command == "group-create") {
            if (parameters.size() == 1) {
                return ControllerCommandReq(CommandType::GroupCreate, "", parameters[0], "");
            }
            else {
                std::cout << "[!] Invalid parameters for group-create command\n";
            }
        }
        else if (command == "group-add") {
            if (parameters.size() == 2) {
                return ControllerCommandReq(CommandType::GroupAdd, parameters[1], parameters[0], "");
            }
            else {
                std::cout << "[!] Invalid parameters for group-add command\n";
            }
        }
        else if (command == "group-list") {
            if (parameters.size() == 1) {
                return ControllerCommandReq(CommandType::ListGroup, "", parameters[0], "");
            }
            else {
                std::cout << "[!] Invalid parameters for group-list command\n";
            }
        }
        else if (command == "group-delete") {
            if (parameters.size() == 1) {
                return ControllerCommandReq(CommandType::GroupDelete, "", parameters[0], "");
            }
            else {
                std::cout << "[!] Invalid parameters for group-delete command\n";
            }
        }
        else if (command == "group-remove") {
            if (parameters.size() == 2) {
                return ControllerCommandReq(CommandType::GroupRemove, parameters[1], parameters[0], "");
            }
            else {
                std::cout << "[!] Invalid parameters for group-remove command\n";
            }
        }
        else if (command == "man") {
             return ControllerCommandReq(CommandType::Man, "", "", "");
        }
        else if (command == "groups") {
            return ControllerCommandReq(CommandType::ListGroupNames, "", "", "");
        }
        else if (command != "") {
            return ControllerCommandReq(CommandType::Unknown, "", "", "");
        }

        return ControllerCommandReq(CommandType::SyntaxError, "", "", "");
    }
}

