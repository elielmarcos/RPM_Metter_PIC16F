#include <pic16f628a.h>;  // PIC16F628A
#include <pic.h>;


__CONFIG(WDTE_OFF & MCLRE_OFF & BOREN_OFF & LVP_OFF & FOSC_HS & CP_OFF & CPD_OFF & PWRTE_OFF); // OSCILADOR Externo 16Mhz

#define _XTAL_FREQ 16000000

// Saídas para segmentos do display 7SEG 
#define SEG1 RB1
#define SEG2 RB2
#define SEG3 RB3
#define SEG4 RB4
#define SEG5 RB5
#define SEG6 RB6
#define SEG7 RB7
#define SEGP RA2

// Saídas para acionar os displays 7SEG
#define DIG1 RA0
#define DIG2 RA1

// Saída do Shift Light
#define S_LIGHT RA3

// Entrada dos botões
#define Botao_1        (!RA5)     // Entrada Botão 1 (CONFIRMA)
#define Botao_2        (!RA4)     // Entrada Botão 2 (INCREMENTA)

// === VARIAVEIS ===
unsigned char MENU=1;
unsigned int RPM=0;
//unsigned int TL=0;
//unsigned int TH=0;
unsigned int TIMER0=0;
unsigned int SHIFT_LIGHT=10000;
unsigned char CILINDROS=4;
unsigned char ACESO=20;
unsigned char APAGADO=0;
unsigned char FILTRO=1;
unsigned char MEDIA=0;
unsigned long RPM_MEDIA=0;


// === SEPARADOR DE UNIDADES ===
unsigned char unidade=0;
unsigned char dezena=0;
unsigned char centena=0;
unsigned char u_milhar=0;
unsigned char d_milhar=0;


// === BOTÕES ===
bit old_B1=0;          		// guarda ultimo valor de Botao_1
bit old_B2=0;         		// guarda ultimo valor de Botao_2

unsigned int TIMER_BOTAO_AGUARDA=0;
unsigned char TIMER_BOTAO_ACIONA=0;


// === CONFIGURA MEMORIA EEPROM PARA GRAVAÇÃO ===
__EEPROM_DATA(0x04,0x0A,0x0A,0x13,0x88,0x01,0x00,0x00);


// ==== Rotina Lê Dado na EEPROM ====

unsigned char LER_EEPROM(unsigned char address)
{
unsigned DADO_EEPROM=0;

  EEADR  = address;     // Seleciona o Endereço da Memória para leitura
  WREN   = 0;			// Habilita Leitura
  RD     = 1;           // Inicia Leitura
  NOP();				// Aguarda um tempinho
  NOP();
  DADO_EEPROM = EEDATA; // Captura o Dado da Memória 

return DADO_EEPROM;

}


// ==== Rotina Escrever Dado na EEPROM ====

void GRAVAR_EEPROM(unsigned char address, unsigned char DADO_EEPROM)
{

  EEADR  = address;     // Seleciona o Endereço da Memória
  EEDATA = DADO_EEPROM; // Dado a ser Gravado na Memória 
  WREN   = 1;           // Habilita Gravação
  GIE    = 0;           // Desabilita Interrupção Geral Para Gravação da Memória (Necessário para não gerar erro na gravação)
  PEIE   = 0;           // Desabilita Interrupção Periféricos Para Gravação da Memória (Necessário para não gerar erro na gravação)
  EECON2 = 0x55;        // Necessario Para Gravação
  EECON2 = 0xAA;        // Necessario Para Gravação 
  WR     = 1;           // Inicia Gravação
while(WR==1) {continue;}// Aguarda Término da Gravação
  WREN   = 0;           // Desabilita Gravação
  GIE    = 1;           // Habilita Interrupção Geral
  PEIE   = 1;           // Habilita Interrupção Periféricos

return;

}


// =============================================================
// ==========================BOTÕES=============================
// =============================================================

