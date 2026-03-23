#include "display.h"
#include "config.h"

/*
 * App-level display initialisation.
 *
 * Called from app_start() after the platform HAL has already created the
 * hardware / simulation layer (hal_display_init() in hal_init()).
 * Sets the display to the application-defined default brightness so the
 * starting level is explicit and recorded by the HAL.
 */
void display_init(void)
{
    hal_display_set_brightness(DISPLAY_DEFAULT_BRIGHTNESS);
    hal_display_on();
}
