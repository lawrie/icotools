
def generate_c_code(icosoc_h, icosoc_c, mod):
    code = """
static inline int32_t icosoc_@name@_get() {
    return *(volatile int32_t*)(0x20000000 + @addr@ * 0x10000);
}
"""

    code = code.replace("@name@", mod["name"])
    code = code.replace("@addr@", mod["addr"])
    icosoc_h.append(code)

