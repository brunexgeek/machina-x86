SECTIONS
{
    .text 0x80000000 :
    {
        *(entryp)
        *(.text)
    }
}
