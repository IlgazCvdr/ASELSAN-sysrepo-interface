#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <sysrepo.h>
#include <libyang/libyang.h>
#include <arpa/inet.h>
#include <sysrepo/values.h>
#include <net/if.h>

sr_session_ctx_t *sess;
sr_subscription_ctx_t *subscription;


// Function prototypes
char *get_command_output(const char *command);

static int get_network_interfaces_cb(sr_session_ctx_t *session, uint32_t sub_id, const char *path, const sr_val_t *input,
                                     const size_t input_cnt, sr_event_t event, uint32_t request_id, sr_val_t **output, size_t *output_cnt,
                                     void *private_data)
{
    sr_val_t *output_val;

    (void)sub_id;
    (void)path;
    (void)input_cnt;
    (void)event;
    (void)request_id;
    (void)private_data;

    // Get the command output for network interfaces
    char *out = get_command_output("ip -o -4 addr show | awk '{print $2, $4}'");
    if (out == NULL)
    {
        return SR_ERR_INTERNAL;
    }

    // Allocate and populate the output structure
    *output_cnt = 1;
    *output = calloc(1, sizeof(**output));
    output_val = *output;

    output_val->xpath = strdup("/aselsan-network-settings:get-network-interfaces/interfaces");
    output_val->type = SR_STRING_T;
    output_val->data.string_val = out;

    return SR_ERR_OK;
}

char *get_command_output(const char *command)
{
    FILE *fp;
    char buffer[1024];        // Temporary buffer to hold the command output
    size_t output_size = 1024; // Initial buffer size for the final output
    size_t used_size = 0;     // Keeps track of the used size in the final buffer

    // Allocate memory for the final output buffer
    char *output = (char *)malloc(output_size);
    if (output == NULL)
    {
        perror("malloc");
        return NULL;
    }

    // Open the command for reading
    fp = popen(command, "r");
    if (fp == NULL)
    {
        perror("popen");
        free(output);
        return NULL;
    }

    // Read the output of the command
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        size_t len = strlen(buffer);
        // Check if the final buffer needs to be resized
        if (used_size + len + 1 > output_size)
        {
            output_size = used_size + len + 1; // Ensure enough space for new data and null terminator
            char *temp = (char *)realloc(output, output_size);
            if (temp == NULL)
            {
                perror("realloc");
                free(output);
                pclose(fp);
                return NULL;
            }
            output = temp;
        }
        // Append the buffer content to the final output
        memcpy(output + used_size, buffer, len);
        used_size += len;
    }

    // Null-terminate the final output buffer
    output[used_size] = '\0';

    // Close the pipe
    if (pclose(fp) == -1)
    {
        perror("pclose");
        free(output);
        return NULL;
    }

    return output;
}

// Function to create a network interface
static int create_network_interface_cb(sr_session_ctx_t *session, uint32_t sub_id, const char *path, const sr_val_t *input,
                                       const size_t input_cnt, sr_event_t event, uint32_t request_id, sr_val_t **output, size_t *output_cnt,
                                       void *private_data)
{
    (void)sub_id;
    (void)path;
    (void)input_cnt;
    (void)event;
    (void)request_id;
    (void)private_data;

    const char *interface_name = NULL;

    // Retrieve input values
    for (size_t i = 0; i < input_cnt; i++) {
        if (strcmp(input[i].xpath, "/aselsan-network-settings:create-network-interface/interface-name") == 0) {
            interface_name = input[i].data.string_val;
            break;
        }
    }

    if (!interface_name) {
        fprintf(stderr, "Error: Interface name not provided.\n");
        return SR_ERR_INVAL_ARG;
    }

    // Implement logic to create new network interface (example: using system command)
    char command[100];
    snprintf(command, sizeof(command), "sudo ip link add %s type dummy", interface_name);
    int ret = system(command);

    if (ret != 0) {
        fprintf(stderr, "Error: Failed to create network interface.\n");
        return SR_ERR_OPERATION_FAILED;
    }

    // Set up the interface
    snprintf(command, sizeof(command), "sudo ip link set %s up", interface_name);
    ret = system(command);

    if (ret != 0) {
        fprintf(stderr, "Error: Failed to set up network interface.\n");
        return SR_ERR_OPERATION_FAILED;
    }

    printf("Created and set up network interface: %s.\n", interface_name);

    *output_cnt = 0; // No output for this RPC
    return SR_ERR_OK;
}

