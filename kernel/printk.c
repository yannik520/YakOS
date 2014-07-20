/*
 * Copyright (c) 2013 Yannik Li(Yanqing Li)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <kernel/kernel.h>
#include <string.h>
#include <kernel/task.h>
#include <kernel/printk.h>

#define LONGFLAG		0x00000001
#define LONGLONGFLAG		0x00000002
#define HALFFLAG		0x00000004
#define HALFHALFFLAG		0x00000008
#define SIZETFLAG		0x00000010
#define ALTFLAG			0x00000020
#define CAPSFLAG		0x00000040
#define SHOWSIGNFLAG		0x00000080
#define SIGNEDFLAG		0x00000100
#define LEFTFORMATFLAG		0x00000200
#define LEADZEROFLAG		0x00000400

#define LOG_BUF_LEN             1024
#define LOG_ALIGN               __alignof__(struct printk_log)

extern bool mmu_opened;

struct printk_log {
	u32 len;                /* length of entire record */
	u16 text_len;		/* length of text buffer */
	u16 level;		/* syslog level */
};

static char __log_buf[LOG_BUF_LEN];
static char *log_buf = __log_buf;
/* index and sequence number of the first record stored in the buffer */
static u64 log_first_seq;
static u32 log_first_idx;

/* index and sequence number of the next record to store in the buffer */
static u64 log_next_seq;
static u32 log_next_idx;

/* human readable text of the record */
static char *log_text(const struct printk_log *msg)
{
	return (char *)msg + sizeof(struct printk_log);
}

/* get record by index; idx must point to valid msg */
static struct printk_log *log_from_idx(u32 idx)
{
	struct printk_log *msg = (struct printk_log *)(log_buf + idx);

	/*
	 * A length == 0 record is the end of buffer marker. Wrap around and
	 * read the message at the start of the buffer.
	 */
	if (!msg->len)
		return (struct printk_log *)log_buf;
	return msg;
}

/* get next record; idx must point to valid msg */
static u32 log_next(u32 idx)
{
	struct printk_log *msg = (struct printk_log *)(log_buf + idx);

	/* length == 0 indicates the end of the buffer; wrap */
	/*
	 * A length == 0 record is the end of buffer marker. Wrap around and
	 * read the message at the start of the buffer as *this* one, and
	 * return the one after that.
	 */
	if (!msg->len) {
		msg = (struct printk_log *)log_buf;
		return msg->len;
	}
	return idx + msg->len;
}

static void log_store(int level, const char *text, u16 text_len)
{
	struct printk_log *msg;
	u32 size, pad_len;

	/* number of '\0' padding bytes to next message */
	size = sizeof(struct printk_log) + text_len;
	pad_len = (-size) & (LOG_ALIGN - 1);
	size += pad_len;

	while (log_first_seq < log_next_seq) {
		u32 free;

		if (log_next_idx > log_first_idx)
			free = max(LOG_BUF_LEN - log_next_idx, log_first_idx);
		else
			free = log_first_idx - log_next_idx;

		if (free > size + sizeof(struct printk_log))
			break;

		/* drop old messages until we have enough contiuous space */
		log_first_idx = log_next(log_first_idx);
		log_first_seq++;
	}

	if (log_next_idx + size + sizeof(struct printk_log) >= LOG_BUF_LEN) {
		/*
		 * This message + an additional empty header does not fit
		 * at the end of the buffer. Add an empty header with len == 0
		 * to signify a wrap around.
		 */
		memset(log_buf + log_next_idx, 0, sizeof(struct printk_log));
		log_next_idx = 0;
	}

	/* fill message */
	msg = (struct printk_log *)(log_buf + log_next_idx);
	memcpy(log_text(msg), text, text_len);
	msg->text_len = text_len;
	msg->level    = level;
	msg->len = sizeof(struct printk_log) + text_len + pad_len;

	/* insert message */
	log_next_idx += msg->len;
	log_next_seq++;
}

void puts(const char *s)
{
	/*if (mmu_opened) {
		__puts(s);
	}
	else {
		__puts_early(s);
		}*/
	__puts(s);
}

