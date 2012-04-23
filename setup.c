#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

#include <stdio.h>

#include "delay.h"

#include "lcd.h"
#include "key.h"
#include "eep.h"

#define MAX_MENU 6

/* Date: 27.06.2007   Time: 12:09:23 */
/* Maximum length of a line: 6  byte */
/* In total byte: 20                 */
/* FileName: вK                      */
prog_char menu[7][20]=
{
{66,120,111,227,184,0}, /* [0] "Входи"  */
{66,184,120,111,227,184,0}, /* [1] "Виходи" */
{0x42,0xB8,0x78,0x2E,0x20,0x63,0xBF,0x70,0x79,0xBC,0}, // "Out select" /*[2]*/
{0x54,0xB8,0xBE,0x20,0xBA,0xBB,0x61,0xBE,0x61,0xBD,0x61,0}, // "Valve type", /*[3]*/
{0x42,0xB8,0x78,'i',0xE3,0x20,0xBD,0x61,0x20,0xA8,0x45,0xA8,0x49,0}, // "Pnevmo output",/*[4]*/
{65,227,112,101,99,97,0}, /* [5] "Адреса" */
{0xAC,0xA8,0xE0,0x20,0xBC,0x65,0x70,0x65,0xB6,0}, // "Net speed", /*[6]*/
};


prog_char chan_n[]={0x4B,0x61,0xBD,0x61,0xBB,0x20,0x25,0x64,0};

extern char s[34];

char readkey()
{
	while(getkey()!=0xF0)
		wdt_reset();
	delay_ms(100);
	while(getkey()==0xF0)
		wdt_reset();
	return getkey();
}

void setup_adc_();
void setup_dac_();
void setup_outsel();
void setup_valve();
void setup_pnevmo();


void setup_addr();
void setup_netspd();

void calc();

void setup()
{
	register unsigned char i=0;
		
	while(1)
	{
		put_lcd_P(menu[i],0);
		s[0]=0;
		put_lcd(s,1);
		
		switch(readkey())
		{
			case MIN:
				if(--i&0x80) i=MAX_MENU;
				break;
			case MAX:
				if(++i>MAX_MENU) i=0;
				break;
				
			case STOP:
				return;
			case SET:
				switch(i)
				{
					case 0:
						setup_adc_();
						break;
					case 1:
						setup_dac_();
						break;
					case 2:
					  setup_outsel();
					  break;
					case 3:
					  setup_valve();
					  break;
					case 4:
					  setup_pnevmo();
					  break;
					case 5:
						setup_addr();
						break;
					case 6:
						setup_netspd();
						break;

				}
		}
		
	}
	return;
}


prog_char msg0[]={75,97,189,97,187,32,0}; /* [0] "Канал %d" */

void calibr_adc(unsigned char i);

void setup_adc_()
{
	char i=0;
	put_lcd_P(msg0,1);
	while (1)
	{
		put_lcd_P(menu[0],0);
		put_lcd_P(msg0,1);
		byte2lcd(128+64+6,0);
		byte2lcd(i+'1',1);
		switch(readkey())
		{
			case MIN:
				--i;
				i&=0x07;
				break;
			case MAX:
				++i;
				i &= 0x07;
				break;
			case SET:
				put_lcd_P(msg0,0);
				byte2lcd(128+6,0);
				byte2lcd(i+'1',1);
				while(getkey()!=0xF0);
				calibr_adc(i);
				break;
			case STOP:
				return;
		}
	}
		
	return ;			

}

prog_char msg1[]={75,97,189,97,187,32,37,100,32,37,52,100,0}; /* [0] "Канал %d %4d" */

extern unsigned int f[8][16];

extern    prog_char mg[] ; //={0x18,0x08,0x11,0x0A,0x14,0x0b,0x12,0x02};
extern 	prog_char grad[] ; // ={2	,5	,2	,0	,0	,0	,0	,0	};
prog_char sim_hi[]={0x1F,0x4,0xE,0x15,0x4,0x4,0x4,0x4};
prog_char sim_lo[]={0x4,0x4,0x4,0x4,0x15,0xE,0x4,0x1F};


