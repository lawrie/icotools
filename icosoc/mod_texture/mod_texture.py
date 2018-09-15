
def generate_c_code(icosoc_h, icosoc_c, mod):
    code = """
static inline void icosoc_@name@_set_tile(uint8_t x, uint8_t y, uint8_t t)
{
    *(volatile uint32_t*)(0x20000000 + @addr@ * 0x10000) = 
		(((y << 8) + x) << 8) + t;
}

static inline void icosoc_@name@_set_sprite_pos(uint16_t x, uint16_t y)
{
    *(volatile uint32_t*)(0x20000004 + @addr@ * 0x10000) = 
		(y << 16) + x;
}
"""
    code = code.replace("@name@", mod["name"])
    code = code.replace("@addr@", mod["addr"])
    icosoc_h.append(code)
