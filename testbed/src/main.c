#include <core/logger.h>
#include <platform/platform.h>

int main(void)
{
    AFATAL("A test message: %f", 3.14f);
    AERROR("A test message: %f", 3.14f);
    AWARN("A test message: %f", 3.14f);
    AINFO("A test message: %f", 3.14f);
    ADEBUG("A test message: %f", 3.14f);
    ATRACE("A test message: %f", 3.14f);

    platform_state state;
    if(platform_startup(&state, "Aldebaran engine testbed", 100, 100, 1200, 720))
    {
        while(TRUE)
        {
            platform_pump_message(&state);
        }
    }

    platform_shutdown(&state);

    return 0;
}
