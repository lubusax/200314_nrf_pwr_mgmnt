
#include <stdbool.h>
#include <stdint.h>
#include "boards.h"
#include "bsp.h"
#include "bsp_nfc.h"
#include "app_timer.h"
#include "nordic_common.h"
#include "app_error.h"
#include "nrf_drv_clock.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_delay.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

// #if NRF_PWR_MGMT_CONFIG_USE_SCHEDULER
// #include "app_scheduler.h"
// #define APP_SCHED_MAX_EVENT_SIZE    0   /**< Maximum size of scheduler events. */
// #define APP_SCHED_QUEUE_SIZE        4   /**< Maximum number of events in the scheduler queue. */
// #endif // NRF_PWR_MGMT_CONFIG_USE_SCHEDULER

#define BTN_ID_READY                0   
/**< ID of the button used to change the readiness to sleep. */

#define BTN_ID_SLEEP                1   
/**< ID of the button used to put the application into sleep/system OFF mode. */

#define BTN_ID_WAKEUP               1   
/**< ID of the button used to wake up the application. */

#define BTN_ID_RESET                2   
/**< ID of the button used to reset the application. */

static volatile bool m_stay_in_sysoff;  
/**< True if the application should stay in system OFF mode. */

static volatile bool m_is_ready;        
/**< True if the application is ready to enter sleep/system OFF mode. */

static volatile bool m_sysoff_started;  
/**< True if the application started sleep preparation. */

/**@brief Handler for shutdown preparation.
 */
bool shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
    uint32_t err_code;

    if (m_is_ready == false)
    {
        m_sysoff_started = true; 
        // application is not ready to enter sysOFF mode but sysOFF started
        NRF_LOG_RAW_INFO("\r\nmessage from shutdown handler\r\n");
        NRF_LOG_RAW_INFO("\r\nnot ready to enter sysOFF mode but sysOFF started\r\n");
        NRF_LOG_RAW_INFO("\r\n");
        NRF_LOG_FLUSH();
        return false; // false: don´t enter sysOFF
    }

    switch (event)
    {
        case NRF_PWR_MGMT_EVT_PREPARE_SYSOFF:
            //NRF_LOG_INFO("NRF_PWR_MGMT_EVT_PREPARE_SYSOFF");
            NRF_LOG_RAW_INFO("\r\n####### ##  shutdown handler shutdown handler  SYSOFF   SYSOFF\r\n");
            NRF_LOG_RAW_INFO("\r\n####### ####### case PWR MGMT EVT PREPARE      SYSOFF   SYSOFF\r\n");
            NRF_LOG_FLUSH();
            err_code = bsp_buttons_disable();
            APP_ERROR_CHECK(err_code);
            break;

        case NRF_PWR_MGMT_EVT_PREPARE_WAKEUP:
            //NRF_LOG_INFO("NRF_PWR_MGMT_EVT_PREPARE_WAKEUP");
            NRF_LOG_RAW_INFO("\r\n####### ##  shutdown handler shutdown handler WAKEUP   WAKEUP\r\n");
            NRF_LOG_RAW_INFO("\r\n####### ####### case PWR MGMT EVT PREPARE     WAKEUP   WAKEUP\r\n");
            NRF_LOG_FLUSH();
            err_code = bsp_buttons_disable();
            // Suppress NRF_ERROR_NOT_SUPPORTED return code.
            UNUSED_VARIABLE(err_code);

            err_code = bsp_wakeup_button_enable(BTN_ID_WAKEUP);
            // Suppress NRF_ERROR_NOT_SUPPORTED return code.
            UNUSED_VARIABLE(err_code);

            err_code = bsp_nfc_sleep_mode_prepare();
            // Suppress NRF_ERROR_NOT_SUPPORTED return code.
            UNUSED_VARIABLE(err_code);
            break;

        case NRF_PWR_MGMT_EVT_PREPARE_DFU:
            NRF_LOG_ERROR("Entering DFU is not supported by this example.");
            APP_ERROR_HANDLER(NRF_ERROR_API_NOT_IMPLEMENTED);
            break;

        case NRF_PWR_MGMT_EVT_PREPARE_RESET:
            // NRF_LOG_INFO("NRF_PWR_MGMT_EVT_PREPARE_RESET");
            NRF_LOG_RAW_INFO("\r\n####### ##  shutdown handler shutdown handler  RESET   RESET\r\n");
            NRF_LOG_RAW_INFO("\r\n####### ####### case PWR MGMT EVT PREPARE      RESET   RESET\r\n");
            NRF_LOG_FLUSH();
            break;
    }

    err_code = app_timer_stop_all();
    APP_ERROR_CHECK(err_code);

    return true; // true: app can sysOFF
}

/**@brief Register application shutdown handler 
 * with priority 0 --> highest priority */
NRF_PWR_MGMT_HANDLER_REGISTER(shutdown_handler, 0); 


/**@brief Function for handling BSP events.
 */
