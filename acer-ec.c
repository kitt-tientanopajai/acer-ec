/*
 acer-ec.c

 Acer Embedded Controller Interpreter
 
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

* Sat, 12 Sep 2009 11:52:47 +0700 - v0.0.3
  - Long options
  - Bluetooth, touchpad, wireless can be on/off explicitly

* Thu, 03 Sep 2009 14:09:43 +0700 - v0.0.2
  - Code clean up
  - Add more options

* Sat, 08 Aug 2009 11:00:58 +0700 - v0.0.1
  - Initial Release 

Known Issues
------------
- Touchpad is not off although the flag is.

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include <sys/io.h>

#define VERSION "0.0.3"

/* EC Port */
#define EC_SC 0x66
#define EC_DATA 0x62

/* EC Command */
#define RD_EC 0x80
#define WR_EC 0x81
#define BE_EC 0x82
#define BD_EC 0x83

void help ();
void toggle_bluetooth ();
void toggle_touchpad ();
void toggle_wireless ();
void bluetooth_on ();
void bluetooth_off ();
void touchpad_on ();
void touchpad_off ();
void wireless_on ();
void wireless_off ();
void show_status ();
void dump_fields ();
void dump_regs ();
unsigned char get_reg (unsigned char);
void set_reg (unsigned char, unsigned char);
void init_port ();
unsigned char read_port (unsigned char);
void write_port (unsigned char, unsigned char);

int quiet = 0;

int
main (int argc, char *argv[])
{
  int opt;
  int status = EXIT_SUCCESS;

  static struct option longopts[] = 
    {
      {"bluetooth", optional_argument, NULL, 'b'},
      {"dump",      no_argument,       NULL, 'd'},
      {"help",      no_argument,       NULL, 'h'},
      {"backlight", required_argument, NULL, 'l'},
      {"quiet",     no_argument,       NULL, 'q'},
      {"registers", no_argument,       NULL, 'r'},
      {"status",    no_argument,       NULL, 's'},
      {"touchpad",  optional_argument, NULL, 't'},
      {"version",   no_argument,       NULL, 'v'},
      {"wireless",  optional_argument, NULL, 'w'},
      {0, 0, 0, 0}
    };

  if (argc == 1)
    show_status ();

  while ((opt = getopt_long (argc, argv, "b::dg:hl:qrst::vw::", longopts, 0)) != -1)
    {
      switch (opt)
        {
        case 'b':               /* bluetooth */
          if (optarg)
            {
              if (strncasecmp (optarg, "off") == 0)
                bluetooth_off ();
              else if (strncasecmp (optarg, "on") == 0)
                bluetooth_on ();
            }
          else 
            toggle_bluetooth ();
          break;
        case 'd':               /* dump fields */
          dump_fields ();
          break;
        case 'g':               /* get register value */
          printf ("%d\n", get_reg (atoi (optarg) % 256));
          break;
        case 'l':               /* backlight */
          set_reg (0xb9, atoi (optarg) % 10);
          break;
        case 'q':
          quiet = 1;
          break;
        case 't':               /* touchpad */
           if (optarg)
            {
              if (strncasecmp (optarg, "off") == 0)
                touchpad_off ();
              else if (strncasecmp (optarg, "on") == 0)
                touchpad_on ();
            }
          else 
            toggle_touchpad ();
          break;
        case 'w':               /* wireless */
           if (optarg)
            {
              if (strncasecmp (optarg, "off") == 0)
                wireless_off ();
              else if (strncasecmp (optarg, "on") == 0)
                wireless_on ();
            }
          else 
            toggle_wireless ();
          break;
        case 'r':               /* dump registers */
          dump_regs ();
          break;
        case 's':               /* show status */
          show_status ();
          break;
        case 'v':               /* version */
          printf ("%s %s\n", argv[0], VERSION);
          break;
        case '?':               /* help */
        case 'h':
          help (argv[0]);
          break;
        }
    }

  return status;
}

