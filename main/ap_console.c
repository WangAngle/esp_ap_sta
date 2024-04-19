#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <ctype.h>
#include <unistd.h>
#include "esp_sleep.h"
#include "esp_spi_flash.h"
#include "driver/rtc_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_wifi.h"
#include "ap_nvs.h"


static const char* TAG = "console";
#define PROMPT_STR " "

static ap_info_t ap_info_backup = {0};
static sta_info_t sta_info_backup = {0};

static struct {
    struct arg_str *mac;
    struct arg_str *ssid;
    struct arg_str *pass;
    struct arg_end *end;
} ap_info_cmd_args;

static uint8_t str2hex(const char *str, uint16_t strLen, uint8_t *out)
{
    uint8_t tmp;
    uint16_t i;

    for (i=0; i<strLen; ++i)
    {
        if (*str >= '0' && *str <= '9')
            tmp = *str - '0';
        else if (*str >= 'a' && *str <= 'f')
            tmp = *str - 'a' + 10;
        else if (*str >= 'A' && *str <= 'F')
            tmp = *str - 'A' + 10;
        else
            return -1;
        
        if (i & 0x01)
            out[i >> 1] = out[i >> 1] | (tmp & 0x0F);
        else
            out[i >> 1] = (tmp << 4) & 0xF0;
        str++;
    }
	return 0;
}

static int _set_ap_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &ap_info_cmd_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, ap_info_cmd_args.end, argv[0]);
        return -1;
    }

    if(ap_info_cmd_args.mac->count != 0)
    {
        const char *mac = ap_info_cmd_args.mac->sval[0];
        if(strlen(mac) == 12)
        {
            printf("ap mac:%s\r\n", mac);
            uint8_t mac_addr[6];
            str2hex(mac, 12, mac_addr);
            memcpy(ap_info_backup.mac, mac_addr, sizeof(ap_info_backup.mac));
        }
        else
        {
            printf("ap mac len err(must be 12)!\r\n");
        }
    }

    if(ap_info_cmd_args.ssid->count != 0)
    {
        const char *ssid = ap_info_cmd_args.ssid->sval[0];
        if(strlen(ssid) > 0)
        {
            printf("ap set ssid:%s\r\n", ssid);
            memcpy(ap_info_backup.ssid, ssid, sizeof(ap_info_backup.ssid));
        }
    }

    if(ap_info_cmd_args.pass->count != 0)
    {
        const char *pass = ap_info_cmd_args.pass->sval[0];
        if(strlen(pass) >= 8)
        {
            printf("ap set password:%s\r\n", pass);
            memcpy(ap_info_backup.pass, pass, sizeof(ap_info_backup.pass));
        }
        else
        {
            printf("ap password len less than 8!\r\n");
        }
    }

    set_ap_info(&ap_info_backup);

    printf("_set_ap_cmd end!!!\r\n\r\n");

    return 0;
}

static void register_set_ap_cmd(void)
{
    ap_info_cmd_args.mac = arg_str0("m", "mac", "<str>", "mac for soft ap");
    ap_info_cmd_args.ssid = arg_str0("s", "ssid", "<str>", "ssid for soft ap");
    ap_info_cmd_args.pass = arg_str0("p", "pass", "<str>", "password for soft ap");
    ap_info_cmd_args.end = arg_end(2);
    const esp_console_cmd_t cmd = {
        .command = "ap",
        .help = "set ap info",
        .hint = NULL,
        .func = &_set_ap_cmd,
        .argtable = NULL,
        // .argtable = &ap_info_cmd_args,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

static struct {
    struct arg_str *ssid;
    struct arg_str *pass;
    struct arg_end *end;
} sta_info_cmd_args;

static int _set_sta_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &sta_info_cmd_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, sta_info_cmd_args.end, argv[0]);
        return -1;
    }

    if(sta_info_cmd_args.ssid->count != 0)
    {
        const char *ssid = sta_info_cmd_args.ssid->sval[0];
        if(strlen(ssid) > 0)
        {
            printf("sta set ssid:%s\r\n", ssid);
            memcpy(sta_info_backup.ssid, ssid, sizeof(sta_info_backup.ssid));
        }
    }

    if(sta_info_cmd_args.pass->count != 0)
    {
        const char *pass = sta_info_cmd_args.pass->sval[0];
        if(strlen(pass) >= 8)
        {
            printf("sta set password:%s\r\n", pass);
            memcpy(sta_info_backup.pass, pass, sizeof(sta_info_backup.pass));
        }
        else
        {
            printf("sta password len less than 8!\r\n");
        }
    }

    set_sta_info(&sta_info_backup);

    printf("_set_sta_cmd end!!!\r\n\r\n");

    return 0;
}

