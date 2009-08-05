/* 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/io.h>

#define VERSION "0.0.1"

int toggle_bluetooth ();
int set_backlight (int);
int toggle_touchpad ();
int toggle_wireless ();
int show_reg (unsigned int);
void dump_reg_hex ();
void dump_reg_dec ();
void init_port ();
unsigned int read_ec (unsigned int);
void write_ec (unsigned char, unsigned int);

int
main (int argc, char *argv[]) 
{
	int opt;
	int status;

	while ((opt = getopt (argc, argv, "bdhl:rs:tvw")) != -1)
		{
			switch (opt)
				{
					case 'b': /* bluetooth */
						status = toggle_bluetooth ();
						break;
					case 'd': /* dump registers */
						dump_reg_hex ();
						status = EXIT_SUCCESS;
						break;
					case 'l': /* backlight */
						status = set_backlight (atoi (optarg));
						break;
					case 't': /* touchpad */
						status = toggle_touchpad ();
						break;
					case 'w': /* wireless */
						status = toggle_wireless ();
						break;
					case 'r': /* register */
						dump_reg_dec ();
						status = EXIT_SUCCESS;
						break;
					case 's': /* show a register value */
						status = show_reg (atoi (optarg));
						break;
					case 'v': /* version */
						printf ("%s %s\n", argv[0], VERSION);
						status = EXIT_SUCCESS;
						break;
					case 'h': /* help */
						printf ("Usage: %s -bdhlrstvw \n", argv[0]);
						status = EXIT_SUCCESS;
						break;
					default:
						status = EXIT_FAILURE;
				}
		}

	return status;
}

int 
toggle_bluetooth ()
{
}

int set_backlight (int n)
{
}

int toggle_touchpad ()
{
}

int toggle_wireless ()
{
}

int
show_reg (unsigned int reg)
{
	unsigned int val;

	if (reg < 0 || reg > 255)
		return EXIT_FAILURE;

	init_port ();
	write_ec (0x80, 0x66);
	write_ec (reg, 0x62);
	val = read_ec (0x62);
	printf ("Register %02x: %02x (%d)\n", reg, val, val);

	return EXIT_SUCCESS;
}

void 
dump_reg_hex (void)
{
	unsigned int i;
	unsigned char val;

	printf ("Dump registers (Hexadecimal)\n\n   | 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f\n---+------------------------------------------------");
	init_port ();
	for (i = 0; i < 256; i++)
		{
			write_ec (0x80, 0x66);
			write_ec (i, 0x62); 
			val = read_ec (0x62); 
			if (i % 16 == 0)
				printf ("\n%02x | ", i);

			printf ("%02x ", val);
		}	
	printf ("\n");
}

void
dump_reg_dec (void)
{
	unsigned int i;
	unsigned char val;

	printf ("Dump registers (Decimal)\n\n   |   00   01   02   03   04   05   06   07   08   09   0a   0b   0c   0d   0e   0f\n---+--------------------------------------------------------------------------------");
	init_port ();
	for (i = 0; i < 256; i++)
		{
			write_ec (0x80, 0x66);
			write_ec (i, 0x62); 
			val = read_ec (0x62); 
			if (i % 16 == 0)
				printf ("\n%02x | ", i);

			printf ("%4d ", val);
		}	
	printf ("\n");
}

void 
init_port (void)
{
	if (ioperm (0x66, 1, 1) == -1) 
		{
			perror ("Error opening port 0x66");
			exit (EXIT_FAILURE);
		}

	if (ioperm (0x62, 1, 1) == -1) 
		{
			perror ("Error opening port 0x62");
			exit (EXIT_FAILURE);
		}
}

unsigned int
read_ec (unsigned int port)
{
	int i = 0;
	/* while (!(inb (0x66) & 0x01) && (i++ < 1000)) */
	while (!(inb (0x66) & 0x01))
		usleep (100);

	return inb (port);
}

void
write_ec (unsigned char data, unsigned int port)
{
	int i = 0;
	/* while ((inb (0x66) & 0x02) && (i++ < 1000)) */
	while (inb (0x66) & 0x02)
		usleep (100);

	outb (data, port);
}