// ==== Botão Confirma ===
bit Botao_Enter(void)
{

   if ( (Botao_1 && (!old_B1)) || ((!Botao_1) && old_B1) ){   // testa se a entrada mudou de valor
       if ( Botao_1 && (!old_B1) ) {                        // Se mudou e esta apertado
        old_B1 = Botao_1;
        return(1);                                         // Retorna Verdadeiro
        }
        old_B1 = Botao_1;                                  // guarda "novo valor antigo"
       }
  return(0);                                               // Se não, retorna Falso

}

// ==== Botão Incrementa ===
bit Botao_Inc(void)
{

   if ( (Botao_2 && (!old_B2)) || ((!Botao_2) && old_B2) ){   // testa se a entrada mudou de valor
       if ( Botao_2 && (!old_B2) ) {                        // Se mudou e esta apertado
        old_B2 = Botao_2;
        return(1);                                         // Retorna Verdadeiro
        }
        old_B2 = Botao_2;                                  // guarda "novo valor antigo"
       }
  return(0);                                               // Se não, retorna Falso

}
// ==== Botão Incrementa Automatico ====
bit Botao_Inc_Auto(void)
{

   if (!Botao_2) {TIMER_BOTAO_AGUARDA=0; TIMER_BOTAO_ACIONA=0; }

   if (Botao_2)  { if (TIMER_BOTAO_AGUARDA!=400) {TIMER_BOTAO_AGUARDA++;}
                   else{ if (TIMER_BOTAO_ACIONA!=25) {TIMER_BOTAO_ACIONA++;}
                   else{TIMER_BOTAO_ACIONA=0; return(1);} }}

  return(0);
}



// =============================================================
// ===============          DIGITO         =====================
// =============================================================

void digito(char alfanumero)
{

switch (alfanumero) {
						
	       case 0: {
			SEG1=1;	SEG2=1;	SEG3=1;	SEG4=1;	SEG5=1;	SEG6=1;	SEG7=0;
			break;
			   }

		   case 1: {
			SEG1=0;	SEG2=1;	SEG3=1;	SEG4=0;	SEG5=0;	SEG6=0;	SEG7=0;
 			break;
			   }

		   case 2: {
			SEG1=1;	SEG2=1;	SEG3=0;	SEG4=1;	SEG5=1;	SEG6=0;	SEG7=1;
 			break;
 			   }

		   case 3: {
			SEG1=1;	SEG2=1;	SEG3=1;	SEG4=1;	SEG5=0;	SEG6=0;	SEG7=1;
 			break;
  			   }

		   case 4: {
			SEG1=0;	SEG2=1;	SEG3=1;	SEG4=0;	SEG5=0;	SEG6=1;	SEG7=1;
 			break;
 			   }

		   case 5: {
			SEG1=1;	SEG2=0;	SEG3=1;	SEG4=1;	SEG5=0;	SEG6=1;	SEG7=1;
 			break;
 			   }

		   case 6: {
			SEG1=1;	SEG2=0;	SEG3=1;	SEG4=1;	SEG5=1;	SEG6=1;	SEG7=1;
 			break;
 			   }

		   case 7: {
			SEG1=1;	SEG2=1;	SEG3=1;	SEG4=0;	SEG5=0;	SEG6=0;	SEG7=0;
 			break;
 			   }

		   case 8: {
			SEG1=1;	SEG2=1;	SEG3=1;	SEG4=1;	SEG5=1;	SEG6=1;	SEG7=1;
 			break;
 			   }

		   case 9: {
			SEG1=1;	SEG2=1;	SEG3=1;	SEG4=1;	SEG5=0;	SEG6=1;	SEG7=1;
 			break;
 			   }

		   case 10: { //H
			SEG1=0;	SEG2=1;	SEG3=1;	SEG4=0;	SEG5=1;	SEG6=1;	SEG7=1;
 			break;
 			   }

		   case 'b': {
			SEG1=0;	SEG2=0;	SEG3=1;	SEG4=1;	SEG5=1;	SEG6=1;	SEG7=1;
 			break;
 			   }

		   case 'C': {
			SEG1=1;	SEG2=0;	SEG3=0;	SEG4=1;	SEG5=1;	SEG6=1;	SEG7=0;
 			break;
			   }

		   case 'F': {
			SEG1=1;	SEG2=0;	SEG3=0;	SEG4=0;	SEG5=1;	SEG6=1;	SEG7=1;
 			break;
			   }

		   case ' ': {
			SEG1=0;	SEG2=0;	SEG3=0;	SEG4=0;	SEG5=0;	SEG6=0;	SEG7=0;
 			break;
 			   }

		   case '1': {
			SEG1=0;	SEG2=0;	SEG3=0;	SEG4=1;	SEG5=0;	SEG6=0;	SEG7=0;
 			break;
 			   }

		   case '2': {
			SEG1=0;	SEG2=0;	SEG3=1;	SEG4=0;	SEG5=1;	SEG6=0;	SEG7=0;
 			break;
 			   }

		   case '3': {
			SEG1=0;	SEG2=0;	SEG3=0;	SEG4=0;	SEG5=0;	SEG6=0;	SEG7=1;
 			break;
 			   }

		   case '4': {
			SEG1=0;	SEG2=1;	SEG3=0;	SEG4=0;	SEG5=0;	SEG6=1;	SEG7=0;
 			break;
 			   }

		   case '5': {
			SEG1=1;	SEG2=0;	SEG3=0;	SEG4=0;	SEG5=0;	SEG6=0;	SEG7=0;
 			break;
 			   }

		   case '-': {
			SEG1=0;	SEG2=0;	SEG3=0;	SEG4=0;	SEG5=0;	SEG6=0;	SEG7=1;
 			break;
 			   }

		   }

return;

}




