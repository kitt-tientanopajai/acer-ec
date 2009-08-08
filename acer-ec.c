/*
 acer-ec.c

 Acer Embedded Controller Intepreter
 
 Copyright 2009 Kitt Tientanopajai <kitty@kitty.in.th>
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 MA 02110-1301, USA.

ChangeLogs
----------

* Sat, 08 Aug 2009 11:00:58 +0700 - 0.0.1 Kitt Tientanopajai
	- Initial Release 

Known Issues
------------

- Touchpad is not off although the flag is.

*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/io.h>

#define VERSION "0.0.1"
#define COMMAND_PORT 0x66
#define DATA_PORT 0x62

void toggle_bluetooth ();
void toggle_wireless ();
void toggle_touchpad ();
void show_status ();
void dump_reg_hex ();
void dump_reg_dec ();
unsigned char get_reg (unsigned char);
void set_reg (unsigned char, unsigned char);
void init_port ();
unsigned char read_port (unsigned char);
void write_port (unsigned char, unsigned char);

int
main (int argc, char *argv[]) 
{
	int opt;
	int status = EXIT_SUCCESS;

	if (argc == 1)
		show_status ();

	while ((opt = getopt (argc, argv, "bdg:hl:rstvw")) != -1)
		{
			switch (opt)
				{
					case 'b': /* bluetooth */
						toggle_bluetooth ();
						break;
					case 'd': /* dump registers */
						dump_reg_hex ();
						break;
					case 'g': /* get register value */
						printf ("%d\n", get_reg (atoi (optarg)));
						break;
					case 'l': /* backlight */
						set_reg (0xb9, atoi (optarg) % 10);
						break;
					case 't': /* touchpad */
						toggle_touchpad ();
						break;
					case 'w': /* wireless */
						toggle_wireless ();
						break;
					case 'r': /* register */
						dump_reg_dec ();
						break;
					case 's': /* show status */
						show_status ();
						break;
					case 'v': /* version */
						printf ("%s %s\n", argv[0], VERSION);
						break;
					case 'h': /* help */
						printf ("Usage: %s -bdhlrstvw \n", argv[0]);
						break;
					default:
						printf ("Usage: %s -bdhlrstvw \n", argv[0]);
						status = EXIT_FAILURE;
				}
		}

	return status;
}

void
toggle_bluetooth ()
{
	unsigned char r = get_reg (0xbb);
	if (r & 0x02)
		{
			set_reg (0xbb, r & 0xfd);
			printf ("Bluetooth is now off.\n");
		}
	else
		{
			set_reg (0xbb, r | 0x02);
			printf ("Bluetooth is now on.\n");
		}
}

void 
toggle_wireless ()
{
	unsigned char r = get_reg (0xbb);
	if (r & 0x01)
		{
			set_reg (0xbb, r & 0xfe);
			printf ("Wireless is now off.\n");
		}
	else
		{
			set_reg (0xbb, r | 0x01);
			printf ("Wireless is now on.\n");
		}
}

void 
toggle_touchpad ()
{
	unsigned char r = get_reg (0x9e);
	if (r & 0x08) 
		{
			set_reg (0x9e, r & 0xf7);
			printf ("Touchpad is now on.\n");
		}
	else 
		{
			set_reg (0x9e, r | 0x08);
			printf ("Touchpad is now off.\n");
		}
}

void
show_status (void)
{
	int r, i;
	/* wireless */
	r = get_reg (0xbb);
	if (r & 0x01)
		printf ("Wireless    : On\n");
	else
		printf ("Wireless    : Off\n");

	/* bluetooth */
	if (r & 0x02)
		printf ("Bluetooth   : On\n");
	else
		printf ("Bluetooth   : Off\n");

	/* touchpad */
	r = get_reg (0x9e);
	if (r & 0x08) 
		printf ("Touchpad    : Off\n");
	else
		printf ("Touchpad    : On\n");
		
	/* backlight */
	r = get_reg (0xb9);
	printf ("Brightness  : [");
	for (i = 0; i < r; i++)
		printf ("+");
	for (i = r; i < 9; i++)
		printf ("-");
	printf ("]\n");

	/* temperature */
	r = get_reg (0xb0);
	printf ("Temperature : %d'C\n", r);
}

void 
dump_reg_hex (void)
{
	unsigned int i;
	unsigned char r;

	printf ("Dump registers (Hexadecimal)\n\n   | 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f\n---+------------------------------------------------");
	init_port ();
	for (i = 0; i < 256; i++)
		{
			r = get_reg (i);
			if (i % 16 == 0)
				printf ("\n%02x | ", i);

			printf ("%02x ", r);
		}	
	printf ("\n");
}

void
dump_reg_dec (void)
{
	unsigned int i;
	unsigned char r;

	printf ("Dump registers (Decimal)\n\n   |   00   01   02   03   04   05   06   07   08   09   0a   0b   0c   0d   0e   0f\n---+--------------------------------------------------------------------------------");
	init_port ();
	for (i = 0; i < 256; i++)
		{
			r = get_reg (i);
			if (i % 16 == 0)
				printf ("\n%02x | ", i);

			printf ("%4d ", r);
		}	
	printf ("\n");
}

unsigned char
get_reg (unsigned char rid)
{
	unsigned char r;

	init_port ();
	write_port (0x80, 0x66);
	write_port (rid, 0x62);
	r = read_port (0x62);

	return r;
}

void
set_reg (unsigned char rid, unsigned char r)
{
	init_port ();
	write_port (0x81, 0x66);
	write_port (rid, 0x62);
	write_port (r, 0x62);
}

void 
init_port (void)
{
	if (ioperm (COMMAND_PORT, 1, 1) == -1) 
		{
			perror ("Error opening port");
			exit (EXIT_FAILURE);
		}

	if (ioperm (DATA_PORT, 1, 1) == -1) 
		{
			perror ("Error opening port");
			exit (EXIT_FAILURE);
		}
}

unsigned char
read_port (unsigned char port)
{
	/* check if port is available for read */
	while (!(inb (COMMAND_PORT) & 0x01))
		usleep (100);

	return inb (port);
}

void
write_port (unsigned char data, unsigned char port)
{
	/* check if port is available for write */
	while (inb (COMMAND_PORT) & 0x02)
		usleep (100);

	outb (data, port);
}
