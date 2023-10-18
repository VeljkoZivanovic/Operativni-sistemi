// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
static int bojas = 0x0700;
static int bojap = 0x0000;
static char tempo[5000];
static void consputc(int);

static int panicked = 0;

static struct {
	struct spinlock lock;
	int locking;
} cons;

static void
printint(int xx, int base, int sign)
{
	static char digits[] = "0123456789abcdef";
	char buf[16];
	int i;
	uint x;

	if(sign && (sign = xx < 0))
		x = -xx;
	else
		x = xx;

	i = 0;
	do{
		buf[i++] = digits[x % base];
	}while((x /= base) != 0);

	if(sign)
		buf[i++] = '-';

	while(--i >= 0)
		consputc(buf[i]);
}

// Print to the console. only understands %d, %x, %p, %s.
void
cprintf(char *fmt, ...)
{
	int i, c, locking;
	uint *argp;
	char *s;

	locking = cons.locking;
	if(locking)
		acquire(&cons.lock);

	if (fmt == 0)
		panic("null fmt");

	argp = (uint*)(void*)(&fmt + 1);
	for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
		if(c != '%'){
			consputc(c);
			continue;
		}
		c = fmt[++i] & 0xff;
		if(c == 0)
			break;
		switch(c){
		case 'd':
			printint(*argp++, 10, 1);
			break;
		case 'x':
		case 'p':
			printint(*argp++, 16, 0);
			break;
		case 's':
			if((s = (char*)*argp++) == 0)
				s = "(null)";
			for(; *s; s++)
				consputc(*s);
			break;
		case '%':
			consputc('%');
			break;
		default:
			// Print unknown % sequence to draw attention.
			consputc('%');
			consputc(c);
			break;
		}
	}

	if(locking)
		release(&cons.lock);
}

void
panic(char *s)
{
	int i;
	uint pcs[10];

	cli();
	cons.locking = 0;
	// use lapiccpunum so that we can call panic from mycpu()
	cprintf("lapicid %d: panic: ", lapicid());
	cprintf(s);
	cprintf("\n");
	getcallerpcs(&s, pcs);
	for(i=0; i<10; i++)
		cprintf(" %p", pcs[i]);
	panicked = 1; // freeze other CPU
	for(;;)
		;
}

#define BACKSPACE 0x100
#define CRTPORT 0x3d4
int pos;
static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory
static ushort *crtniz [5000];

static void
cgaputc(int c)
{

	// Cursor position: col + 80*row.
	outb(CRTPORT, 14);
	pos = inb(CRTPORT+1) << 8;
	outb(CRTPORT, 15);
	pos |= inb(CRTPORT+1);

	if(c == '\n')
		pos += 80 - pos%80;
	else if(c == BACKSPACE){
		if(pos > 0) --pos;
	} else
		crt[pos++] = (c&0xff) | bojap + bojas;

	if(pos < 0 || pos > 25*80)
		panic("pos under/overflow");

	if((pos/80) >= 24){  // Scroll up.
		memmove(crt, crt+80, sizeof(crt[0])*23*80);
		pos -= 80;
		memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
	}

	outb(CRTPORT, 14);
	outb(CRTPORT+1, pos>>8);
	outb(CRTPORT, 15);
	outb(CRTPORT+1, pos);
	crt[pos] = ' ' | 0x0700;
}

void
consputc(int c)
{
	if(panicked){
		cli();
		for(;;)
			;
	}

	if(c == BACKSPACE){
		uartputc('\b'); uartputc(' '); uartputc('\b');
	} else
		uartputc(c);
	cgaputc(c);
}

#define INPUT_BUF 128
struct {
	char buf[INPUT_BUF];
	uint r;  // Read index
	uint w;  // Write index
	uint e;  // Edit index
} input;

