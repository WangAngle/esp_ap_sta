
#include "ap_nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"

#define AP_INFO_NVS_NAME    "ap_info_name"
#define AP_INFO_KEY         "ap_info"
#define STA_INFO_KEY        "sta_info"

static const char *TAG = "ap_nvs";

int get_ap_info(ap_info_t *ap_info)
{
    esp_err_t err = ESP_OK;
    size_t required_size = 0;
    nvs_handle_t nv_handle;

    err = nvs_open(AP_INFO_NVS_NAME, NVS_READWRITE, &nv_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "NVS open operation failed");
        return err;
    }

    err = nvs_get_blob(nv_handle, AP_INFO_KEY, NULL, &required_size);
    if(err != ESP_OK)
    {
        ESP_LOGE(TAG, "NVS get blob operation failed");
        goto end;
    }

    err = nvs_get_blob(nv_handle, AP_INFO_KEY, ap_info, &required_size);

end:
    nvs_close(nv_handle);
    return err;
}

int set_ap_info(ap_info_t *ap_info)
{
    esp_err_t err = ESP_OK;
    size_t required_size = 0;
    nvs_handle_t nv_handle;

    err = nvs_open(AP_INFO_NVS_NAME, NVS_READWRITE, &nv_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "NVS open operation failed");
        return err;
    }

    required_size = sizeof(ap_info_t);
    err = nvs_set_blob(nv_handle, AP_INFO_KEY, ap_info, required_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS write operation failed !!");
        goto error;
    }

    /* NVS commit and close */
    err = nvs_commit(nv_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS commit operation failed !!");
        goto error;
    }

error:
    nvs_close(nv_handle);
    return err;
}

int get_sta_info(sta_info_t *sta_info)
{
    esp_err_t err = ESP_OK;
    size_t required_size = 0;
    nvs_handle_t nv_handle;

    err = nvs_open(AP_INFO_NVS_NAME, NVS_READWRITE, &nv_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "NVS open operation failed");
        return err;
    }

    err = nvs_get_blob(nv_handle, STA_INFO_KEY, NULL, &required_size);
    if(err != ESP_OK)
    {
        ESP_LOGE(TAG, "NVS get blob operation failed");
        goto end;
    }

    err = nvs_get_blob(nv_handle, STA_INFO_KEY, sta_info, &required_size);

end:
    nvs_close(nv_handle);
    return err;
}

int set_sta_info(sta_info_t *sta_info)
{
    esp_err_t err = ESP_OK;
    size_t required_size = 0;
    nvs_handle_t nv_handle;

    err = nvs_open(AP_INFO_NVS_NAME, NVS_READWRITE, &nv_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "NVS open operation failed");
        return err;
    }

    required_size = sizeof(sta_info_t);
    err = nvs_set_blob(nv_handle, STA_INFO_KEY, sta_info, required_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS write operation failed !!");
        goto error;
    }

    /* NVS commit and close */
    err = nvs_commit(nv_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS commit operation failed !!");
        goto error;
    }

error:
    nvs_close(nv_handle);
    return err;
}