#ifndef _ENCRY__H
#define _ENCRY__H

static inline char* XOR(char* buf, size_t len)
{
	for (size_t i = 0; i < len; ++i)
	{
		buf[i] ^= 1;
	}
}

static inline void Decrypt(char* buf, size_t len)
{
	XOR(buf, len);
}

static inline void Encry(char* buf, size_t len)
{
	XOR(buf, len);
}

#endif //_ENCRY__H