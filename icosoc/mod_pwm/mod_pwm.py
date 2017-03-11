
def generate_c_code(icosoc_h, icosoc_c, mod):
    code = """
static inline void icosoc_@name@_setcounter(uint32_t val)
{
    *(volatile uint32_t*)(0x20000000 + @addr@ * 0x10000) = val;
}

static inline void icosoc_@name@_setmaxcnt(uint32_t val)
{
    *(volatile uint32_t*)(0x20000004 + @addr@ * 0x10000) = val;
}

static inline void icosoc_@name@_setoncnt(uint32_t val)
{
    *(volatile uint32_t*)(0x20000008 + @addr@ * 0x10000) = val;
}

static inline void icosoc_@name@_setoffcnt(uint32_t val)
{
    *(volatile uint32_t*)(0x2000000c + @addr@ * 0x10000) = val;
}

static inline uint32_t icosoc_@name@_getcounter()
{
    return *(volatile uint32_t*)(0x20000000 + @addr@ * 0x10000);
}

static inline uint32_t icosoc_@name@_getmaxcnt()
{
    return *(volatile uint32_t*)(0x20000004 + @addr@ * 0x10000);
}

static inline uint32_t icosoc_@name@_getoncnt()
{
    return *(volatile uint32_t*)(0x20000008 + @addr@ * 0x10000);
}

static inline uint32_t icosoc_@name@_getoffcnt()
{
    return *(volatile uint32_t*)(0x2000000c + @addr@ * 0x10000);
}
"""
    code = code.replace("@name@", mod["name"])
    code = code.replace("@addr@", mod["addr"])
    icosoc_h.append(code)
