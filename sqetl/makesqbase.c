// Create Squish message base from plain text

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "smapi/msgapi.h"

#define DEFAULT_ZONE 2

#define ERR_AREA 1

int main(int argc, char *argv[])
{
    struct _minf msgapi_info;
    HAREA out_area;

    memset(&msgapi_info, 0, sizeof(msgapi_info));
    msgapi_info.def_zone = DEFAULT_ZONE;

    MsgOpenApi(&msgapi_info);

    if (!(out_area = MsgOpenArea("test", MSGAREA_CREATE, MSGTYPE_SQUISH))) {
        perror("Cannot create area: ");
        exit(ERR_AREA);
    };

    MsgCloseArea(out_area);
    MsgCloseApi();

}