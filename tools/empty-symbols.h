#ifndef SYMBOLS_H_
#define SYMBOLS_H_

struct symbols {
  const char *name;
  void *value;
};

extern const int symbols_nelts;

extern const struct symbols symbols[/* symbols_nelts */];

#endif /* SYMBOLS_H_ */