void
help (char *progname) 
{
  printf ("Usage: %s [OPTION...] \n", progname);
  printf ("\n");
  printf ("  -b, --blueooth             toggle bluetooth\n");
  printf ("      --blueooth={on | off}  set bluetooth on / off\n");
  printf ("  -t, --touchpad             toggle touchpad\n");
  printf ("      --touchpad={on | off}  set touchpad on / off\n");
  printf ("  -w, --wireless=on          toggle wireless\n");
  printf ("      --wireless={on | off}  set wireless on / off\n");
  printf ("  -l, --backlight n          set backlight to n (0 - 9)\n");
  printf ("  -q, --quiet                quiet mode (specify before -b, -t, -w)\n");
  printf ("  -g r                       get register value (0 - 255)\n");
  printf ("  -d, --dump                 dump known fields\n");
  printf ("  -r, --registers            dump registers\n");
  printf ("  -s, --status               show status\n");
  printf ("  -v, --version              show version\n");
  printf ("  -h, -?, --help             print this help\n");
  printf ("\n");
  printf ("Report bugs to kitty@kitty.in.th\n");
}

void
toggle_bluetooth ()
{
  unsigned char r = get_reg (0xbb);
  if (r & 0x02)
    bluetooth_off ();
  else
    bluetooth_on ();
}

void
toggle_touchpad ()
{
  unsigned char r = get_reg (0x9e);
  if (r & 0x08)
    touchpad_on ();
  else
    touchpad_off ();
}

void
toggle_wireless ()
{
  unsigned char r = get_reg (0xbb);
  if (r & 0x01)
    wireless_off ();
  else
    wireless_on ();
}

void
bluetooth_off (void)
{
  unsigned char r = get_reg (0xbb);
  set_reg (0xbb, r & 0xfd);
  if (!quiet)
    printf ("Bluetooth is now off.\n");
}

void
bluetooth_on (void)
{
  unsigned char r = get_reg (0xbb);
  set_reg (0xbb, r | 0x02);
  if (!quiet)
    printf ("Bluetooth is now on.\n");
} 

void
touchpad_off (void)
{
  unsigned char r = get_reg (0x9e);
  set_reg (0x9e, r | 0x08);
  if (!quiet)
    printf ("Touchpad is now off.\n");
}
 
void
touchpad_on (void)
{
  unsigned char r = get_reg (0x9e);
  set_reg (0x9e, r & 0xf7);
  if (!quiet)
    printf ("Touchpad is now on.\n");
}

void
wireless_off (void)
{
  unsigned char r = get_reg (0xbb);
  set_reg (0xbb, r & 0xfe);
  if (!quiet)
    printf ("Wireless is now off.\n");
}

void
wireless_on (void)
{
  unsigned char r = get_reg (0xbb);
  set_reg (0xbb, r | 0x01);
  if (!quiet)
    printf ("Wireless is now on.\n");
}