// =============================================================
// ===============   TEMPO DIGITO ACESO    =====================
// =============================================================
void DELAY(unsigned char L)
{

if (L==0){__delay_us(10);}
while(L>=1){__delay_us(50); L--;}
	
return;
}




// =============================================================
// ===============   ESCREVE NO DISPLAY    =====================
// =============================================================

void escreve_display(unsigned char D1, unsigned char D2, unsigned char P1, unsigned char P2)
{

DIG1=DIG2=0;
digito(D1);
SEGP=P1;
DIG1=1;
DELAY(ACESO);
DIG1=DIG2=0;
DELAY(APAGADO);
digito(D2);
SEGP=P2;
DIG2=1;
DELAY(ACESO);
DIG1=DIG2=0;
DELAY(APAGADO);

return;

}



// =============================================================
// ===============SEPARA UNIDADES COM ZEROS=====================
// =============================================================

void separa_unidades(unsigned int Valor)
{

unidade=0;
dezena=0;
centena=0;
u_milhar=0;
d_milhar=0;

while(Valor>=10000){Valor=Valor-10000; d_milhar++;}
while(Valor>=1000){Valor=Valor-1000; u_milhar++;}
while(Valor>=100){Valor=Valor-100; centena++;}
while(Valor>=10){Valor=Valor-10; dezena++;}
while(Valor>0){Valor=Valor-1; unidade++;}

return;

}


// =============================================================
// =================   TESTE DO DISPLAY    =====================
// =============================================================

void teste_display(void)
{
unsigned char repet=0;

for (repet=0;repet<150;repet++){escreve_display('1','1',1,1);}
for (repet=0;repet<150;repet++){escreve_display('2','2',0,0);}
for (repet=0;repet<150;repet++){escreve_display('3','3',0,0);}
for (repet=0;repet<150;repet++){escreve_display('4','4',0,0);}
for (repet=0;repet<150;repet++){escreve_display('5','5',0,0);}
for (repet=0;repet<255;repet++){escreve_display(8,8,1,1);}
for (repet=0;repet<200;repet++){escreve_display(' ',' ',0,0);}


return;

}




// =============================================================
// =================   MOSTRAR SHIFT LIGHT   ====================
// =============================================================

