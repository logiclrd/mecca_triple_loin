#ifndef IO_H
#define IO_H

uint text_in();
void text_out(uint value);

void binary_in(uchar *target, int size, int stride);
void binary_skip_in(int size);
void binary_out(uchar *source, int size, int stride);

#endif /* IO_H */

