#include <glib.h>
#include "aweather-gui.h"
#include "plugin-radar.h"

	  //{0xcc,0xff,0xff,0xff},{0xcc,0xff,0xff,0xff}, {0xcc,0xff,0xff,0xff},
	  //{0xcc,0x99,0xcc,0xff},{0xcc,0x99,0xcc,0xff}, {0xcc,0x99,0xcc,0xff},
	  //{0x99,0x66,0x99,0xff},{0x99,0x66,0x99,0xff}, {0x99,0x66,0x99,0xff},
	  //{0x66,0x33,0x66,0xff},{0x66,0x33,0x66,0xff}, {0x66,0x33,0x66,0xff},
	  //{0xcc,0xcc,0x99,0xff},{0xcc,0xcc,0x99,0xff}, {0xcc,0xcc,0x99,0xff},
	  //{0x99,0x99,0x66,0xff},{0x99,0x99,0x66,0xff}, {0x99,0x99,0x66,0xff},
colormap_t colormaps[] = {
	{"Reflectivity",
	 {{0x00,0x00,0x00,0x00}, //  0 dBZ
          {0x00,0x00,0x00,0x00}, //  1 dBZ
          {0x00,0x00,0x00,0x00}, //  2 dBZ
          {0x00,0x00,0x00,0x00}, //  3 dBZ
          {0x04,0xe9,0xe7,0x00}, //  4 dBZ
          {0x04,0xe9,0xe7,0x00}, //  5 dBZ
          {0x04,0xe9,0xe7,0x10}, //  6 dBZ
          {0x04,0xe9,0xe7,0x20}, //  7 dBZ
          {0x04,0xe9,0xe7,0x30}, //  8 dBZ
          {0x01,0x9f,0xf4,0x40}, //  9 dBZ
          {0x01,0x9f,0xf4,0x50}, // 10 dBZ
          {0x01,0x9f,0xf4,0x60}, // 11 dBZ
          {0x01,0x9f,0xf4,0x70}, // 12 dBZ
          {0x01,0x9f,0xf4,0x80}, // 13 dBZ
          {0x03,0x00,0xf4,0x90}, // 14 dBZ
          {0x03,0x00,0xf4,0xa0}, // 15 dBZ
          {0x03,0x00,0xf4,0xb0}, // 16 dBZ
          {0x03,0x00,0xf4,0xc0}, // 17 dBZ
          {0x03,0x00,0xf4,0xd0}, // 18 dBZ
          {0x02,0xfd,0x02,0xe0}, // 19 dBZ
          {0x02,0xfd,0x02,0xf0}, // 20 dBZ
          {0x02,0xfd,0x02,0xff}, // 21 dBZ
          {0x02,0xfd,0x02,0xff}, // 22 dBZ
          {0x02,0xfd,0x02,0xff}, // 23 dBZ
          {0x01,0xc5,0x01,0xff}, // 24 dBZ
          {0x01,0xc5,0x01,0xff}, // 25 dBZ
          {0x01,0xc5,0x01,0xff}, // 26 dBZ
          {0x01,0xc5,0x01,0xff}, // 27 dBZ
          {0x01,0xc5,0x01,0xff}, // 28 dBZ
          {0x00,0x8e,0x00,0xff}, // 29 dBZ
          {0x00,0x8e,0x00,0xff}, // 30 dBZ
          {0x00,0x8e,0x00,0xff}, // 31 dBZ
          {0x00,0x8e,0x00,0xff}, // 32 dBZ
          {0x00,0x8e,0x00,0xff}, // 33 dBZ
          {0xfd,0xf8,0x02,0xff}, // 34 dBZ
          {0xfd,0xf8,0x02,0xff}, // 35 dBZ
          {0xfd,0xf8,0x02,0xff}, // 36 dBZ
          {0xfd,0xf8,0x02,0xff}, // 37 dBZ
          {0xfd,0xf8,0x02,0xff}, // 38 dBZ
          {0xe5,0xbc,0x00,0xff}, // 39 dBZ
          {0xe5,0xbc,0x00,0xff}, // 40 dBZ
          {0xe5,0xbc,0x00,0xff}, // 41 dBZ
          {0xe5,0xbc,0x00,0xff}, // 42 dBZ
          {0xe5,0xbc,0x00,0xff}, // 43 dBZ
          {0xfd,0x95,0x00,0xff}, // 44 dBZ
          {0xfd,0x95,0x00,0xff}, // 45 dBZ
          {0xfd,0x95,0x00,0xff}, // 46 dBZ
          {0xfd,0x95,0x00,0xff}, // 47 dBZ
          {0xfd,0x95,0x00,0xff}, // 48 dBZ
          {0xfd,0x00,0x00,0xff}, // 49 dBZ
          {0xfd,0x00,0x00,0xff}, // 50 dBZ
          {0xfd,0x00,0x00,0xff}, // 51 dBZ
          {0xfd,0x00,0x00,0xff}, // 52 dBZ
          {0xfd,0x00,0x00,0xff}, // 53 dBZ
          {0xd4,0x00,0x00,0xff}, // 54 dBZ
          {0xd4,0x00,0x00,0xff}, // 55 dBZ
          {0xd4,0x00,0x00,0xff}, // 56 dBZ
          {0xd4,0x00,0x00,0xff}, // 57 dBZ
          {0xd4,0x00,0x00,0xff}, // 58 dBZ
          {0xbc,0x00,0x00,0xff}, // 59 dBZ
          {0xbc,0x00,0x00,0xff}, // 60 dBZ
          {0xbc,0x00,0x00,0xff}, // 61 dBZ
          {0xbc,0x00,0x00,0xff}, // 62 dBZ
          {0xbc,0x00,0x00,0xff}, // 63 dBZ
          {0xf8,0x00,0xfd,0xff}, // 64 dBZ
          {0xf8,0x00,0xfd,0xff}, // 65 dBZ
          {0xf8,0x00,0xfd,0xff}, // 66 dBZ
          {0xf8,0x00,0xfd,0xff}, // 67 dBZ
          {0xf8,0x00,0xfd,0xff}, // 68 dBZ
          {0x98,0x54,0xc6,0xff}, // 69 dBZ
          {0x98,0x54,0xc6,0xff}, // 70 dBZ
          {0x98,0x54,0xc6,0xff}, // 71 dBZ
          {0x98,0x54,0xc6,0xff}, // 72 dBZ
          {0x98,0x54,0xc6,0xff}, // 73 dBZ
          {0xfd,0xfd,0xfd,0xff}, // 74 dBZ
	  {0xfd,0xfd,0xfd,0xff}, // 75 dBZ
	  {0xfd,0xfd,0xfd,0xff}, // 76 dBZ
	  {0xfd,0xfd,0xfd,0xff}, // 77 dBZ
	  {0xfd,0xfd,0xfd,0xff}, // 78 dBZ
	  {0xfd,0xfd,0xfd,0xff}, // 79 dBZ
	  {0xfd,0xfd,0xfd,0xff}, // 80 dBZ
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00}}},
	{"Velocity",
	 {
	  {0x88,0x88,0x88,0x00}, //  0  
	  {0x98,0x77,0x77,0xff}, //  1  
	  {0x98,0x77,0x77,0xff}, //  2  
	  {0x98,0x77,0x77,0xff}, //  3  
	  {0x98,0x77,0x77,0xff}, //  4  
          {0x89,0x00,0x00,0xff}, //  5  
          {0xa2,0x00,0x00,0xff}, //  6  
          {0xa2,0x00,0x00,0xff}, //  7  
          {0xa2,0x00,0x00,0xff}, //  8  
          {0xa2,0x00,0x00,0xff}, //  9  
          {0xa2,0x00,0x00,0xff}, //  10 
          {0xb9,0x00,0x00,0xff}, //  11 
          {0xb9,0x00,0x00,0xff}, //  12 
          {0xb9,0x00,0x00,0xff}, //  13 
          {0xb9,0x00,0x00,0xff}, //  14 
          {0xb9,0x00,0x00,0xff}, //  15 
          {0xb9,0x00,0x00,0xff}, //  16 
          {0xb9,0x00,0x00,0xff}, //  17 
          {0xb9,0x00,0x00,0xff}, //  18 
          {0xb9,0x00,0x00,0xff}, //  19 
          {0xb9,0x00,0x00,0xff}, //  20 
          {0xb9,0x00,0x00,0xff}, //  21 
          {0xb9,0x00,0x00,0xff}, //  22 
          {0xd8,0x00,0x00,0xff}, //  23 
          {0xd8,0x00,0x00,0xff}, //  24 
          {0xd8,0x00,0x00,0xff}, //  25 
          {0xd8,0x00,0x00,0xff}, //  26 
          {0xd8,0x00,0x00,0xff}, //  27 
          {0xd8,0x00,0x00,0xff}, //  28 
          {0xd8,0x00,0x00,0xff}, //  29 
          {0xd8,0x00,0x00,0xff}, //  30 
          {0xef,0x00,0x00,0xff}, //  31 
          {0xef,0x00,0x00,0xff}, //  32 
          {0xef,0x00,0x00,0xff}, //  33 
          {0xef,0x00,0x00,0xff}, //  34 
          {0xef,0x00,0x00,0xff}, //  35 
          {0xef,0x00,0x00,0xff}, //  36 
          {0xef,0x00,0x00,0xff}, //  37 
          {0xef,0x00,0x00,0xff}, //  38 
          {0xef,0x00,0x00,0xff}, //  39 
          {0xef,0x00,0x00,0xff}, //  40 
          {0xfe,0x00,0x00,0xff}, //  41 
          {0xfe,0x00,0x00,0xff}, //  42 
          {0xfe,0x00,0x00,0xff}, //  43 
          {0xfe,0x00,0x00,0xff}, //  44 
          {0xfe,0x00,0x00,0xff}, //  45 
          {0xfe,0x00,0x00,0xff}, //  46 
          {0xfe,0x00,0x00,0xff}, //  47 
          {0xfe,0x00,0x00,0xff}, //  48 
          {0xfe,0x00,0x00,0xff}, //  49 
          {0xfe,0x00,0x00,0xff}, //  50 
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
	  {0x00,0x00,0x00,0x00},
          {0x02,0xfc,0x02,0xff}, // -50
          {0x02,0xfc,0x02,0xff}, // -49
          {0x02,0xfc,0x02,0xff}, // -48
          {0x02,0xfc,0x02,0xff}, // -47
          {0x02,0xfc,0x02,0xff}, // -46
          {0x02,0xfc,0x02,0xff}, // -45
          {0x02,0xfc,0x02,0xff}, // -44
          {0x02,0xfc,0x02,0xff}, // -43
          {0x02,0xfc,0x02,0xff}, // -42
          {0x02,0xfc,0x02,0xff}, // -41
          {0x01,0xe4,0x01,0xff}, // -40
          {0x01,0xe4,0x01,0xff}, // -39
          {0x01,0xe4,0x01,0xff}, // -38
          {0x01,0xe4,0x01,0xff}, // -37
          {0x01,0xe4,0x01,0xff}, // -36
          {0x01,0xe4,0x01,0xff}, // -35
          {0x01,0xe4,0x01,0xff}, // -34
          {0x01,0xe4,0x01,0xff}, // -33
          {0x01,0xe4,0x01,0xff}, // -32
          {0x01,0xe4,0x01,0xff}, // -31
          {0x01,0xc5,0x01,0xff}, // -30
          {0x01,0xc5,0x01,0xff}, // -29
          {0x01,0xc5,0x01,0xff}, // -28
          {0x01,0xc5,0x01,0xff}, // -27
          {0x01,0xc5,0x01,0xff}, // -26
          {0x01,0xc5,0x01,0xff}, // -25
          {0x01,0xc5,0x01,0xff}, // -24
          {0x01,0xc5,0x01,0xff}, // -23
          {0x07,0xac,0x04,0xff}, // -22
          {0x07,0xac,0x04,0xff}, // -21
          {0x07,0xac,0x04,0xff}, // -20
          {0x07,0xac,0x04,0xff}, // -19
          {0x07,0xac,0x04,0xff}, // -18
          {0x07,0xac,0x04,0xff}, // -17
          {0x07,0xac,0x04,0xff}, // -16
          {0x07,0xac,0x04,0xff}, // -15
          {0x07,0xac,0x04,0xff}, // -14
          {0x07,0xac,0x04,0xff}, // -13
          {0x07,0xac,0x04,0xff}, // -12
          {0x07,0xac,0x04,0xff}, // -11
          {0x06,0x8f,0x03,0xff}, // -10
          {0x06,0x8f,0x03,0xff}, // -9 
          {0x06,0x8f,0x03,0xff}, // -8 
          {0x06,0x8f,0x03,0xff}, // -7 
          {0x06,0x8f,0x03,0xff}, // -6 
          {0x04,0x72,0x02,0xff}, // -5 
          {0x7c,0x97,0x7b,0xff}, // -4 
          {0x7c,0x97,0x7b,0xff}, // -3 
          {0x7c,0x97,0x7b,0xff}, // -2 
          {0x7c,0x97,0x7b,0xff}, // -1 
	  }},
	{"Spectrum width",
	 {{0x00,0x00,0x00,0x00},
	  {0x00,0x00,0xa0,0xff},
	  {0x00,0x00,0xd0,0xff},
	  {0x00,0x00,0xff,0xff},
	  {0x20,0x20,0xff,0xff},
	  {0x40,0x40,0xff,0xff},
	  {0x70,0x70,0xff,0xff},
	  {0xa0,0xa0,0xff,0xff},
	  {0xd0,0xd0,0xff,0xff},
	  {0xff,0xff,0xff,0xff},
	  {0xff,0xb0,0xa0,0xff},
	  {0xff,0xd0,0x60,0xff},
	  {0x00,0x00,0xff,0xff},
	  {0x00,0x00,0xff,0xff},
	  {0x00,0x00,0xff,0xff},
	  {0x00,0x00,0xff,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0x00,0xf0,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0x00,0xf0,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0xf0,0x00,0x00,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0x00,0xf0,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0x00,0xf0,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0x00,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0xf0,0xf0,0xf0,0xff},
	  {0x00,0x00,0x00,0xff},
	  {0x00,0x00,0x00,0xff},
	  {0x00,0x00,0x00,0xff},
	  {0x00,0x00,0x00,0xff},
	  {0x00,0x00,0x00,0xff},
	  {0x00,0x00,0x00,0xff},
	  {0x00,0x00,0x00,0xff},
	  {0x00,0x00,0x00,0xff},
	  {0x00,0x00,0x00,0xff},
	  {0x00,0x00,0x00,0xff},
	  {0x00,0x00,0x00,0xff},
	  {0x00,0x00,0x00,0xff},
	  {0x00,0x00,0x00,0xff},
	  {0x00,0x00,0x00,0xff},
	  {0x00,0x00,0x00,0xff}}},
	{NULL, {{}}},
};