void MOSTAR_SHIFT_LIGHT(void)
{
unsigned int repet=0;

separa_unidades(SHIFT_LIGHT);
S_LIGHT=1;

for (repet=0;repet<1000;repet++){escreve_display(u_milhar,centena,1,0);}

S_LIGHT=0;

for (repet=0;repet<200;repet++){escreve_display(' ',' ',0,0);}

for (repet=0;repet<300;repet++){escreve_display('-','-',0,0);}

return;
}






// =============================================================
// ================   M E T T E R - R P M   ====================
// =============================================================

void METTER(void)
{

separa_unidades(RPM);

if (RPM>=10000) {escreve_display(d_milhar,u_milhar,0,1);}
				else
				{escreve_display(u_milhar,centena,1,0);}


if (RPM>=SHIFT_LIGHT){S_LIGHT=1;}		// Aciona o Shitf Light com Switch Trigger (Histerese) 
if (RPM<=(SHIFT_LIGHT-100)){S_LIGHT=0;}


return;
}





// =============================================================
// =================   CONFIGURAR S-LIGHT   ====================
// =============================================================

void CONFIG_SL(void)
{

S_LIGHT=1;

separa_unidades(SHIFT_LIGHT);

escreve_display(u_milhar,centena,1,0);

if (Botao_Enter()){MENU=3; S_LIGHT=0; GRAVAR_EEPROM(0x03,(SHIFT_LIGHT >> 8)); GRAVAR_EEPROM(0x04,(SHIFT_LIGHT & 0x00FF));}
if (Botao_Inc() || Botao_Inc_Auto()){ if(SHIFT_LIGHT!=10000) {SHIFT_LIGHT+=100;} if(SHIFT_LIGHT==10000) {SHIFT_LIGHT=100;}}

return;
}




// =============================================================
// =================   CONFIGURAR CILINDRO   ===================
// =============================================================

void CONFIG_CILINDRO(void)
{

separa_unidades(CILINDROS);

escreve_display(unidade,'C',0,1);

if (Botao_Enter()){MENU=4; GRAVAR_EEPROM(0x00,CILINDROS);}
if (Botao_Inc()){ if(CILINDROS!=10) {CILINDROS+=2;} if(CILINDROS==10) {CILINDROS=2;}}

return;
}





// =============================================================
// ==================   CONFIGURAR FILTRO   ====================
// =============================================================

void CONFIG_FILTRO(void)
{

escreve_display('F',FILTRO,0,0);

if (Botao_Enter()){MENU=5; GRAVAR_EEPROM(0x05,FILTRO);}
if (Botao_Inc()){ if(FILTRO!=11) {FILTRO++;} if(FILTRO==11) {FILTRO=1;}}

return;
}




// =============================================================
// ===============   CONFIGURAR BRILHO DISPLAY   ===============
// =============================================================

void CONFIG_BRILHO(void)
{
unsigned int repet=0;
unsigned char ponto=0;

if ((ACESO==1)||(ACESO==3)||(ACESO==5)||(ACESO==7)||(ACESO==9)||(ACESO==11)||(ACESO==13)||(ACESO==15)||(ACESO==17)||(ACESO==19)){ponto=1;}
else{ponto=0;}

escreve_display('b',(ACESO/2),0,ponto);

if (Botao_Enter()){MENU=1; GRAVAR_EEPROM(0x01,ACESO); GRAVAR_EEPROM(0x02,APAGADO);
for (repet=0;repet<200;repet++){
escreve_display(' ',' ',0,0);}}

if (Botao_Inc() || Botao_Inc_Auto()){ if(ACESO!=21) {ACESO++; APAGADO--;} if(ACESO==21) {ACESO=0; APAGADO=20;}}


return;
}






// =============================================================
// ===================   ROTINA PRINCIPAL   ====================
// =============================================================


