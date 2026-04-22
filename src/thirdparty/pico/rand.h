#ifndef TOUCHORD_PICO_RAND_SHIM_H
#define TOUCHORD_PICO_RAND_SHIM_H

/*
 * AMY's RP2040 path includes pico/rand.h, but current Touchord builds do not
 * need any symbols from it. Provide a local shim so we can build against the
 * Pico SDK version in this repo without patching the nested AMY checkout.
 */

#endif