static void register_set_sta_cmd(void)
{
    sta_info_cmd_args.ssid = arg_str0("s", "ssid", "<str>", "ssid for station");
    sta_info_cmd_args.pass = arg_str0("p", "pass", "<str>", "password for station");
    sta_info_cmd_args.end = arg_end(2);
    const esp_console_cmd_t cmd = {
        .command = "sta",
        .help = "set sta info",
        .hint = NULL,
        .func = &_set_sta_cmd,
        .argtable = NULL,
        // .argtable = &sta_info_cmd_args,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

static int restart(int argc, char **argv)
{
    ESP_LOGI(TAG, "Restarting");
    esp_restart();
}

static void register_restart(void)
{
    const esp_console_cmd_t cmd = {
        .command = "restart",
        .help = "Software reset of the chip",
        .hint = NULL,
        .func = &restart,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

//---------------------------------------------------------------------------------
static void initialize_console(void)
{
    fflush(stdout);
    fsync(fileno(stdout));
    setvbuf(stdin, NULL, _IONBF, 0);

    esp_vfs_dev_uart_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR);
    esp_vfs_dev_uart_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF);
    const uart_config_t uart_config = {
            .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
        .source_clk = UART_SCLK_REF_TICK,
#else
        .source_clk = UART_SCLK_XTAL,
#endif
    };
    ESP_ERROR_CHECK( uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM, 256, 0, 0, NULL, 0) );
    ESP_ERROR_CHECK( uart_param_config(CONFIG_ESP_CONSOLE_UART_NUM, &uart_config) );
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

    esp_console_config_t console_config = {
            .max_cmdline_args = 8,
            .max_cmdline_length = 256,
#if CONFIG_LOG_COLORS
            .hint_color = atoi(LOG_COLOR_CYAN)
#endif
    };
    ESP_ERROR_CHECK( esp_console_init(&console_config) );

    linenoiseSetMultiLine(1);
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);
    linenoiseAllowEmpty(false);
}

static void console_task(void *arg)
{
    initialize_console();

    get_ap_info(&ap_info_backup);  
    get_sta_info(&sta_info_backup);  

    esp_console_register_help_command();
    register_restart();

    ESP_LOGI(TAG, "register_set_ap_cmd");
    register_set_ap_cmd();
    register_set_sta_cmd();

    const char* prompt = LOG_COLOR_I PROMPT_STR " " LOG_RESET_COLOR;

    int probe_status = linenoiseProbe();
    if (probe_status) { /* zero indicates success */
        linenoiseSetDumbMode(1);
#if CONFIG_LOG_COLORS
        prompt = PROMPT_STR " ";
#endif //CONFIG_LOG_COLORS
    }

    while(true) {
        char* line = linenoise(prompt);
        if (line == NULL) { 
            continue;
        }

        if (strlen(line) > 0) {
            linenoiseHistoryAdd(line);
#if CONFIG_STORE_HISTORY
            /* Save command history to filesystem */
            linenoiseHistorySave(HISTORY_PATH);
#endif
        }

        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND) {
            printf("Unrecognized command\n");
        } else if (err == ESP_ERR_INVALID_ARG) {
            // command was empty
        } else if (err == ESP_OK && ret != ESP_OK) {
            printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(ret));
        } else if (err != ESP_OK) {
            printf("Internal error: %s\n", esp_err_to_name(err));
        }
        linenoiseFree(line);
    }

    esp_console_deinit();
}

void console_task_init(void)
{
    int ret = xTaskCreate(console_task, "console_task", 4096, NULL, 5, NULL);
    if (ret != pdTRUE)
    {
        printf("create console_task failed ret:%d\r\n", ret);
    }
}