void main(void)
 {

CM0=1;						// Desabilita os comparadores
CM1=1;						// Desabilita os comparadores
CM2=1;						// Desabilita os comparadores

INTCON=0x00;				// Controle de interrupções
OPTION_REG=0b10000000;		// Opções de registradores (Prescaler, Timer0, RB pull-up, borda para interrupção)

TRISA=0b00110000;			// Configura I/O do PORTA
TRISB=0b00000001;			// Configura I/O do PORTB
PORTB=0;					// Zera o PORTA
PORTA=0;					// Zera o PORTB


TMR2IE=0;					// Desabilita Timer2
T0IE=0;						// Desabilita Timer0

// ======= Lê EEPROM ==========
CILINDROS=LER_EEPROM(0x00);
ACESO=LER_EEPROM(0x01);
APAGADO=LER_EEPROM(0x02);
SHIFT_LIGHT=(LER_EEPROM(0x03)<<8)+LER_EEPROM(0x04);
FILTRO=LER_EEPROM(0x05);


// =========== TESTE DE DISPLAY ==========
teste_display();			// Rotina Testa Display
MOSTAR_SHIFT_LIGHT();		// Rotina Mostrar o valor do Shitf Light que esta configurado


INTE=1;						// Habilita RB0 interrupção
INTF=0;						// Limpa flag de interrupção RB0

// ===== CONFIGURA TIMER 1 ======

   T1CON = 0b00100001; 		// NULO, NULO, T1CKPS1, T1CKPS0, T1OSCEN, T1SYNC, TMR1CS, TMR1ON - PRESCALAE 1:4
   PIE1 = 0b00000001; 		// EEIE, ADIE, NULO, NULO, CMIE, NULO, NULO, TMR1IE
   PIR1 = 0b00000000; 		// EEIF, ADIF, NULO, NULO, CMIF, NULO, NULO, TMR1IF

   GIE=1;              		// Habilita Interrupção GERAL
   PEIE=1;             		// Habilita Interrupção Periperal 


while(1)					// Loop infinito
 {


	// === Acesso as rotinas de menu ===
	if (MENU==1){METTER();			if(Botao_Enter()){MENU=2;}}
	if (MENU==2){CONFIG_SL();		}
	if (MENU==3){CONFIG_CILINDRO();	}
	if (MENU==4){CONFIG_FILTRO();	}
	if (MENU==5){CONFIG_BRILHO();	}


//RPM = ((60*(1/((TL+(TH*256))*0.000001))) / (CILINDROS/2));


//MEDIA++;
//RPM_MEDIA = RPM_MEDIA + ((60*(1/((TL+(TH*256))*0.000001))) / (CILINDROS/2));
//RPM_MEDIA = RPM_MEDIA + (120000000/((TL+(TH*256))*CILINDROS));
RPM_MEDIA = RPM_MEDIA + (120000000/(TIMER0*CILINDROS));
if (++MEDIA==(FILTRO*10))
	{
	RPM = RPM_MEDIA / (FILTRO*10);
	RPM_MEDIA=0;
	MEDIA=0;
	}

//		_delay(1000);		// delay for 1000 instruction cycles
//		__delay_us(100);	// delay for 400 microseconds
//		__delay_ms(1);		// delay for 2 milliseconds

 }
}


// =============================================================
// =======================INTERRUPÇÕES==========================
// =============================================================

void interrupt my_isr(void)
{

GIE=0;

if(TMR1IF && TMR1IE)  // ++++++++++++++++++++++++++++++++++++++++ INTERRUPÇÃO TIMER2 ++++++++++++++++++++++++++++++++++++++++
  {TMR1IF = 0;

RPM=0; 
//TL=0;
//TH=0;
//TMR1L=0;
//TMR1H=0;
TIMER0=0;
TMR1=0;
TMR1ON=0;

  }

if(INTF && INTE)  // ++++++++++++++++++++++++++++++++++++++++ INTERRUPÇÃO RB0 ++++++++++++++++++++++++++++++++++++++++
  {INTF = 0;

TMR1ON = 0;
//TL = TMR1L;
//TH = TMR1H;
TIMER0=TMR1;
 
    TMR1IF      = 0;                            
    //TMR1H 		= 0;
    //TMR1L		= 0;
    TMR1		= 0;
TMR1ON = 1;

  }

return;
}