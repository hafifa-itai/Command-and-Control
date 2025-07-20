#include "UserInputHandler.hpp"

ControllerCommandReq UserInputHandler::CreateCommandObject()
{
    std::string param;
    std::string input;
    std::string szCommand;
    std::vector<std::string> parameters;

    while (TRUE) {

        std::cout << "Enter Full command: ";
        std::getline(std::cin, input);

        std::istringstream iss(input);
        iss >> szCommand;

        while (iss >> param) {
            parameters.push_back(param);
        }

        for (const auto& p : parameters) {
            std::cout << "- " << p << "\n";
        }

        if (szCommand == "quit") {
            return ControllerCommandReq(CommandType::Quit, "", "", "");
        }
        else if (szCommand == "close") {
            if (parameters.size() != 1) {
                std::cout << "[!] Invalid parameters for close command\n";
            }
            else {
                return ControllerCommandReq(CommandType::Close, parameters[0], "", "");
            }
        }
        else if (szCommand == "cmd") {
            if (parameters.size() != 1) {
                std::cout << "[!] Invalid parameters for cmd command\n";
            }
            else {
                return ControllerCommandReq(CommandType::OpenCmdWindow, parameters[0], "", "");
            }
        }
        else if (szCommand == "list") {
            return ControllerCommandReq(CommandType::List, "", "", "");
        }
        else if (szCommand == "group-cmd") {
            if (parameters.size() == 1) {
                return ControllerCommandReq(CommandType::OpenCmdWindow, "", parameters[0], "");
            }
            else {
                std::cout << "[!] Invalid parameters for group-cmd command\n";
            }
        }
        else if (szCommand == "group-create") {
            if (parameters.size() == 1) {
                return ControllerCommandReq(CommandType::GroupCreate, "", parameters[0], "");
            }
            else {
                std::cout << "[!] Invalid parameters for group-create command\n";
            }
        }
        else if (szCommand == "group-add") {
            if (parameters.size() == 2) {
                return ControllerCommandReq(CommandType::GroupAdd, parameters[1], parameters[0], "");
            }
            else {
                std::cout << "[!] Invalid parameters for group-add command\n";
            }
        }
        else if (szCommand == "group-list") {
            if (parameters.size() == 1) {
                return ControllerCommandReq(CommandType::ListGroup, "", parameters[0], "");
            }
            else {
                std::cout << "[!] Invalid parameters for group-list command\n";
            }
        }
        else if (szCommand == "group-delete") {
            if (parameters.size() == 1) {
                return ControllerCommandReq(CommandType::GroupDelete, "", parameters[0], "");
            }
            else {
                std::cout << "[!] Invalid parameters for group-delete command\n";
            }
        }
        else if (szCommand == "group-remove") {
            if (parameters.size() == 2) {
                return ControllerCommandReq(CommandType::GroupRemove, parameters[1], parameters[0], "");
            }
            else {
                std::cout << "[!] Invalid parameters for group-remove command\n";
            }
        }
        else if (szCommand == "man") {
             return ControllerCommandReq(CommandType::Man, "", "", "");
        }
        else if (szCommand == "groups") {
            return ControllerCommandReq(CommandType::ListGroupNames, "", "", "");
        }
        else if (szCommand != "") {
            return ControllerCommandReq(CommandType::Unknown, "", "", "");
        }

        return ControllerCommandReq(CommandType::SyntaxError, "", "", "");
    }
}