static char *longlong_to_string(char *buf, unsigned long long n, int len, unsigned int flag)
{
	int	pos	 = len;
	int	negative = 0;
	int	digit;

	if((flag & SIGNEDFLAG) && (long long)n < 0) {
		negative = 1;
		n = -n;
	}

	buf[--pos] = 0;
	
	/* only do the math if the number is >= 10 */
	while(n >= 10) {
		digit = (unsigned int)n % 10;
		n = (unsigned int)n / 10;

		buf[--pos] = digit + '0';
	}
	buf[--pos] = n + '0';
	
	if(negative)
		buf[--pos] = '-';
	else if((flag & SHOWSIGNFLAG))
		buf[--pos] = '+';

	return &buf[pos];
}

static char *longlong_to_hexstring(char *buf, unsigned long long u, int len, unsigned int flag)
{
	int pos = len;
	static const char hextable[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	static const char hextable_caps[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	const char *table;

	if((flag & CAPSFLAG))
		table = hextable_caps;
	else
		table = hextable;

	buf[--pos] = 0;
	do {
		unsigned int digit = u % 16;
		u /= 16;
	
		buf[--pos] = table[digit];
	} while(u != 0);

	return &buf[pos];
}

int vsnprintf(char *str, size_t len, const char *fmt, va_list ap)
{
	char			 c;
	unsigned char		 uc;
	const char		*s;
	unsigned long long	 n;
	void			*ptr;
	int			 flags;
	unsigned int		 format_num;
	size_t			 chars_written = 0;
	char			 num_buffer[32];

#define OUTPUT_CHAR(c) do { (*str++ = c); chars_written++; if (chars_written + 1 == len) goto done; } while(0)
#define OUTPUT_CHAR_NOLENCHECK(c) do { (*str++ = c); chars_written++; } while(0)

	for(;;) {	
		/* handle regular chars that aren't format related */
		while((c = *fmt++) != 0) {
			if(c == '%')
				break; /* we saw a '%', break and start parsing format */
			OUTPUT_CHAR(c);
		}

		/* make sure we haven't just hit the end of the string */
		if(c == 0)
			break;

		/* reset the format state */
		flags	   = 0;
		format_num = 0;

	next_format:
		/* grab the next format character */
		c = *fmt++;
		if(c == 0)
			break;
					
		switch(c) {
		case '0'...'9':
			if (c == '0' && format_num == 0)
				flags |= LEADZEROFLAG;
			format_num *= 10;
			format_num += c - '0';
			goto next_format;
		case '.':
			/* XXX for now eat numeric formatting */
			goto next_format;
		case '%':
			OUTPUT_CHAR('%');
			break;
		case 'c':
			uc = va_arg(ap, unsigned int);
			OUTPUT_CHAR(uc);
			break;
		case 's':
			s = va_arg(ap, const char *);
			if(s == 0)
				s = "<null>";
			goto _output_string;
		case '-':
			flags |= LEFTFORMATFLAG;
			goto next_format;
		case '+':
			flags |= SHOWSIGNFLAG;
			goto next_format;
		case '#':
			flags |= ALTFLAG;
			goto next_format;
		case 'l':
			if(flags & LONGFLAG)
				flags |= LONGLONGFLAG;
			flags |= LONGFLAG;
			goto next_format;
		case 'h':
			if(flags & HALFFLAG)
				flags |= HALFHALFFLAG;
			flags |= HALFFLAG;
			goto next_format;
		case 'z':
			flags |= SIZETFLAG;
			goto next_format;
		case 'D':
			flags |= LONGFLAG;
			/* fallthrough */
		case 'i':
		case 'd':
			n = (flags & LONGLONGFLAG) ? va_arg(ap, long long) :
			(flags & LONGFLAG) ? va_arg(ap, long) : 
			(flags & HALFHALFFLAG) ? (signed char)va_arg(ap, int) :
			(flags & HALFFLAG) ? (short)va_arg(ap, int) :
			(flags & SIZETFLAG) ? va_arg(ap, ssize_t) :
			va_arg(ap, int);
			flags |= SIGNEDFLAG;
			s = longlong_to_string(num_buffer, n, sizeof(num_buffer), flags);
			goto _output_string;
		case 'U':
			flags |= LONGFLAG;
			/* fallthrough */
		case 'u':
			n = (flags & LONGLONGFLAG) ? va_arg(ap, unsigned long long) :
			(flags & LONGFLAG) ? va_arg(ap, unsigned long) : 
			(flags & HALFHALFFLAG) ? (unsigned char)va_arg(ap, unsigned int) :
			(flags & HALFFLAG) ? (unsigned short)va_arg(ap, unsigned int) :
			(flags & SIZETFLAG) ? va_arg(ap, size_t) :
			va_arg(ap, unsigned int);
			s = longlong_to_string(num_buffer, n, sizeof(num_buffer), flags);
			goto _output_string;
		case 'p':
			flags |= LONGFLAG | ALTFLAG;
			goto hex;
		case 'X':
			flags |= CAPSFLAG;
			/* fallthrough */
		hex:
		case 'x':
			n = (flags & LONGLONGFLAG) ? va_arg(ap, unsigned long long) :
			(flags & LONGFLAG) ? va_arg(ap, unsigned long) : 
			(flags & HALFHALFFLAG) ? (unsigned char)va_arg(ap, unsigned int) :
			(flags & HALFFLAG) ? (unsigned short)va_arg(ap, unsigned int) :
			(flags & SIZETFLAG) ? va_arg(ap, size_t) :
			va_arg(ap, unsigned int);
			s = longlong_to_hexstring(num_buffer, n, sizeof(num_buffer), flags);
			if(flags & ALTFLAG) {
				OUTPUT_CHAR('0');
				OUTPUT_CHAR((flags & CAPSFLAG) ? 'X': 'x');
			}
			goto _output_string;
		case 'n':
			ptr = va_arg(ap, void *);
			if(flags & LONGLONGFLAG)
				*(long long *)ptr = chars_written;
			else if(flags & LONGFLAG)
				*(long *)ptr = chars_written;
			else if(flags & HALFHALFFLAG)
				*(signed char *)ptr = chars_written;
			else if(flags & HALFFLAG)
				*(short *)ptr = chars_written;
			else if(flags & SIZETFLAG)
				*(size_t *)ptr = chars_written;
			else 
				*(int *)ptr = chars_written;
			break;
		default:
			OUTPUT_CHAR('%');
			OUTPUT_CHAR(c);
			break;
		}

		/* move on to the next field */
		continue;

		/* shared output code */
_output_string:
		if (flags & LEFTFORMATFLAG) {
			/* left justify the text */
			unsigned int count = 0;
			while(*s != 0) {
				OUTPUT_CHAR(*s++);
				count++;
			}

			/* pad to the right (if necessary) */
			for (; format_num > count; format_num--)
				OUTPUT_CHAR(' ');
		} else {
			/* right justify the text (digits) */
			size_t string_len = strlen(s);
			char outchar = (flags & LEADZEROFLAG) ? '0' : ' ';
			for (; format_num > string_len; format_num--)
				OUTPUT_CHAR(outchar);

			/* output the string */
			while(*s != 0)
				OUTPUT_CHAR(*s++);
		}
		continue;
	}

done:
	/* null terminate */
	OUTPUT_CHAR_NOLENCHECK('\0');
	chars_written--; /* don't count the null */

#undef OUTPUT_CHAR
#undef OUTPUT_CHAR_NOLENCHECK

	return chars_written;
}

int _dvprintf(const char *fmt, va_list ap)
{
	char	buf[256];
	int	len;

	enter_critical_section();

	len = vsnprintf(buf, sizeof(buf), fmt, ap);
	if (0 < len) {
		log_store(0, buf, len+1);
		puts(buf);
	}

	exit_critical_section();

	return len;
}

int printk(const char *fmt, ...)
{
	int	err;
	va_list ap;

	va_start(ap, fmt);
	err = _dvprintf(fmt, ap);
	va_end(ap);

	return err;
}

void kmsg_dump(void)
{
	u32	 current_idx = log_first_idx;
	char	*current_text;

	if (current_idx == log_next_idx) {
		return;
	}
	
	while ((current_idx = log_next(current_idx)) != log_next_idx) {
		current_text = log_text(log_from_idx(current_idx));
		puts(current_text);
	}
}