void calibr_adc(unsigned char i)
{
	unsigned char j;
	long sum;
	
	unsigned int hi,lo;

	hi=eeprom_read_word(sca_hi+i);
	lo=eeprom_read_word(sca_lo+i);

	// перегрузити таблицю символів
	setcg(6,sim_lo);
	setcg(7,sim_hi);

	while(1)
	{
		sum=0;
		for(j=0;j<16;++j)
			sum+=f[i][j];
		sum>>=4;

		sprintf_P(s,msg1,i+1,(int)sum);
		put_lcd(s,0);
		
		sprintf_P(s,PSTR(" \06=%4d \07=%4d"),lo,hi);
		put_lcd(s,1);
		
		switch(getkey())
		{
			case MIN:
				lo=sum;
				break;
			case MAX:
				hi=sum;
				break;
			case SET:
				eeprom_write_word(sca_hi+i,hi);
				eeprom_write_word(sca_lo+i,lo);
			case STOP:
				// вернути назад таблицю символів
				setcg(6,mg);
				setcg(7,grad);

				return;
				
		}
		wdt_reset();
		delay_ms(300);
	
	}
	
	return;
}

//---------------------------------------------------------------------------------------------------------------------------
void calibr_dac(unsigned char i);


//* [0] "Виходи" */  prog_char msg2[]={66,184,120,111,227,184,0};

void setup_dac_()
{
	char i=0;
	put_lcd_P(msg0,1);
	while (1)
	{
		put_lcd_P(menu[1],0);
		put_lcd_P(msg0,1);
		byte2lcd(128+64+6,0);
		byte2lcd(i+'1',1);
		switch(readkey())
		{
			case MIN:
				--i;
				i&=0x01;
				break;
			case MAX:
				++i;
				i &= 0x01;
				break;
			case SET:
				put_lcd_P(msg0,0);
				byte2lcd(128+6,0);
				byte2lcd(i+'1',1);
				while(getkey()!=0xF0);
				calibr_dac(i);
				break;
			case STOP:
				return;
		}
	}
		
	return ;			

}

extern union {
	unsigned int i[2];
	unsigned char c[4];
} ao;

extern unsigned char c_d;

prog_char dacout[]={0x4D,0x61,0xBA,0x63,0x20,0xE1,0x41,0xA8,0x3D,0x25,0x34,0x64,0};
void calibr_dac(unsigned char i)
{
	c_d=0;

	ao.i[i]=eeprom_read_word(dac_hi+i);
	while(1)
	{
		sprintf_P(s,dacout,ao.i[i]);
		put_lcd(s,1);

		switch(getkey())
		{
			case MIN:
				if(--ao.i[i]<3900) ao.i[i]=3900;
				break;
			case MAX:
				if(++ao.i[i]>4095) ao.i[i]=4095;
				break;
				
			case SET:
				eeprom_write_word(dac_hi+i,ao.i[i]);
			case STOP:
				c_d=1;
				return;
		}
		delay_ms(100);
		wdt_reset();
	}	

}


prog_char setaddr[]={0x41,0xE3,0x70,0x65,0x63,0x61,0x20,0x25,0x64,0};
void setup_addr()
{
  char i=eeprom_read_byte(&addr);
  while(1)
  {
	sprintf_P(s,setaddr,i); // "Address %2d"
	put_lcd(s,1);
	switch(readkey())
	{
	  case MIN:
		if(--i<1) i=1;
		break;
	  case MAX:
		if(++i>100) i=100;
		break;
	  case SET:
		eeprom_write_byte(&addr,i);
	  case STOP:
		return;

	}
  }
  
}

prog_char mode[3][20]={"4-20mA","0-20mA","0-5mA"};