void
show_status (void)
{
  int r, i;
  /* wireless */
  r = get_reg (0xbb);
  if (r & 0x01)
    printf ("Wireless      : On\n");
  else
    printf ("Wireless      : Off\n");

  /* bluetooth */
  if (r & 0x02)
    printf ("Bluetooth     : On\n");
  else
    printf ("Bluetooth     : Off\n");

  /* touchpad */
  r = get_reg (0x9e);
  if (r & 0x08)
    printf ("Touchpad      : Off\n");
  else
    printf ("Touchpad      : On\n");

  /* backlight */
  r = get_reg (0xb9);
  printf ("Brightness    : [");
  for (i = 0; i < r; i++)
    printf ("+");
  for (i = r; i < 9; i++)
    printf ("-");
  printf ("]\n");

  /* temperature */
  r = get_reg (0xb0);
  printf ("CPU temp      : %d'C\n", r);

  /* Lid Switch */
  r = get_reg (0x9f);
  printf ("Lid switch    : %s\n", (r & 0x02) == 0x02 ? "Yes" : "No");

  /* Adapter Preset */
  r = get_reg (0xa3);
  printf ("Power adapter : %s\n", (r & 0x20) == 0x20 ? "Yes" : "No");

  /* Battery Status */
  printf ("Batt. status  : ");
  r = get_reg (0xc1);
  if ((r & 0x01) == 0x01)
    printf ("Discharging\n");
  else if ((r & 0x02) == 0x02)
    printf ("Charging\n");
  else if ((r & 0x04) == 0x04)
    printf ("Critical\n");
  else if ((r & 0x0f) == 0x00)
    printf ("Charged\n");
  else
    printf ("unknown\n");

  /* Battery Remain Capacity (mAh) */
  r = get_reg (0xc3) * 256 + get_reg (0xc2);
  printf ("Batt. capacity: %d mAh ", r);
  r = get_reg (0xce);
  printf ("(%d %%)\n", r);

  /* Battery Present Voltage (mV) */
  r = get_reg (0xc7) * 256 + get_reg (0xc6);
  printf ("Voltage       : %2.3f V\n", r / 1000.0);
}