#define C(x)  ((x)-'@')  // Control-x
#define A(x)  ((x)-'@')
static int t = 0;
static int red = -8;
void
consoleintr(int (*getc)(void))
{
	int c, doprocdump = 0;
	int pozicija = pos;
	acquire(&cons.lock);
	while((c = getc()) >= 0){
		switch(c){
		case C('P'):  // Process listing.
			// procdump() locks cons.lock indirectly; invoke later
			doprocdump = 1;
			break;
		case C('U'):  // Kill line.
			while(input.e != input.w &&
			      input.buf[(input.e-1) % INPUT_BUF] != '\n'){
				input.e--;
				consputc(BACKSPACE);
			}
			break;
		case C('H'): case '\x7f':  // Backspace
			if(input.e != input.w){
				input.e--;
				consputc(BACKSPACE);
			}
			break;
		case A('C'):
		{
			if(t == 0)
			{
				pozicija = pos;
				sacuvaj(pozicija);
				nacrtaj(pozicija);
				t=1;
				if(pozicija%80<=70)
				{
					for(int k = (pos+1)+(red*80); k<(pos+10)+(red*80); k++)
					{
						tempo[k] = crt[k];
						crt[k] = 0x2000 | tempo[k];
					}
				}
				else
				{
					for(int k = (pos-1)+(red*80); k>(pos-10)+(red*80); k--)
					{
						tempo[k] = crt[k];
						crt[k] = 0x2000 | tempo[k];
					}
				}
				break;
			}
			else
			{
				obrisi(pozicija);
				t = 0;
				if(red == -8)
				{
					bojap = 0x0000;
					bojas = 0x0700;
					break;
				
				}
				if(red == -6)
				{
					bojap = 0x7000;
					bojas = 0x0500;
					break;
			
				}
				if(red == -4)
				{
					bojap = 0x3000;
					bojas = 0x0400;
					break;
			
				}
				if(red == -2)
				{
					bojap = 0x6000;
					bojas = 0x0700;
					break;
				}
			}
			
		}
		case('s'):
		{
			if(t == 1)
			{
				if(pozicija%80<=70)
				{
					for(int k = (pos+1)+(red*80); k<(pos+10)+(red*80); k++)
					{
						tempo[k] = crt[k];
						crt[k] = 0x7000 | tempo[k];
					}
				}
				else
				{
					for(int k = (pos-1)+(red*80); k>(pos-10)+(red*80); k--)
					{
						tempo[k] = crt[k];
						crt[k] = 0x7000 | tempo[k];
					}
				}
				
				red +=2;
				if(red == 0)
					red = -8;
				
				if(pozicija%80<=70)
				{
					for(int k = (pos+1)+(red*80); k<(pos+10)+(red*80); k++)
					{
						tempo[k] = crt[k];
						crt[k] = 0x2000 | tempo[k];
					}
				}
				else
				{
					for(int k = (pos-1)+(red*80); k>(pos-10)+(red*80); k--)
					{
						tempo[k] = crt[k];
						crt[k] = 0x2000 | tempo[k];
					}
				}
				break;
			}
			
		}
		case('w'):
		{
			if(t == 1)
			{
				if(pozicija%80<=70)
				{
					for(int k = (pos+1)+(red*80); k<(pos+10)+(red*80); k++)
					{
						tempo[k] = crt[k];
						crt[k] = 0x7000 | tempo[k];
					}
				}
				else
				{
					for(int k = (pos-1)+(red*80); k>(pos-10)+(red*80); k--)
					{
						tempo[k] = crt[k];
						crt[k] = 0x7000 | tempo[k];
					}
				}
				
				red -= 2;
				if(red == -10)
					red = -2;
			
				if(pozicija%80<=70)
				{
					for(int k = (pos+1)+(red*80); k<(pos+10)+(red*80); k++)
					{
						tempo[k] = crt[k];
						crt[k] = 0x2000 | tempo[k];
					}
				}
				else
				{
					for(int k = (pos-1)+(red*80); k>(pos-10)+(red*80); k--)
					{
						tempo[k] = crt[k];
						crt[k] = 0x2000 | tempo[k];
					}
				}
				break;
			}
		}
		default:
			if(t==0)
			{
				if(c != 0 && input.e-input.r < INPUT_BUF){
					c = (c == '\r') ? '\n' : c;
					input.buf[input.e++ % INPUT_BUF] = c;
					consputc(c);
					if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
						input.w = input.e;
						wakeup(&input.r);
					}
				}
				
			}
			break;
		}
	}
	release(&cons.lock);
	if(doprocdump) {
		procdump();  // now call procdump() wo. cons.lock held
	}
}

