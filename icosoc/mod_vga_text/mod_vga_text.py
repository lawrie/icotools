
def generate_c_code(icosoc_h, icosoc_c, mod):
    code = """
static inline void icosoc_@name@_setchar(uint8_t x, uint8_t y, uint8_t c)
{
    *(volatile uint32_t*)(0x20000000 + @addr@ * 0x10000) = 
		(((y << 6) + x) << 8) + c;
}
"""
    code = code.replace("@name@", mod["name"])
    code = code.replace("@addr@", mod["addr"])
    icosoc_h.append(code)
