#include <core/logger.h>
#include <core/application.h>

int main(void)
{
    application_config config;
    config.start_pos_x = 100;
    config.start_pos_y = 100;
    config.start_width = 1280;
    config.start_height = 720;
    config.name = "Aldebaran Engine Testbed";

    application_create(&config);

    application_run();

    return 0;
}
