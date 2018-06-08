
def generate_c_code(icosoc_h, icosoc_c, mod):
    code = """
static inline void icosoc_@name@_write(uint8_t addr, uint8_t reg, uint8_t data1, uint8_t data2)
{
    *(volatile uint32_t*)(0x20000000 + @addr@ * 0x10000) = 
		(addr << 24) + (reg << 16) + (data1 << 8) + data2;
}

static inline void icosoc_@name@_write1(uint8_t addr, uint8_t reg, uint8_t data)
{
    *(volatile uint32_t*)(0x20000000 + @addr@ * 0x10000) = 
		((addr << 24) + (reg << 16) + (data << 8)) | 0x80000000;
}

static inline uint32_t icosoc_@name@_status()
{
    return *(volatile int32_t*)(0x20000000 + @addr@ * 0x10000);
}

static inline void icosoc_@name@_read(uint8_t addr, uint8_t reg)
{
    *(volatile uint32_t*)(0x20000004 + @addr@ * 0x10000) = 
		(addr << 17) + (reg << 8);
}
"""
    code = code.replace("@name@", mod["name"])
    code = code.replace("@addr@", mod["addr"])
    icosoc_h.append(code)
