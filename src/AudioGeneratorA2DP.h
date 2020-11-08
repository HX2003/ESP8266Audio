#ifndef _AUDIOGENERATORA2DP_H
#define _AUDIOGENERATORA2DP_H

#include "AudioGenerator.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "esp_system.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"

#ifdef ARDUINO_ARCH_ESP32
#include "esp32-hal-log.h"
#include "esp32-hal-bt.h"
#endif

#define APP_CORE_TAG  "BT_APP_CORE"
#define APP_SIG_WORK_DISPATCH (0x01)

/**
 * @brief     handler for the dispatched work
 */
typedef void (* app_callback_t) (uint16_t event, void *param);


/* message to be sent */
typedef struct {
    uint16_t             sig;      /*!< signal to app_task */
    uint16_t             event;    /*!< message event id */
    app_callback_t          cb;       /*!< context switch callback */
    void                 *param;   /*!< parameter area needs to be last */
} app_msg_t;

/* event for handler "bt_av_hdl_stack_up */
enum {
    BT_APP_EVT_STACK_UP = 0,
};

class AudioGeneratorA2DP : public AudioGenerator
{
  public:
    AudioGeneratorA2DP(const  char* name = "ESP Bluetooth Speaker");
    virtual ~AudioGeneratorA2DP() override;
    virtual bool begin(AudioFileSource *source, AudioOutput *output) override;
    virtual bool loop() override;
    virtual bool stop() override;
    virtual bool isRunning() override;
	
	void start(char* name);
    esp_a2d_audio_state_t get_audio_state();
    esp_a2d_mct_t get_audio_type();
    void set_on_connection_changed(void (*callBack)());

    /**
     * Wrappbed methods called from callbacks
     */
    void app_a2d_callback(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param);
    void app_rc_ct_callback(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param);
    void app_task_handler();
    // Callback for music stream 
    void audio_data_callback(const uint8_t *data, uint32_t len);
    // av event handler
    void av_hdl_stack_evt(uint16_t event, void *p_param);
    // a2dp event handler 
    void av_hdl_a2d_evt(uint16_t event, void *p_param);
    // avrc event handler 
    void av_hdl_avrc_evt(uint16_t event, void *p_param);
	
  protected:

    // Input buffering
    const uint16_t ringBuffSize = 16384;
	const uint16_t buffSize = 256;
	uint8_t *buff;
	uint16_t buffPtr = 0;
	uint16_t availBytes = 0;
	bool empty = true;
    bool GetOneSample(int16_t sample[2]);
    uint32_t lastRate = 44100;
	uint32_t underflow_cnt = 0;
    xQueueHandle app_task_queue;
    xTaskHandle app_task_handle;  
    RingbufHandle_t ringBuff = NULL;
    const char * bt_name;
    uint32_t m_pkt_cnt = 0;
    const char *m_a2d_conn_state_str[4] = {"Disconnected", "Connecting", "Connected", "Disconnecting"};
    const char *m_a2d_audio_state_str[3] = {"Suspended", "Stopped", "Started"};
    esp_a2d_audio_state_t audio_state = ESP_A2D_AUDIO_STATE_STOPPED;
    esp_a2d_mct_t audio_type;
    void (*connection_changed)() = NULL;

    int init_bluetooth();
    void app_task_start_up(void);
    void app_task_shut_down(void);
    bool app_send_msg(app_msg_t *msg);
    bool app_work_dispatch(app_callback_t p_cback, uint16_t event, void *p_params, int param_len);
    void app_work_dispatched(app_msg_t *msg);
    void app_alloc_meta_buffer(esp_avrc_ct_cb_param_t *param);
    void av_new_track();
    void av_notify_evt_handler(uint8_t event_id, esp_avrc_rn_param_t event_parameter);
};

#endif

