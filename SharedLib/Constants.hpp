#pragma once

#define MAX_BUFFER_SIZE 4096
#define CONTROLLER_PORT 3000
#define AGENT_PORT 3001
#define SERVER_IP "192.168.20.5"
#define NO_GROUP ""
#define CONTROLLER_INDEX 0
#define AGENT_INDEX 1
#define GET_CWD ";$PWD.Path;"
#define VERBOSE_JSON 4
#define NOP_COMMAND L"$null = $null"
#define NOP_COMMAND_SIZE 26
#define QUIT_COMMAND L"quit"
#define EXIT_COMMAND L"exit"
#define MAX_MSG_SIZE 20 * 1024 * 1024 //20MB


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
    NewLine
};


const std::unordered_map<std::wstring, CommandType> StringToCommandTypeMap = {
    {L"quit",         CommandType::Quit},
    {L"close",        CommandType::Close},
    {L"cmd",          CommandType::Execute},
    {L"group-cmd",    CommandType::GroupExecute},
    {L"list",         CommandType::List},
    {L"group-create", CommandType::GroupCreate},
    {L"group-delete", CommandType::GroupDelete},
    {L"group-add",    CommandType::GroupAdd},
    {L"group-remove", CommandType::GroupRemove},
    {L"group-list",   CommandType::ListGroup},
    {L"groups",       CommandType::ListGroupNames},
    {L"man",          CommandType::Man},
    {L"",             CommandType::NewLine}
};