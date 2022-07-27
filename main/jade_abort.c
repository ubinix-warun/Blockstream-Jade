#include "gui.h"
#include "keychain.h"
#include "sensitive.h"
#include "ui.h"

#include <freertos/task.h>

#include <esp_system.h>

#define SIZE_OF_ERR_BUFF 128

extern void __real_abort(void);

void jade_abort(const char* file, const int line_n)
{
    // Clear senstitive data
    keychain_clear();
    sensitive_clear_stack();

    if (gui_initialized()) {
        char details[SIZE_OF_ERR_BUFF] = { 0 };
        const int ret = snprintf(details, SIZE_OF_ERR_BUFF, "%s:%d", file, line_n);
        if (ret > 0 && ret < SIZE_OF_ERR_BUFF) {
            display_message_activity_two_lines("Internal error - restarting", details);
        } else {
            display_message_activity("Internal error - restarting");
        }
    }

    // Brief delay before abort
    // 1) for the user to see the "Internal error" message
    // 2) to give serial writer a chance to process outstanding logging
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    __real_abort();
}

// we wrap the real abort in the entire firmwawre so that ours gets called instead
// (see CMakeLists.txt) which will in turn call the real abort
// we do this so that we can clear the keychain and the sensitive stack

void __wrap_abort(void) { jade_abort("WRAPPED", 0); }
