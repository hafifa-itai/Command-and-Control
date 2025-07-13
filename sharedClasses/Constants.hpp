#pragma once

#include "Libraries.hpp"

#define MAX_BUFFER_SIZE 4096
#define CONTROLLER_PORT 3000
#define AGENT_PORT 3001
#define NO_GROUP ""
#define CONTROLLER_INDEX 0
#define AGENT_INDEX 1

enum class CommandType : INT {
    Unknown,
    Quit,
    Close,
    Execute,
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