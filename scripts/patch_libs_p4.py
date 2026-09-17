#
# ESP32-P4 (M5Stack Tab5) library compatibility patches.
#
# Referenced only from [env:m5stack-tab5] via `extra_scripts`, and it only ever
# rewrites files inside THIS env's own .pio/libdeps/<env>/ tree, so the S3 board
# builds (which keep their own libdeps copies) are never affected.
#
# paulstoffregen/OneWire 2.3.8 predates the ESP32-P4. Its "plain ESP32" direct
# GPIO path writes bare `GPIO.out_w1ts = mask`, but on the P4 those registers are
# typed structs (gpio_out_w1ts_reg_t, ...) that are only assignable through their
# `.val` member. The bank-1 (pin >= 32) accesses already use `.val`; we just add
# `.val` to the five bank-0 accesses so the timing-critical bit-bang compiles.
#
Import("env")  # noqa: F821
import os

ONEWIRE_REPLACEMENTS = [
    ("return (GPIO.in >> pin) & 0x1;", "return (GPIO.in.val >> pin) & 0x1;"),
    ("GPIO.out_w1tc = ((uint32_t)1 << pin);", "GPIO.out_w1tc.val = ((uint32_t)1 << pin);"),
    ("GPIO.out_w1ts = ((uint32_t)1 << pin);", "GPIO.out_w1ts.val = ((uint32_t)1 << pin);"),
    ("GPIO.enable_w1tc = ((uint32_t)1 << pin);", "GPIO.enable_w1tc.val = ((uint32_t)1 << pin);"),
    ("GPIO.enable_w1ts = ((uint32_t)1 << pin);", "GPIO.enable_w1ts.val = ((uint32_t)1 << pin);"),
]


def _patch_file(path, replacements, label):
    if not os.path.isfile(path):
        print("[patch_libs_p4] %s: file not present yet (%s)" % (label, path))
        return
    with open(path, "r") as handle:
        src = handle.read()
    changed = False
    for old, new in replacements:
        if old in src:
            src = src.replace(old, new)
            changed = True
    if changed:
        with open(path, "w") as handle:
            handle.write(src)
        print("[patch_libs_p4] %s: patched for ESP32-P4" % label)
    else:
        print("[patch_libs_p4] %s: already compatible, nothing to do" % label)


libdeps_dir = os.path.join(env.subst("$PROJECT_LIBDEPS_DIR"), env.subst("$PIOENV"))  # noqa: F821

_patch_file(
    os.path.join(libdeps_dir, "OneWire", "util", "OneWire_direct_gpio.h"),
    ONEWIRE_REPLACEMENTS,
    "paulstoffregen/OneWire",
)
