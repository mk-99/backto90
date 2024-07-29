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

int append_message(
    HAREA out_area,
    NETADDR *src,
    NETADDR *dst,
    const char *from,
    const char *to,
    const char *subj,
    const char *body_text
)
{
    XMSG msg;
    HMSG out_msg;
    dword msg_len = 0;

    out_msg = MsgOpenMsg(out_area, MOPEN_CREATE, 0);
    if (!out_msg) {
        perror("Error writing to output area: ");
        MsgCloseApi();
        exit(ERR_MSG);
    }

    msg_len = (dword)strlen(body_text);

    memset(&msg, 0, sizeof(msg));
    msg.attr = 0;
    memmove(&msg.dest, dst, sizeof(msg.dest));
    memmove(&msg.orig, src, sizeof(msg.orig));
    // msg.date_arrived = 0;
    // msg.date_written = 0;
    strncpy((char *)msg.from, from, sizeof(msg.from) - 1);
    strncpy((char *)msg.to, to, sizeof(msg.to) - 1);
    strncpy((char *)msg.subj, subj, sizeof(msg.subj) - 1);

    MsgWriteMsg(out_msg, false, &msg, (unsigned char *)body_text, msg_len, msg_len * 2, 0, 0);

    MsgCloseMsg(out_msg);

    return 0;
}

int show_message(
    int number,
    NETADDR *src_addr,
    NETADDR *dst_addr,
    char *from_name,
    char *to_name,
    char *snt_datetime,
    char *rcv_datetime,
    char *subj,
    char *text
)
{
    printf("===== message number %d\n", number);

    printf("Sent %s\n", snt_datetime);
    printf("Received %s\n", rcv_datetime);

    printf("From %s ", from_name);
    printf("%d:%d/%d.%d\n", src_addr->zone, src_addr->net, src_addr->node, src_addr->point);

    printf("To %s ", to_name);
    printf("%d:%d/%d.%d\n", dst_addr->zone, dst_addr->net, dst_addr->node, dst_addr->point);

    printf("Subj %s\n", subj);
    printf("%s\n", text);

    printf("\n");

    return 0;
}

// Parse messages as json string, returns cJSON struct pointer, must be freed
cJSON *parse_msg_json(char *msg_str)
{
    cJSON *msg_json = cJSON_Parse(msg_str);
    if (msg_json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
            exit(ERR_JSON);
        }
        else {
            fprintf(stderr, "invalid format, empty array\n");
            exit(ERR_JSON);
        }
    }
    if (!cJSON_IsArray(msg_json)) {
            fprintf(stderr, "invalid format, not an array\n");
            exit(ERR_JSON);
    }

    return msg_json;
}

// Read file to string, dynamically allocated, must be freed
char *read_to_string(char *filename)
{
    size_t pos = 0, size = 8192, nread;
    char *buf0 = malloc(size);
    char *buf = buf0;

    FILE *input_file = NULL;

    if (filename == NULL) {
        input_file = stdin;
    }
    else if (*filename == '\0') {
        input_file = stdin;
    }
    else if ((strlen(filename) == 1) && (*filename == '-')) {
        input_file = stdin;
    }
    else {
        input_file = fopen(filename, "r");
    }

    if (input_file == NULL) {
        perror("Cannot open input file for reading");
        exit(1);
    }

    for (;;) {
        if (buf == NULL) {
            fprintf(stderr, "not enough memory for %zu bytes\n", size);
            free(buf0);
            exit(ERR_INPUT);
        }
        nread = fread(buf + pos, 1, size - pos - 1, input_file);
        if (nread == 0)
            break;
        pos += nread;
        /* Grow the buffer size exponentially (Fibonacci ratio) */
        if (size - pos < size / 2) {
            size += size / 2 + size / 8;
            buf0 = buf;
            buf = realloc(buf, size);
            if (buf == NULL) {
                fprintf(stderr, "not enough memory for %zu bytes\n", size);
                free(buf0);
                exit(ERR_INPUT);
            }
        }
    }
    buf[pos] = '\0';
    return buf;
}

// Check and acquire string from cJSON object
char *get_json_string(const cJSON * const current_item, const char *tag)
{
    cJSON *object = cJSON_GetObjectItem(current_item, tag);

    if (cJSON_IsString(object)) {
        return cJSON_GetStringValue(object);
    }
    else {
        fprintf(stderr, "get_json_string: invalid format, not a string");
        exit(ERR_JSON);
    }
}

// Check and acquire int from cJSON object
int get_json_int(const cJSON * const current_item, const char *tag)
{
    cJSON *object = cJSON_GetObjectItem(current_item, tag);

    if (cJSON_IsNumber(object)) {
        return (int)cJSON_GetNumberValue(object);
    }
    else {
        fprintf(stderr, "get_json_string: invalid format, not a number");
        exit(ERR_JSON);
    }
}

// Check and acquire int or null from cJSON object
int get_json_int_or_null(const cJSON * const current_item, const char *tag)
{
    cJSON *object = cJSON_GetObjectItem(current_item, tag);

    if (cJSON_IsNumber(object)) {
        return (int)cJSON_GetNumberValue(object);
    }
    else if (cJSON_IsNull(object)) {
        return 0;
    }
    else {
        fprintf(stderr, "get_json_string: invalid format, not a number");
        exit(ERR_JSON);
    }
}



// Check and acquire NETADDR from cJSON object, dynamically allocated, must be freed
NETADDR *get_json_netaddr(const cJSON * const current_item, const char *tag)
{
    NETADDR *addr;

    cJSON *addr_object = cJSON_GetObjectItem(current_item, tag);

    if (cJSON_IsObject(addr_object)) {
        addr = malloc(sizeof(NETADDR));
        if (addr == NULL) {
            fprintf(stderr, "cannot allocate memory for addr");
            exit(ERR_JSON);
        }
        addr->zone = get_json_int(addr_object, "zone");
        addr->net = get_json_int(addr_object, "net");;
        addr->node = get_json_int(addr_object, "node");;
        addr->point = get_json_int_or_null(addr_object, "point");;

        return addr;
    }
    else {
        fprintf(stderr, "get_json_string: invalid format, not a string");
        exit(ERR_JSON);
    }
}


int main(int argc, char *argv[])
{
    struct _minf msgapi_info;
    HAREA in_area;
    HMSG in_msg;

    // Set of messages
    char *msg_string;
    cJSON *msg_json, *current_message;
    
    // Message attributes
    NETADDR *src_addr, *dst_addr;
    char *from_name, *to_name, *snt_datetime, *rcv_datetime, *subj, *text;

    int num_messages, i;

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

        if (arguments.verbose) {
            printf("Message %d\n", i);
        }

        MsgCloseMsg(in_msg);

    }

    MsgUnlock(in_area);
    MsgCloseArea(in_area);
    MsgCloseApi();

    return 0;
 
}