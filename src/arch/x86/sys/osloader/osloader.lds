SECTIONS
{
	.text 0x90800 :
	{
		*(entryp)
		*(.text)
	}
}