int
consoleread(struct inode *ip, char *dst, int n)
{
	uint target;
	int c;

	iunlock(ip);
	target = n;
	acquire(&cons.lock);
	while(n > 0){
		while(input.r == input.w){
			if(myproc()->killed){
				release(&cons.lock);
				ilock(ip);
				return -1;
			}
			sleep(&input.r, &cons.lock);
		}
		c = input.buf[input.r++ % INPUT_BUF];
		if(c == C('D')){  // EOF
			if(n < target){
				// Save ^D for next time, to make sure
				// caller gets a 0-byte result.
				input.r--;
			}
			break;
		}
		*dst++ = c;
		--n;
		if(c == '\n')
			break;
	}
	release(&cons.lock);
	ilock(ip);

	return target - n;
}

int
consolewrite(struct inode *ip, char *buf, int n)
{
	int i;

	iunlock(ip);
	acquire(&cons.lock);
	for(i = 0; i < n; i++)
		consputc(buf[i] & 0xff);
	release(&cons.lock);
	ilock(ip);

	return n;
}

void
consoleinit(void)
{
	initlock(&cons.lock, "console");

	devsw[CONSOLE].write = consolewrite;
	devsw[CONSOLE].read = consoleread;
	cons.locking = 1;

	ioapicenable(IRQ_KBD, 0);
}
void nacrtaj(int pos)
{
	if(pos%80<=70)
	{
		int k = (pos+1)-(9*80);
		for(int i = 0; i < 9; i++)
		{
			crt[k] = 0x7000 | '_';
			crt[k+160] = 0x7000 | '_';
			crt[k+320] = 0x7000 | '_';
			crt[k+480] = 0x7000 | '_';
			crt[k+640] = 0x7000 | '_';
			k++;
		}
		k = (pos+1)-(8*80);
		crt[k++] = 0x7000 | '|';
		crt[k++] = 0x7000 | 'W';
		crt[k++] = 0x7000 | 'H';
		crt[k++] = 0x7000 | 'T';
		crt[k++] = 0x7000 | ' ';
		crt[k++] = 0x7000 | 'B';
		crt[k++] = 0x7000 | 'L';
		crt[k++] = 0x7000 | 'K';
		crt[k++] = 0x7000 | '|';
		k = (pos+1)-(6*80);
		crt[k++] = 0x7000 | '|';
		crt[k++] = 0x7000 | 'P';
		crt[k++] = 0x7000 | 'U';
		crt[k++] = 0x7000 | 'R';
		crt[k++] = 0x7000 | ' ';
		crt[k++] = 0x7000 | 'W';
		crt[k++] = 0x7000 | 'H';
		crt[k++] = 0x7000 | 'T';
		crt[k++] = 0x7000 | '|';
		k = (pos+1)-(4*80);
		crt[k++] = 0x7000 | '|';
		crt[k++] = 0x7000 | 'R';
		crt[k++] = 0x7000 | 'E';
		crt[k++] = 0x7000 | 'D';
		crt[k++] = 0x7000 | ' ';
		crt[k++] = 0x7000 | 'A';
		crt[k++] = 0x7000 | 'Q';
		crt[k++] = 0x7000 | 'U';
		crt[k++] = 0x7000 | '|';
		k = (pos+1)-(2*80);
		crt[k++] = 0x7000 | '|';
		crt[k++] = 0x7000 | 'W';
		crt[k++] = 0x7000 | 'H';
		crt[k++] = 0x7000 | 'T';
		crt[k++] = 0x7000 | ' ';
		crt[k++] = 0x7000 | 'Y';
		crt[k++] = 0x7000 | 'E';
		crt[k++] = 0x7000 | 'L';
		crt[k++] = 0x7000 | '|';
	}
	else
	{
		int k = (pos-1)-(9*80);
		for(int i = 0; i < 9; i++)
		{
			crt[k] = 0x7000 | '_';
			crt[k+160] = 0x7000 | '_';
			crt[k+320] = 0x7000 | '_';
			crt[k+480] = 0x7000 | '_';
			crt[k+640] = 0x7000 | '_';
			k--;
		}
		k = (pos-9)-(8*80);
		crt[k++] = 0x7000 | '|';
		crt[k++] = 0x7000 | 'W';
		crt[k++] = 0x7000 | 'H';
		crt[k++] = 0x7000 | 'T';
		crt[k++] = 0x7000 | ' ';
		crt[k++] = 0x7000 | 'B';
		crt[k++] = 0x7000 | 'L';
		crt[k++] = 0x7000 | 'K';
		crt[k++] = 0x7000 | '|';
		k = (pos-9)-(6*80);
		crt[k++] = 0x7000 | '|';
		crt[k++] = 0x7000 | 'P';
		crt[k++] = 0x7000 | 'U';
		crt[k++] = 0x7000 | 'R';
		crt[k++] = 0x7000 | ' ';
		crt[k++] = 0x7000 | 'W';
		crt[k++] = 0x7000 | 'H';
		crt[k++] = 0x7000 | 'T';
		crt[k++] = 0x7000 | '|';
		k = (pos-9)-(4*80);
		crt[k++] = 0x7000 | '|';
		crt[k++] = 0x7000 | 'R';
		crt[k++] = 0x7000 | 'E';
		crt[k++] = 0x7000 | 'D';
		crt[k++] = 0x7000 | ' ';
		crt[k++] = 0x7000 | 'A';
		crt[k++] = 0x7000 | 'Q';
		crt[k++] = 0x7000 | 'U';
		crt[k++] = 0x7000 | '|';
		k = (pos-9)-(2*80);
		crt[k++] = 0x7000 | '|';
		crt[k++] = 0x7000 | 'W';
		crt[k++] = 0x7000 | 'H';
		crt[k++] = 0x7000 | 'T';
		crt[k++] = 0x7000 | ' ';
		crt[k++] = 0x7000 | 'Y';
		crt[k++] = 0x7000 | 'E';
		crt[k++] = 0x7000 | 'L';
		crt[k++] = 0x7000 | '|';
	}	
}
void sacuvaj(int pos)
{
	if(pos%80<=70)
	{
		int k = (pos+1)-(9*80);
		for(int i = 0; i < 9; i++)
		{
			for(int j = k+(i*80); j < k+9+(i*80); j++)
			{
				crtniz[j] = crt[j];
			}
		}
	}
	else
	{
		int k = (pos-1)-(9*80);
		for(int i = 0; i < 9; i++)
		{
			for(int j = k+(i*80); j > k-9+(i*80); j--)
			{
				crtniz[j] = crt[j];
			}
		}
	}
}
void obrisi(int pos)
{
	if(pos%80<=70)
	{
		int k = (pos+1)-(9*80);
		for(int i = 0; i < 9; i++)
		{
			for(int j = k+(i*80); j < k+9+(i*80); j++)
			{
				crt[j] = crtniz[j];
			}
		}
	}
	else
	{
		int k = (pos-1)-(9*80);
		for(int i = 0; i < 9; i++)
		{
			for(int j = k+(i*80); j > k-9+(i*80); j--)
			{
				crt[j] = crtniz[j];
			}
		}
	}
}