void
dump_fields (void)
{
  unsigned int i;
  unsigned char r;

  r = get_reg (0x08);
  printf ("BATM %02x ", r);
  r = get_reg (0x09);
  printf ("%02x \n", r);


  r = get_reg (0x19);
  printf ("BATD %02x ", r);
  r = get_reg (0x1a);
  printf ("%02x ", r);
  r = get_reg (0x1b);
  printf ("%02x ", r);
  r = get_reg (0x1c);
  printf ("%02x ", r);
  r = get_reg (0x1d);
  printf ("%02x ", r);
  r = get_reg (0x1e);
  printf ("%02x ", r);
  r = get_reg (0x1f);
  printf ("%02x\n", r);

  /* SMB Protocol */
  r = get_reg (0x60);
  printf ("SMPR %02x\n", r);

  /* SMB Status */
  r = get_reg (0x61);
  printf ("SMST %02x\n", r);

  /* SMB Address */
  r = get_reg (0x62);
  printf ("SMAD %02x\n", r);

  /* SMB Command */
  r = get_reg (0x63);
  printf ("SMCM %02x\n", r);

  /* SMB Data */
  r = get_reg (0x64);
  printf ("SMDR %02x ", r);
  r = get_reg (0x65);
  printf ("%02x ", r);
  r = get_reg (0x66);
  printf ("%02x ", r);
  r = get_reg (0x67);
  printf ("%02x\n", r);

  /* SMB Block Count */
  r = get_reg (0x68);
  printf ("BCNT %02x\n", r);

  /* SMB Alarm Address */
  r = get_reg (0x69);
  printf ("SMAA %02x\n", r);

  /* SMB Alarm Data 0 */
  r = get_reg (0x6a);
  printf ("SMD0 %02x\n", r);

  /* SMB Alarm Data 1 */
  r = get_reg (0x6b);
  printf ("SMD1 %02x\n", r);

  r = get_reg (0x94);
  printf ("ERIB %02x ", r);
  r = get_reg (0x95);
  printf ("%02x \n", r);

  r = get_reg (0x96);
  printf ("ERBD %02x\n", r);

  r = get_reg (0x99);
  printf ("OSIF %d\n", r & 0x01);

  r = get_reg (0x9a);
  printf ("BAL1 %d\n", r & 0x01);
  printf ("BAL2 %d\n", (r & 0x02) == 0x02);
  printf ("BAL3 %d\n", (r & 0x04) == 0x04);
  printf ("BAL4 %d\n", (r & 0x08) == 0x08);
  printf ("BCL1 %d\n", (r & 0x10) == 0x10);
  printf ("BCL2 %d\n", (r & 0x20) == 0x20);
  printf ("BCL3 %d\n", (r & 0x40) == 0x40);
  printf ("BCL4 %d\n", (r & 0x80) == 0x80);

  r = get_reg (0x9b);
  printf ("BPU1 %d\n", r & 0x01);
  printf ("BPU2 %d\n", (r & 0x02) == 0x02);
  printf ("BPU3 %d\n", (r & 0x04) == 0x04);
  printf ("BPU4 %d\n", (r & 0x08) == 0x08);
  printf ("BOS1 %d\n", (r & 0x10) == 0x10);
  printf ("BOS2 %d\n", (r & 0x20) == 0x20);
  printf ("BOS3 %d\n", (r & 0x40) == 0x40);
  printf ("BOS4 %d\n", (r & 0x80) == 0x80);

  r = get_reg (0x9c);
  printf ("PHDD %d\n", r & 0x01);
  printf ("IFDD %d\n", (r & 0x02) == 0x02);
  printf ("IODD %d\n", (r & 0x04) == 0x04);
  printf ("SHDD %d\n", (r & 0x08) == 0x08);
  printf ("LS20 %d\n", (r & 0x10) == 0x10);
  printf ("EFDD %d\n", (r & 0x20) == 0x20);
  printf ("ECRT %d\n", (r & 0x40) == 0x40);
  printf ("LANC %d\n", (r & 0x80) == 0x80);

  r = get_reg (0x9d);
  printf ("SBTN %d\n", r & 0x01);
  printf ("VIDO %d\n", (r & 0x02) == 0x02);
  printf ("VOLD %d\n", (r & 0x04) == 0x04);
  printf ("VOLU %d\n", (r & 0x08) == 0x08);
  printf ("MUTE %d\n", (r & 0x10) == 0x10);
  printf ("CONT %d\n", (r & 0x20) == 0x20);
  printf ("BRGT %d\n", (r & 0x40) == 0x40);
  printf ("HBTN %d\n", (r & 0x80) == 0x80);

  r = get_reg (0x9e);
  printf ("S4SE %d\n", r & 0x01);
  printf ("SKEY %d\n", (r & 0x02) == 0x02);
  printf ("BKEY %d\n", (r & 0x04) == 0x04);
  printf ("TKEY %d\n", (r & 0x08) == 0x08);
  printf ("FKEY %d\n", (r & 0x10) == 0x10);
  printf ("DVDM %d\n", (r & 0x20) == 0x20);
  printf ("DIGM %d\n", (r & 0x40) == 0x40);
  printf ("CDLK %d\n", (r & 0x80) == 0x80);

  r = get_reg (0x9f);
  /* Lid Switch */
  printf ("LIDO %d\n", (r & 0x02) == 0x02);
  printf ("PMEE %d\n", (r & 0x04) == 0x04);
  printf ("PBET %d\n", (r & 0x08) == 0x08);
  printf ("RIIN %d\n", (r & 0x10) == 0x10);
  printf ("BTWK %d\n", (r & 0x20) == 0x20);
  printf ("DKIN %d\n", (r & 0x40) == 0x40);

  r = get_reg (0xa0);
  printf ("SWTH %d\n", (r & 0x40) == 0x40);
  printf ("HWTH %d\n", (r & 0x80) == 0x80);

  r = get_reg (0xa1);
  printf ("DTK0 %d\n", r & 0x01);
  printf ("DTK1 %d\n", (r & 0x02) == 0x02);
  printf ("OSUD %d\n", (r & 0x10) == 0x10);
  printf ("OSDK %d\n", (r & 0x20) == 0x20);
  printf ("OSSU %d\n", (r & 0x40) == 0x40);
  printf ("DKCG %d\n", (r & 0x80) == 0x80);

  r = get_reg (0xa2);
  printf ("ODTS %d\n", r);

  r = get_reg (0xa3);
  printf ("S1LD %d\n", r & 0x01);
  printf ("S3LD %d\n", (r & 0x02) == 0x02);
  printf ("VGAQ %d\n", (r & 0x04) == 0x04);
  printf ("PCMQ %d\n", (r & 0x08) == 0x08);
  printf ("PCMR %d\n", (r & 0x10) == 0x10);
  /* Adapter Preset */
  printf ("ADPT %d\n", (r & 0x20) == 0x20);
  printf ("SYS6 %d\n", (r & 0x40) == 0x40);
  printf ("SYS7 %d\n", (r & 0x80) == 0x80);

  r = get_reg (0xa4);
  printf ("PWAK %d\n", r & 0x01);
  printf ("MWAK %d\n", (r & 0x02) == 0x02);
  printf ("LWAK %d\n", (r & 0x04) == 0x04);
  printf ("RWAK %d\n", (r & 0x08) == 0x08);
  printf ("KWAK %d\n", (r & 0x40) == 0x40);
  printf ("MSWK %d\n", (r & 0x80) == 0x80);

  r = get_reg (0xa5);
  printf ("CCAC %d\n", r & 0x01);
  printf ("AOAC %d\n", (r & 0x02) == 0x02);
  printf ("BLAC %d\n", (r & 0x04) == 0x04);
  printf ("PSRC %d\n", (r & 0x08) == 0x08);
  printf ("BOAC %d\n", (r & 0x10) == 0x10);
  printf ("LCAC %d\n", (r & 0x20) == 0x20);
  printf ("AAAC %d\n", (r & 0x40) == 0x40);
  printf ("ACAC %d\n", (r & 0x80) == 0x80);

  r = get_reg (0xa6);
  printf ("PCEC %d\n", r);

  /* Passive Trip Point Temp. */
  r = get_reg (0xa7);
  printf ("THON %d\n", r);

  /* Critical Trip Point Temp. */
  r = get_reg (0xa8);
  printf ("THSD %d\n", r);

  r = get_reg (0xa9);
  printf ("THEM %d\n", r);

  r = get_reg (0xaa);
  printf ("TCON %d\n", r);

  r = get_reg (0xab);
  printf ("THRS %d\n", r);

  r = get_reg (0xac);
  printf ("TSSE %d\n", r);

  r = get_reg (0xad);
  printf ("FSSN %d\n", r & 0x0f);
  printf ("FANU %d\n", (r & 0xf0) > 4);

  r = get_reg (0xae);
  printf ("PTVL %d\n", r & 0x07);
  printf ("TTSR %d\n", (r & 0x40) == 0x40);
  printf ("TTHR %d\n", (r & 0x80) == 0x80);

  r = get_reg (0xaf);
  printf ("TSTH %d\n", r & 0x01);
  printf ("TSBC %d\n", (r & 0x02) == 0x02);
  printf ("TSBF %d\n", (r & 0x04) == 0x04);
  printf ("TSPL %d\n", (r & 0x08) == 0x08);
  printf ("TSBT %d\n", (r & 0x10) == 0x10);
  printf ("THTA %d\n", (r & 0x80) == 0x80);

  /* CPU Temp */
  r = get_reg (0xb0);
  printf ("CTMP %d\n", r);

  r = get_reg (0xb1);
  printf ("LTMP %d\n", r);

  r = get_reg (0xb2);
  printf ("SKTA %d\n", r);

  r = get_reg (0xb3);
  printf ("SKTB %d\n", r);

  r = get_reg (0xb4);
  printf ("SKTC %d\n", r);

  r = get_reg (0xb5);
  printf ("SKTD %d\n", r);

  r = get_reg (0xb6);
  printf ("NBTP %d\n", r);

  r = get_reg (0xb7);
  printf ("LANP %d\n", r & 0x01);
  printf ("LCDS %d\n", (r & 0x02) == 0x02);

  r = get_reg (0xb8);
  printf ("BTPV %d\n", r);

  /* Brightness */
  r = get_reg (0xb9);
  printf ("BRTS %d\n", r);

  r = get_reg (0xba);
  printf ("CRTS %d\n", r);

  r = get_reg (0xbb);
  /* WLAN Active */
  printf ("WLAT %d\n", r & 0x01);
  /* Bluetooth Active */
  printf ("BTAT %d\n", (r & 0x02) == 0x02);
  /* WLAN Adapter Present */
  printf ("WLEX %d\n", (r & 0x04) == 0x04);
  /* Bluetooth Adapter Present */
  printf ("BTEX %d\n", (r & 0x08) == 0x08);
  printf ("KLSW %d\n", (r & 0x10) == 0x10);
  printf ("WLOK %d\n", (r & 0x20) == 0x20);
  /* 3G Active */
  printf ("W3GA %d\n", (r & 0x40) == 0x40);
  /* 3G Adapter Present */
  printf ("W3GE %d\n", (r & 0x80) == 0x80);

  r = get_reg (0xbc);
  printf ("PJID %d\n", r);

  r = get_reg (0xbd);
  printf ("CPUN %d\n", r);

  r = get_reg (0xbe);
  printf ("THFN %d\n", r);

  r = get_reg (0xbf);
  printf ("MLED %d\n", r & 0x01);
  printf ("SCHG %d\n", (r & 0x02) == 0x02);
  printf ("SCCF %d\n", (r & 0x04) == 0x04);
  printf ("SCPF %d\n", (r & 0x08) == 0x08);
  printf ("ACIS %d\n", (r & 0x10) == 0x10);

  r = get_reg (0xc0);
  /* Battery Manufacturer */
  printf ("BTMF %d\n", (r & 0x70) > 4);
  printf ("BTY0 %d\n", (r & 0x80) == 0x80);

  /* Battery Status */
  /* Bit 0 = discharging */
  /* Bit 1 = charging */
  /* Bit 2 = critical */
  r = get_reg (0xc1);
  printf ("BST0 %d \n", r);

  /* Battery Remain Capacity (mAh) */
  r = get_reg (0xc2);
  printf ("BRC0 %02x ", r);
  r = get_reg (0xc3);
  printf ("%02x\n", r);

  r = get_reg (0xc4);
  printf ("BSN0 %02x ", r);
  r = get_reg (0xc5);
  printf ("%02x\n", r);

  /* Battery Present Voltage (mV) */
  r = get_reg (0xc6);
  printf ("BPV0 %02x ", r);
  r = get_reg (0xc7);
  printf ("%02x\n", r);

  /* Battery Design Voltage (mV) */
  r = get_reg (0xc8);
  printf ("BDV0 %02x ", r);
  r = get_reg (0xc9);
  printf ("%02x\n", r);

  /* Battery Design Capacity (mAh) */
  r = get_reg (0xca);
  printf ("BDC0 %02x ", r);
  r = get_reg (0xcb);
  printf ("%02x\n", r);

  /* Battery Full Charge (mAh) */
  r = get_reg (0xcc);
  printf ("BFC0 %02x ", r);
  r = get_reg (0xcd);
  printf ("%02x\n", r);

  /* Battery Guage (%) */
  r = get_reg (0xce);
  printf ("GAU0 %d\n", r);

  r = get_reg (0xcf);
  printf ("BSCY %d\n", r);

  r = get_reg (0xd0);
  printf ("BSCU %02x ", r);
  r = get_reg (0xd1);
  printf ("%02x\n", r);

  r = get_reg (0xd2);
  printf ("BAC0 %02x ", r);
  r = get_reg (0xd3);
  printf ("%02x\n", r);

  r = get_reg (0xd4);
  printf ("BTW0 %d\n", r);

  r = get_reg (0xd5);
  printf ("BATV %d\n", r);

  r = get_reg (0xd6);
  printf ("BPTC %d\n", r);

  r = get_reg (0xd7);
  printf ("BTTC %d\n", r);

  r = get_reg (0xd8);
  printf ("BTMA %02x ", r);
  r = get_reg (0xd9);
  printf ("%02x\n", r);

  r = get_reg (0xda);
  printf ("BTSC %d\n", r);

  r = get_reg (0xdb);
  printf ("BCIX %d\n", r);

  r = get_reg (0xdc);
  printf ("CCBA %d\n", r);

  r = get_reg (0xdd);
  printf ("CBOT %d\n", r);

  r = get_reg (0xde);
  printf ("BTSS %02x ", r);
  r = get_reg (0xdf);
  printf ("%02x\n", r);

  r = get_reg (0xe0);
  printf ("OVCC %d\n", r);

  r = get_reg (0xe1);
  printf ("CCFC %d\n", r);

  r = get_reg (0xe2);
  printf ("BADC %d\n", r);

  r = get_reg (0xe3);
  printf ("BSC1 %02x ", r);
  r = get_reg (0xe4);
  printf ("%02x\n", r);

  r = get_reg (0xe5);
  printf ("BSC2 %02x ", r);
  r = get_reg (0xe6);
  printf ("%02x\n", r);

  r = get_reg (0xe7);
  printf ("BSC3 %02x ", r);
  r = get_reg (0xe8);
  printf ("%02x\n", r);

  r = get_reg (0xe9);
  printf ("BSE4 %02x ", r);
  r = get_reg (0xea);
  printf ("%02x\n", r);

  r = get_reg (0xeb);
  printf ("BDME %02x ", r);
  r = get_reg (0xec);
  printf ("%02x\n", r);

  r = get_reg (0xf0);
  printf ("BTS1 %d\n", r);

  r = get_reg (0xf1);
  printf ("BTS2 %d\n", r);

  r = get_reg (0xf2);
  printf ("BSCS %02x ", r);
  r = get_reg (0xf3);
  printf ("%02x\n", r);

  r = get_reg (0xf4);
  printf ("BDAD %02x ", r);
  r = get_reg (0xf5);
  printf ("%02x\n", r);

  r = get_reg (0xf6);
  printf ("BACV %02x ", r);
  r = get_reg (0xf7);
  printf ("%02x\n", r);

  r = get_reg (0xf8);
  printf ("BDFC %02x ", r);
  r = get_reg (0xf9);
  printf ("%02x\n", r);
}

