#pragma once

#define MAX_BUFFER_SIZE 4096
#define CONTROLLER_PORT 3000
#define AGENT_PORT 3001
#define NO_GROUP ""
#define CONTROLLER_INDEX 0
#define AGENT_INDEX 1
#define GET_CWD ";$PWD.Path;"
#define VERBOSE_JSON 4


enum class CommandType : int {
    Unknown,
    Quit,
    Close,
    Execute,
    GroupExecute,
    OpenCmdWindow,
    List,
    GroupCreate,
    GroupDelete,
    GroupAdd,
    GroupRemove,
    ListGroup,
    ListGroupNames,
    Man,
    SyntaxError
};


const std::unordered_map<std::string, CommandType> StringToCommandTypeMap = {
    {"quit",         CommandType::Quit},
    {"close",        CommandType::Close},
    {"cmd",          CommandType::Execute},
    {"group-cmd",    CommandType::GroupExecute},
    {"list",         CommandType::List},
    {"group-create", CommandType::GroupCreate},
    {"group-delete", CommandType::GroupDelete},
    {"group-add",    CommandType::GroupAdd},
    {"group-remove", CommandType::GroupRemove},
    {"group-list",   CommandType::ListGroup},
    {"groups",       CommandType::ListGroupNames},
    {"man",          CommandType::Man}
};