// Function to set IP settings for a network interface
static int set_ip_settings_cb(sr_session_ctx_t *session, uint32_t sub_id, const char *path, const sr_val_t *input,
                              const size_t input_cnt, sr_event_t event, uint32_t request_id, sr_val_t **output, size_t *output_cnt,
                              void *private_data)
{
    (void)sub_id;
    (void)path;
    (void)event;
    (void)request_id;
    (void)private_data;

    const char *interface_name = NULL;
    const char *ip_address = NULL;

    // Retrieve input values
    printf("Input count: %zu\n", input_cnt);
    for (size_t i = 0; i < input_cnt; i++) {
        printf("Input %zu: xpath = %s, value = %s\n", i, input[i].xpath, input[i].data.string_val);
        if (strcmp(input[i].xpath, "/aselsan-network-settings:set-ip-settings/interface-name") == 0) {
            interface_name = input[i].data.string_val;
        } else if (strcmp(input[i].xpath, "/aselsan-network-settings:set-ip-settings/ip-address") == 0) {
            ip_address = input[i].data.string_val;
        }
    }

    if (!interface_name || !ip_address) {
        fprintf(stderr, "Error: Interface name or IP address not provided.\n");
        return SR_ERR_INVAL_ARG;
    }

    // Implement logic to set IP address (example: using system command)
    char command[200];
    snprintf(command, sizeof(command), "sudo ip addr add %s dev %s", ip_address, interface_name);
    int ret = system(command);

    if (ret != 0) {
        fprintf(stderr, "Error: Failed to set IP address.\n");
        return SR_ERR_OPERATION_FAILED;
    }

    printf("IP address %s configured for interface %s.\n", ip_address, interface_name);

    *output_cnt = 0; // No output for this RPC
    return SR_ERR_OK;
}

static int delete_ip_address_cb(sr_session_ctx_t *session, uint32_t sub_id, const char *path, const sr_val_t *input,
                                const size_t input_cnt, sr_event_t event, uint32_t request_id, sr_val_t **output, size_t *output_cnt,
                                void *private_data)
{
    (void)sub_id;
    (void)path;
    (void)event;
    (void)request_id;
    (void)private_data;

    const char *interface_name = NULL;
    const char *ip_address = NULL;

    // Retrieve input values
    for (size_t i = 0; i < input_cnt; i++) {
        if (strcmp(input[i].xpath, "/aselsan-network-settings:delete-ip-address/interface-name") == 0) {
            interface_name = input[i].data.string_val;
        } else if (strcmp(input[i].xpath, "/aselsan-network-settings:delete-ip-address/ip-address") == 0) {
            ip_address = input[i].data.string_val;
        }
    }

    if (!interface_name || !ip_address) {
        fprintf(stderr, "Error: Interface name or IP address not provided.\n");
        return SR_ERR_INVAL_ARG;
    }

    // Implement logic to delete IP address (example: using system command)
    char command[200];
    snprintf(command, sizeof(command), "sudo ip addr del %s dev %s", ip_address, interface_name);
    int ret = system(command);

    if (ret != 0) {
        fprintf(stderr, "Error: Failed to delete IP address.\n");
        return SR_ERR_OPERATION_FAILED;
    }

    printf("IP address %s deleted from interface %s.\n", ip_address, interface_name);

    *output_cnt = 0; // No output for this RPC
    return SR_ERR_OK;
}

// Initialization callback for the plugin
int sr_plugin_init_cb(sr_session_ctx_t *session, void **private_data)
{
    int rc;

    sess = session;

    // Subscribe to RPCs
    rc = sr_rpc_subscribe(session, "/aselsan-network-settings:create-network-interface", create_network_interface_cb, NULL, 0, 0, &subscription);
    if (rc != SR_ERR_OK) {
        fprintf(stderr, "Failed to subscribe to create-network-interface RPC: %s\n", sr_strerror(rc));
        return rc;
    }

    rc = sr_rpc_subscribe(session, "/aselsan-network-settings:set-ip-settings", set_ip_settings_cb, NULL, 0, 0, &subscription);
    if (rc != SR_ERR_OK) {
        fprintf(stderr, "Failed to subscribe to set-ip-settings RPC: %s\n", sr_strerror(rc));
        return rc;
    }

    // Subscribe to the new RPC
    rc = sr_rpc_subscribe(session, "/aselsan-network-settings:get-network-interfaces", get_network_interfaces_cb, NULL, 0, 0, &subscription);
    if (rc != SR_ERR_OK) {
        fprintf(stderr, "Failed to subscribe to get-network-interfaces RPC: %s\n", sr_strerror(rc));
        return rc;
    }

    rc = sr_rpc_subscribe(session, "/aselsan-network-settings:delete-ip-address", delete_ip_address_cb, NULL, 0, 0, &subscription);
    if (rc != SR_ERR_OK) {
        fprintf(stderr, "Failed to subscribe to delete-ip-address RPC: %s\n", sr_strerror(rc));
        return rc;
    }

    printf("Network settings plugin initialized successfully.\n");

    return SR_ERR_OK;
}


// Cleanup callback for the plugin
void sr_plugin_cleanup_cb(sr_session_ctx_t *session, void *private_data)
{
    (void)session;
    (void)private_data;

    sr_unsubscribe(subscription);
    printf("Network settings plugin cleanup finished.\n");
}
