// Create Squish message base from plain text

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "smapi/msgapi.h"

#include "cjson/cJSON.h"

#define DEFAULT_ZONE 2
#define BUFSIZE 4096

enum {
    ERR_AREA=1,
    ERR_MSG=2
};

char *body_text = "This is test message body";

char *area_name = "test";

int append_message(
    HAREA out_area,
    NETADDR *src,
    NETADDR *dst,
    char *from,
    char *to,
    char *subj,
    char *body_text
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
    strncpy(msg.from, from, sizeof(msg.from) - 1);
    strncpy(msg.to, to, sizeof(msg.to) - 1);
    strncpy(msg.subj, subj, sizeof(msg.subj) - 1);

    MsgWriteMsg(out_msg, false, &msg, body_text, msg_len, msg_len * 2, 0, 0);

    MsgCloseMsg(out_msg);

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
            exit(1);
        }
        else {
            fprintf(stderr, "invalid format, empty array\n");
            exit(1);
        }
    }
    if (!cJSON_IsArray(msg_json)) {
            fprintf(stderr, "invalid format, not an array\n");
            exit(1);
    }

    return msg_json;
}

// Read stdin to string, dynamically allocated, must be freed
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
    else if (strncmp(filename, "-", 2) == 0) {
        input_file = stdin;
    }
    else {
        input_file = fopen(filename, "r");
    }

    if (input_file == NULL) {
        fprintf(stderr, "cannot open input file for reading\n");
        exit(1);
    }

    for (;;) {
        if (buf == NULL) {
            fprintf(stderr, "not enough memory for %zu bytes\n", size);
            free(buf0);
            exit(1);
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
                exit(1);
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
        exit(1);
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
        exit(1);
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
        exit(1);
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
            exit(1);
        }
        addr->zone = get_json_int(addr_object, "zone");
        addr->net = get_json_int(addr_object, "net");;
        addr->node = get_json_int(addr_object, "node");;
        addr->point = get_json_int_or_null(addr_object, "point");;

        return addr;
    }
    else {
        fprintf(stderr, "get_json_string: invalid format, not a string");
        exit(1);
    }
}


int main(int argc, char *argv[])
{
    struct _minf msgapi_info;
    HAREA out_area;

    // Set of messages
    char *msg_string;
    cJSON *msg_json, *current_message;
    
    // Message attributes
    NETADDR *src_addr, *dst_addr;
    char *from_name, *to_name, *snt_datetime, *rcv_datetime, *subj, *text;

    int num_messages, i;

    msg_string = read_to_string(argv[1]);
    msg_json = parse_msg_json(msg_string);

    num_messages = cJSON_GetArraySize(msg_json);


    // Initialize msgapi and create message base
    memset(&msgapi_info, 0, sizeof(msgapi_info));
    msgapi_info.def_zone = DEFAULT_ZONE;
    MsgOpenApi(&msgapi_info);
    out_area = MsgOpenArea(area_name, MSGAREA_CREATE, MSGTYPE_SQUISH);
    if (!out_area) {
        perror("Cannot create area: ");
        exit(ERR_AREA);
    };

    MsgLock(out_area);


    for(i = 0; i < num_messages; i++) {
        current_message = cJSON_GetArrayItem(msg_json, i);
        if (!cJSON_IsObject(current_message)) {
            fprintf(stderr, "invalid format, not an object\n");
            exit(1);
        }

        src_addr = get_json_netaddr(current_message, "src");
        dst_addr = get_json_netaddr(current_message, "dst");

        snt_datetime = get_json_string(current_message, "snt_datetime");
        rcv_datetime = get_json_string(current_message, "rcv_datetime");
        from_name = get_json_string(current_message, "from_name");
        to_name = get_json_string(current_message, "to_name");
        subj = get_json_string(current_message, "subj");
        text = get_json_string(current_message, "text");

        printf("==========================\n");
        printf("Message number %d\n", i);

        printf("Sent %s\n", snt_datetime);
        printf("Received %s\n", rcv_datetime);

        printf("From %s\n", from_name);
        printf("Source address %d:%d/%d.%d\n", src_addr->zone, src_addr->net, src_addr->node, src_addr->point);

        printf("To %s\n", to_name);
        printf("Destination address %d:%d/%d.%d\n", dst_addr->zone, dst_addr->net, dst_addr->node, dst_addr->point);

        printf("Subj %s\n", subj);
        printf("Text %s\n", text);

        printf("\n\n\n\n");

        printf("Writing to database.....\n");
        append_message(out_area, src_addr, dst_addr, from_name, to_name, subj, text);
    }

    MsgCloseArea(out_area);
    MsgCloseApi();

    return 0;
 
}