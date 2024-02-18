// Create Squish message base from plain text

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "smapi/msgapi.h"

#define DEFAULT_ZONE 2

enum {
    ERR_AREA=1,
    ERR_MSG=2
};

char *body_text = "This is test message body";

char *area_name = "test";

int append_message(HAREA out_area, char *body_text)
{
    XMSG msg;
    HMSG out_msg;
    NETADDR src, dst;
    dword msg_len = 0;

    src.zone = 2;
    src.net = 5020;
    src.node = 99;
    src.point = 0;

    dst.zone = 2;
    dst.net = 5020;
    dst.node = 189;
    dst.point = 0;

    out_msg = MsgOpenMsg(out_area, MOPEN_CREATE, 0);
    if (!out_msg) {
        perror("Error writing to output area: ");
        MsgCloseApi();
        exit(ERR_MSG);
    }

    msg_len = (dword)strlen(body_text);

    memset(&msg, 0, sizeof(msg));
    msg.attr = 0;
    // msg.date_arrived = 0;
    // msg.date_written = 0;
    msg.dest = dst;
    strcpy(msg.from, "Max Klochkov");
    msg.orig = src;
    strcpy(msg.subj,"Test subject");
    strcpy(msg.to, "Nick G. Raysky");

    MsgWriteMsg(out_msg, false, &msg, body_text, msg_len, msg_len * 2, 0, 0);

    MsgCloseMsg(out_msg);

}


int main(int argc, char *argv[])
{
    struct _minf msgapi_info;
    HAREA out_area;

    memset(&msgapi_info, 0, sizeof(msgapi_info));
    msgapi_info.def_zone = DEFAULT_ZONE;

    MsgOpenApi(&msgapi_info);

    out_area = MsgOpenArea(area_name, MSGAREA_CREATE, MSGTYPE_SQUISH);

    if (!out_area) {
        perror("Cannot create area: ");
        exit(ERR_AREA);
    };

    MsgLock(out_area);

    append_message(out_area, "Test message 1");
    append_message(out_area, "Test message 2");

    MsgCloseArea(out_area);
    MsgCloseApi();

}