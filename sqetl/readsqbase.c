// Read Squish message base to json

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <argp.h>

#include "smapi/msgapi.h"

#include "cjson/cJSON.h"

#define DEFAULT_ZONE 2
#define BUFSIZE 4096

enum {
    ERR_AREA=1,
    ERR_MSG=2,
    ERR_JSON=3,
    ERR_INPUT=4
};

const char *argp_program_version = "readsqbase 0.1.0";
static char doc[] = "reads squish message base to json";
static char args_doc[] = "inarea";

// cli argument availble options.
static struct argp_option options[] = {
    {"verbose", 'v', 0, 0, "Produce verbose output"},
    {"outfile", 'o', "outfile", 0, "output json file, stdout if ommited"},
    {0}
};


// define a struct to hold the arguments.
struct arguments {
    int  verbose;
    char *args[1];
    char *outfile;
};


// define a function which will parse the args.
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;
    switch(key) {
        case 'v':
            arguments->verbose = 1;
            break;

        case 'o':
            arguments->outfile = arg;
            break;

        case ARGP_KEY_ARG:        
            if(state->arg_num > 1) // Too many arguments
                argp_usage(state);
            arguments->args[state->arg_num] = arg;
            break;

        case ARGP_KEY_END:
            if(state->arg_num < 1) // Not enough arguments
                argp_usage(state);
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}


// initialize the argp struct. Which will be used to parse and use the args.
static struct argp argp = {options, parse_opt, args_doc, doc};

// Convert message to json
int convert_message(HMSG hmsg, XMSG *msg, dword text_len, char *text, dword ctrl_len, char *ctrl)
{
    dword i;

    printf("Src address: %d:%d/%d.%d\n", msg->orig.zone, msg->orig.net, msg->orig.node, msg->orig.point);
    printf("Dst address: %d:%d/%d.%d\n", msg->dest.zone, msg->dest.net, msg->dest.node, msg->dest.point);
    printf("From: %s\n", msg->from);
    printf("To: %s\n", msg->to);
    printf("Subj: %s\n", msg->subj);

    printf("Body: ");
    for (i = 0; i < text_len; i++) {
        if (text[i] == 0) {
            break;
        }
        putchar(text[i]);
    }

    printf("\n");

    printf("Control info: ");
    for (i = 0; i < ctrl_len; i++) {
        if (ctrl[i] == 0)
            break;
        putchar(ctrl[i]);
    }

    printf("\n");

    return 0;
}


int main(int argc, char *argv[])
{
    struct _minf msgapi_info;
    HAREA in_area;
    HMSG in_msg;
    XMSG msg;

    // Set of messages
    cJSON *msg_json, *current_message;
    
    // Message attributes
    NETADDR *src_addr, *dst_addr;
    char *from_name, *to_name, *snt_datetime, *rcv_datetime, *subj, *text, *ctrl;

    int num_messages, i;

    dword ctrl_len, text_len, bytes_read;

    // Parse command line arguments
    struct arguments arguments;
    arguments.verbose = 0;
    arguments.outfile = "";
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    // Initialize msgapi and create message base
    memset(&msgapi_info, 0, sizeof(msgapi_info));
    msgapi_info.def_zone = DEFAULT_ZONE;
    MsgOpenApi(&msgapi_info);
    in_area = MsgOpenArea((unsigned char *)arguments.args[0], MSGAREA_NORMAL, MSGTYPE_SQUISH);
    if (!in_area) {
        perror("Cannot open area: ");
        exit(ERR_AREA);
    };
    MsgLock(in_area);
    num_messages = MsgHighMsg(in_area);

    for(i = 1; i <= num_messages; i++ ) {
        in_msg = MsgOpenMsg(in_area, MOPEN_READ, i);

        ctrl_len = MsgGetCtrlLen(in_msg);
        text_len = MsgGetTextLen(in_msg);

        if ((ctrl = malloc(ctrl_len)) == NULL)
            ctrl_len = 0;

        if ((text = malloc(text_len)) == NULL)
            text_len = 0;

        bytes_read = MsgReadMsg(in_msg, &msg, 0, text_len, text, ctrl_len, ctrl);

        if (arguments.verbose)
            printf("Message %d, bytes read %d, control read %d\n", i, bytes_read, ctrl_len);

        convert_message(in_msg, &msg, text_len, text, ctrl_len, ctrl);

        if (text)
            free(text);

        if (ctrl)
           free(ctrl);

        MsgCloseMsg(in_msg);
    }

    MsgUnlock(in_area);
    MsgCloseArea(in_area);
    MsgCloseApi();

    return 0;
 
}