void
dump_regs (void)
{
  unsigned int i;
  unsigned char r;

  printf
    ("Dump registers (Decimal)\n\n   |   00   01   02   03   04   05   06   07   08   09   0a   0b   0c   0d   0e   0f\n---+--------------------------------------------------------------------------------");
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
  write_port (RD_EC, EC_SC);
  write_port (rid, EC_DATA);
  r = read_port (EC_DATA);

  return r;
}

void
set_reg (unsigned char rid, unsigned char r)
{
  init_port ();
  write_port (WR_EC, EC_SC);
  write_port (rid, EC_DATA);
  write_port (r, EC_DATA);
}

void
init_port (void)
{
  if (ioperm (EC_SC, 1, 1) == -1)
    {
      perror ("Error opening port");
      exit (EXIT_FAILURE);
    }

  if (ioperm (EC_DATA, 1, 1) == -1)
    {
      perror ("Error opening port");
      exit (EXIT_FAILURE);
    }
}

unsigned char
read_port (unsigned char port)
{
  /* check if port is available for read */
  while (!(inb (EC_SC) & 0x01))
    {
      struct timespec ts;
      ts.tv_sec = (time_t) 0;
      ts.tv_nsec = 100000;
      nanosleep (&ts, NULL);
    }

  return inb (port);
}

void
write_port (unsigned char data, unsigned char port)
{
  /* check if port is available for write */
  while (inb (EC_SC) & 0x02)
    {
      struct timespec ts;
      ts.tv_sec = (time_t) 0;
      ts.tv_nsec = 100000;
      nanosleep (&ts, NULL);
    }

  outb (data, port);
}
