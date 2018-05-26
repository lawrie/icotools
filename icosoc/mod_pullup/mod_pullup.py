
def generate_c_code(icosoc_h, icosoc_c, mod):
    code = """
static inline void icosoc_@name@_set(uint32_t bitmask) {
    *(volatile uint32_t*)(0x20000000 + @addr@ * 0x10000) = bitmask;
}

static inline uint32_t icosoc_@name@_get() {
    return *(volatile uint32_t*)(0x20000000 + @addr@ * 0x10000);
}

static inline void icosoc_@name@_dir(uint32_t bitmask) {
    *(volatile uint32_t*)(0x20000004 + @addr@ * 0x10000) = bitmask;
}
"""

    code = code.replace("@name@", mod["name"])
    code = code.replace("@addr@", mod["addr"])
    icosoc_h.append(code)