void setup_out_modesel(char v)
{
  char i=eeprom_read_byte(dac_m+v);
  while(1)
  {
	put_lcd_P(mode[i],1);
	switch(readkey())
	{
	  case MIN:
		if(--i&0x80) i=2;
		break;
	  case MAX:
		if(++i>2) i=0;
		break;
	  case SET:
		eeprom_write_byte(dac_m+v,i);
	  case STOP:
		return ;
	}
  }

}


void setup_outsel()
{
  char i=0;
  while(1)
  {
	put_lcd_P(menu[2],0);
	sprintf_P(s,chan_n,i+1);
	put_lcd(s,1);
	switch(readkey())
	{
	  case MIN:
		i=0;
		break;
	  case MAX:
		i=1;
		break;
	  case SET:
		  put_lcd(s,0);
		  setup_out_modesel(i);
		  calc();
		  break;
	  case STOP:
		return;
	}
  }
}

prog_char modevt[2][18]={
{0x4B,0xBB,0x61,0xBE,0x61,0xBD,0x20,0x48,0x4F,0x28,0x42,0x4F,0x29,0}, // "NO","NC"
{0x4B,0xBB,0x61,0xBE,0x61,0xBD,0x20,0x48,0xA4,0x28,0x42,0xA4,0x29,0}
};

void setup_vt(char v)
{
  char i=eeprom_read_byte(rev+v);
  while(1)
  {
	put_lcd_P(modevt[i],1);
	switch(readkey())
	{
	  case MIN:
		i=0;
		break;
	  case MAX:
		i=1;
		break;
	  case SET:
		eeprom_write_byte(rev+v,i);
	  case STOP:
		return ;
	}
  }
}



void setup_valve()
{
  char i=0;
  while(1)
  {
	put_lcd_P(menu[3],0);
	sprintf_P(s,chan_n,i+1);
	put_lcd(s,1);
	switch(readkey())
	{
	  case MIN:
		i=0;
		break;
	  case MAX:
		i=1;
		break;
	  case SET:
		  put_lcd(s,0);
		  setup_vt(i);
		  calc();
		  break;
	  case STOP:
		return;
	}
  }
}


prog_char pnen_msg[2][20]={
{0xA8,0x45,0xA8,0x49,0x20,0xB3,0xB8,0xBA,0xBB,0xC6,0xC0,0x65,0xBD,0x6F,0}, // "Pnevmo enable"
{0xA8,0x45,0xA8,0x49,0x20,0xB3,0xBA,0xBB,0xC6,0xC0,0x65,0xBD,0x6F,0} // "Pnevmo disble"
};

void setup_pnen(char v )
{
  register char i=eeprom_read_byte(pn_en+v);

  while(1)
  {
	put_lcd_P(pnen_msg+i,1);

	switch(readkey())
	{
	  case MIN:
		i=0;
		break;
	  case MAX:
		i=1;
		break;
	  case SET:
			eeprom_write_byte(pn_en+v,i);
	  case STOP:
		return;
	}

  }

}

void setup_pnevmo()
{
  char i=0;
  while(1)
  {
	put_lcd_P(menu[4],0);
	sprintf_P(s,chan_n,i+1);
	put_lcd(s,1);
	switch(readkey())
	{
	  case MIN:
		i=0;
		break;
	  case MAX:
		i=1;
		break;
	  case SET:
		  put_lcd(s,0);
		  setup_pnen(i);
		  break;
	  case STOP:
		return;
	}
  }

}


prog_char speed[5][10]={"9600","19200","38400","57600","115200"};

void setup_netspd()
{
  register char i=eeprom_read_byte(&spd);
  put_lcd_P(menu[6],0);
  while(1)
  {
	put_lcd_P(speed[i],1);
	switch(readkey())
	{
	  case MIN:
		if(--i&0x80) i=4;
		break;
	  case MAX:
		if(++i>4) i=0;
		break;
	  case SET:
		  eeprom_write_byte(&spd,i);
		  put_lcd_P(PSTR("Restart!"),1);
		  while(1); // перезапуск програми
	  case STOP:
		return;
	}
	
  }

}


