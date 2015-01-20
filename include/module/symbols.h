#ifndef __SYMBOLS_H__
#define __SYMBOLS_H__
struct symbols {
	const char	*name;
	void		*value;
};


extern const int symbols_nelts;
extern const struct symbols symbols[177];
#endif
