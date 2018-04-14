
def generate_c_code(icosoc_h, icosoc_c, mod):
    code = """
static inline uint32_t icosoc_@name@_get() {
    return *(volatile uint32_t*)(0x20000000 + @addr@ * 0x10000);
}
"""

    code = code.replace("@name@", mod["name"])
    code = code.replace("@addr@", mod["addr"])
    icosoc_h.append(code)

