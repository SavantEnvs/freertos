/* libFuzzer entry point: bridges LLVMFuzzerTestOneInput to the harness's
 * mayhem_process_input(). One-time setup goes through mayhem_init() (weak
 * default is a no-op; the full-stack harness overrides it to boot the stack).
 *
 * All sanitizer runtime options (including leak-detection on/off) are owned
 * entirely by Mayhem at run time — this harness must never compile in a
 * default-options override for any sanitizer. */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern int mayhem_process_input(char *buf, size_t len);
extern int mayhem_init(void);

__attribute__((weak)) int mayhem_init(void)
{
    return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    static int initialized = 0;

    if (!initialized) {
        if (mayhem_init() != 0) {
            exit(1);
        }
        initialized = 1;
    }

    char *buf = malloc(size > 0 ? size : 1);
    if (buf == NULL) {
        return 0;
    }
    memcpy(buf, data, size);
    mayhem_process_input(buf, size);
    free(buf);
    return 0;
}