static void bsp_evt_handler(bsp_event_t evt)
{
    #if NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_ENABLED
        nrf_pwr_mgmt_feed();
        NRF_LOG_RAW_INFO("\r\nwhenever you push any button the sysoff timeout timer is reset");
        NRF_LOG_RAW_INFO("\r\nbsp evt handler \r\n#######  Power management fed - It has a timeout of 5 seconds\r\n");
        NRF_LOG_RAW_INFO("\r\n");
        NRF_LOG_FLUSH();
    #endif // NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_ENABLED
    // whenever you push any button the sysoff timeout timer is reset

    switch (evt)
    {
        case BSP_EVENT_KEY_0: // BUTTON 0


            if (m_is_ready)
            {
                m_is_ready = false;
                NRF_LOG_RAW_INFO("bsp evt handler - BUTTON 0 - EVENT KEY 0 \r\nxxxxxxxxxxx NOT READY for shutdown\r\n");
                NRF_LOG_RAW_INFO("\r\n");
                NRF_LOG_FLUSH();
            }
            else
            {
                m_is_ready = true;
                NRF_LOG_INFO("bsp evt handler - BUTTON 0 - EVENT KEY 0 \r\n ------  ## ---  READY for shutdown\r\n");
                NRF_LOG_RAW_INFO("\r\n");
                NRF_LOG_FLUSH();
            }
            if (m_sysoff_started && m_is_ready)
            {
                nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_CONTINUE);
                NRF_LOG_INFO("\r\n bsp evt handler - BUTTON 0 - EVENT KEY 0 \r\n ------  ## ---  SHUTDOWN CONTINUE\r\n");
                NRF_LOG_RAW_INFO("\r\n");
                NRF_LOG_FLUSH();
            }
            break;

        case BSP_EVENT_SYSOFF: // BUTTON 1
            m_stay_in_sysoff = true;
            NRF_LOG_INFO("\r\n bsp evt handler - BUTTON 1  - EVENT SYSOFF \r\n ------  ## ---  STAY IN SYSOFF (set stay_in_sysoff to true)\r\n");
            NRF_LOG_RAW_INFO("\r\n");
            NRF_LOG_FLUSH();
            break;

        case BSP_EVENT_SLEEP: // BUTTON 1
            if (m_stay_in_sysoff)
            {
                nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_STAY_IN_SYSOFF);
                NRF_LOG_INFO("\r\n bsp evt handler - BUTTON 1  - EVENT SLEEP \r\n ------  ## ---   Go to System OFF and stay there (stay_in_sysoff is already true)\r\n");
                NRF_LOG_RAW_INFO("\r\n");
                NRF_LOG_FLUSH();
            }
            else
            {
                nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
                NRF_LOG_INFO("\r\n bsp evt handler - BUTTON 1  - EVENT SLEEP \r\n ------  ## ---  GO TO SYSOFF (stay_in_sysoff is false)\r\n");
                NRF_LOG_RAW_INFO("\r\n");
                NRF_LOG_FLUSH();
            }
            break;

        case BSP_EVENT_RESET: // BUTTON 2
            nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_RESET);
            NRF_LOG_INFO("\r\n bsp evt handler - BUTTON 2  - EVENT RESET \r\n ------  ## ---  SHUTDOWN RESET \r\n");
            NRF_LOG_RAW_INFO("\r\n");
            NRF_LOG_FLUSH();
            break;

        default:
            return; // no implementation needed
    }
}


/**@brief Function for initializing low-frequency clock.
 */
static void lfclk_config(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_clock_lfclk_request(NULL);
}


/**@brief Function for initializing the BSP module.
 */
static void bsp_configuration()
{
    uint32_t err_code;

    err_code = bsp_init(BSP_INIT_BUTTONS, bsp_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_event_to_button_action_assign(BTN_ID_SLEEP, // BUTTON 1
                                                 BSP_BUTTON_ACTION_LONG_PUSH,
                                                 BSP_EVENT_SYSOFF);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_event_to_button_action_assign(BTN_ID_SLEEP, // BUTTON 1
                                                 BSP_BUTTON_ACTION_RELEASE,
                                                 BSP_EVENT_SLEEP);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_event_to_button_action_assign(BTN_ID_READY, // BUTTON 0
                                                 BSP_BUTTON_ACTION_PUSH,
                                                 BSP_EVENT_NOTHING);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_event_to_button_action_assign(BTN_ID_READY, // BUTTON 0
                                                 BSP_BUTTON_ACTION_RELEASE,
                                                 BSP_EVENT_KEY_0);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_event_to_button_action_assign(BTN_ID_RESET, // BUTTON 2
                                                 BSP_BUTTON_ACTION_RELEASE,
                                                 BSP_EVENT_RESET);
    APP_ERROR_CHECK(err_code);
}


/**
 * @brief Function for application main entry.
 */
int main(void)
{
     APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    NRF_LOG_RAW_INFO("\r\n\n ######       Power Management example\r\n");
    NRF_LOG_RAW_INFO("\r\n");
    NRF_LOG_FLUSH();

    lfclk_config();

    uint32_t err_code = app_timer_init(); // RTC initialize
    NRF_LOG_RAW_INFO("\r\n");
    NRF_LOG_FLUSH();

    APP_ERROR_CHECK(err_code);

    // #if NRF_PWR_MGMT_CONFIG_USE_SCHEDULER
    //     APP_SCHED_INIT(APP_SCHED_MAX_EVENT_SIZE, APP_SCHED_QUEUE_SIZE);
    // #endif // NRF_PWR_MGMT_CONFIG_USE_SCHEDULER
    bsp_configuration();

    ret_code_t ret_code = nrf_pwr_mgmt_init(); // pwr mgment initialize
    NRF_LOG_RAW_INFO("\r\n");
    NRF_LOG_FLUSH();


    APP_ERROR_CHECK(ret_code);
    static uint8_t number_pwr_mgmnt_cycles = 0;

    while (true)
    {
        // #if NRF_PWR_MGMT_CONFIG_USE_SCHEDULER
        //         app_sched_execute();
        // #endif // NRF_PWR_MGMT_CONFIG_USE_SCHEDULER

        if (NRF_LOG_PROCESS() == false)
        {
            nrf_pwr_mgmt_run();

            NRF_LOG_RAW_INFO("\r\n number_pwr_mgmnt_cycles: %d", number_pwr_mgmnt_cycles);
            NRF_LOG_RAW_INFO("\r\n");
            NRF_LOG_FLUSH();

            number_pwr_mgmnt_cycles++;
            //nrf_delay_ms(200);

        }
    }
}
