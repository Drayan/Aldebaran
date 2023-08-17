#include <core/logger.h>

int main(void)
{
    initialize_logging();

    AFATAL("A test message: %f", 3.14f);
    AERROR("A test message: %f", 3.14f);
    AWARN("A test message: %f", 3.14f);
    AINFO("A test message: %f", 3.14f);
    ADEBUG("A test message: %f", 3.14f);
    ATRACE("A test message: %f", 3.14f);

    AASSERT(1 == 0);

    shutdown_logging();

    return 0;
}
