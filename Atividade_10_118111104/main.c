/*
 * Atividade_10_118111104.c
 * Implemtação de um semáforo com incremento e decremento do tempo usando Interrupções/Timers.
 * Semáforo funcionando no modo automático e manual.
 * Implementação de um sensor de luminosidade
 * Implementação de sensor de pessoas e controle de luminosidade via PWM
 * Implementação de um sensor de chuva
 * Foi acrescentado um sensor de chuva, utilizado dois CIs LM555, um trabalhando como oscialdo e outro como timer.
 * A saída do sensor é inversamente propocional a quantidade de chuva, ou seja, quanto maior a quantidade de chuva,
 * menor é período t_on. 
 * O display foi atualizado para SSS1306, um dispaly oled. Para a implentação deste é necesario usar o protocolo I2C/TWI é 
 * uma biblioteca especifica, listada abaixo.
 * link da biblioteca SSD1306 -> https://github.com/efthymios-ks/AVR-SSD1306.git
 * O TBJ foi atulalizado para um MOSFET  
 * Author : jose.joseilton@ee.ufcg.edu.br
 * Matrícula : 118111104
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include "SSD1306/SSD1306.h"
#include "SSD1306/Font5x8.h"

//Definições de macros - empregadas para o trabalho com bits
#define set_bit(Y,bit_x) (Y|=(1<<bit_x)) //ativa o bit x da variável
#define clr_bit(Y,bit_x) (Y&=~(1<<bit_x)) //limpa o bit x da variável
#define tst_bit(Y,bit_x) (Y&(1<<bit_x)) //testa o bit x da variável
#define cpl_bit(Y,bit_x) (Y^=(1<<bit_x)) //troca o estado do bit x

/*************************************************************************************************************************///variáveis globais
volatile uint8_t Modo = 1; //Seleção de modo Automático ou Manual
volatile uint8_t flaq_200ms = 0;
volatile uint8_t flag_5000ms = 0;//flad de disparo da lampada
volatile uint16_t valorVerme_ms = 3000; //Vermelho
volatile uint16_t valorAmare_ms = 1000; //Amarelo
volatile uint16_t valorVerde_ms = 5000; //Verde
volatile uint16_t a = 1; // Variálvel seleção da posição para incrementar/decrementar o tempo / Cursor
volatile uint32_t tempo_1ms = 0; //Contador de tempo em ms
volatile uint32_t freq_carros = 0;//Contador de carros
volatile uint32_t carros_min = 0;//Carro/minuto
volatile uint32_t T_on = 0;//tempo on do sinal (sensor de chuva)

/*************************************************************************************************************************/
void display_nokia(uint8_t *valor)
{
	GLCD_Clear();
	unsigned char t_verd[2], t_amar[2], t_verm[2], carros_1[6], lux_LDR[5], chuva[5];
	if (*valor)
	{
		
		GLCD_Setup();//Inicializa
		GLCD_SetFont(Font5x8, 5, 8, GLCD_Overwrite);//Nome da fonte; tamanho e comprimento
			
		sprintf(t_verd, "%u", valorVerde_ms/1000);//Sinal verde
		sprintf(t_amar, "%u", valorAmare_ms/1000);//Sinal amarelo
		sprintf(t_verm, "%u", valorVerme_ms/1000);//Sinal vermelho
		sprintf(carros_1, "%u", carros_min);//Número de carros por min
		sprintf(lux_LDR, "%u", ((1023000/ADC) - 1000));//Quantidade de lux
		sprintf(chuva, "%u",(((-100/13)*T_on)+(1400/13)));//Porcentagem de chuva
			
		GLCD_GotoXY(0, 0);//Posição x(coluna),y(linha)
		
		GLCD_PrintString("Modo:");//Posição x(coluna),y(linha)
		if (Modo == 1){
			GLCD_GotoXY(35, 0);//Posição x(coluna),y(linha)
			GLCD_PrintString(" M");//Posição x(coluna),y(linha)
		}
		else
		{	
			GLCD_GotoXY(35, 0);//Posição x(coluna),y(linha)
			GLCD_PrintString(" A");//Posição x(coluna),y(linha)
		}

		GLCD_GotoXY(0, 10);//Posição x(coluna),y(linha)
		GLCD_PrintString("T. Vm");//Posição x(coluna),y(linha)
		GLCD_GotoXY(0, 25);//Posição x(coluna),y(linha)
		GLCD_PrintString("T. Am");//Posição x(coluna),y(linha)
		GLCD_GotoXY(0, 40);//Posição x(coluna),y(linha)
		GLCD_PrintString("T. Vd");//Posição x(coluna),y(linha)


		GLCD_GotoXY(35, 10);//Posição x(coluna),y(linha)
		GLCD_PrintString(t_verm);//Posição x(coluna),y(linha)

		GLCD_GotoXY(70, 10);//Posição x(coluna),y(linha)
		GLCD_PrintString(lux_LDR);//Posição x(coluna),y(linha)
		GLCD_GotoXY(90, 10);//Posição x(coluna),y(linha)
		GLCD_PrintString(" Lux");//Posição x(coluna),y(linha)
		
		GLCD_GotoXY(70, 25);//Posição x(coluna),y(linha)
		GLCD_PrintString(carros_1);//Posição x(coluna),y(linha)
		GLCD_GotoXY(90, 25);//Posição x(coluna),y(linha)
		GLCD_PrintString(" c/min");//Posição x(coluna),y(linha)
		
		GLCD_GotoXY(70, 40);//Posição x(coluna),y(linha)
		GLCD_PrintString(chuva);//Posição x(coluna),y(linha)
		GLCD_GotoXY(90, 40);//Posição x(coluna),y(linha)
		GLCD_PrintString(" %");//Posição x(coluna),y(linha)

		GLCD_GotoXY(35, 25);//Posição x(coluna),y(linha)
		GLCD_PrintString(t_amar);//Posição x(coluna),y(linha)
		
		GLCD_GotoXY(35, 40);//Posição x(coluna),y(linha)
		GLCD_PrintString(t_verd);//Posição x(coluna),y(linha)
		

		if (a==1)
		{
			GLCD_GotoXY(50, 40);//Posição x(coluna),y(linha)
			GLCD_PrintString("  ");//Escreve um texto
			GLCD_GotoXY(50, 0);//Posição x(coluna),y(linha)
			GLCD_PrintString(" <");//Escreve um texto
		}

		if (a==2)
		{
			GLCD_GotoXY(50, 0);//Posição x(coluna),y(linha)
			GLCD_PrintString("  ");//Escreve um texto
			GLCD_GotoXY(50, 10);//Posição x(coluna),y(linha)
			GLCD_PrintString(" <");//Escreve um texto
			
		}
		if (a==3)
		{
			GLCD_GotoXY(50, 10);//Posição x(coluna),y(linha)
			GLCD_PrintString("  ");//Escreve um texto
			GLCD_GotoXY(50, 25);//Posição x(coluna),y(linha)
			GLCD_PrintString(" <");//Escreve um texto
		}
		if (a==4)
		{
			GLCD_GotoXY(50, 25);//Posição x(coluna),y(linha)
			GLCD_PrintString("  ");//Escreve um texto
			GLCD_GotoXY(50, 40);//Posição x(coluna),y(linha)
			GLCD_PrintString(" <");//Escreve um texto
			
		}
		GLCD_InvertScreen();
		GLCD_Render();
	}
	flaq_200ms = 0;
}


/*************************************************************************************************************************/
void lampada(uint8_t *flag_disp_lamp){//Liga ou desliga a lampada
	if(*flag_disp_lamp){
		*flag_disp_lamp = 0;
		carros_min = (freq_carros*6);
		freq_carros = 0;
		
		if ((((-100/13)*T_on)+(1400/13))>=20)//luminária do semáforo deve ser acionada, indepentendemente do valor lux
		{
			OCR2B = 255;
		}
		else if ((((1023000/ADC) - 1000) < 300) && ((tst_bit(PIND,0))==0 || carros_min > 0))
		{
			OCR2B = 255;
		}
		else if ((((tst_bit(PIND,0))!=0 && carros_min!=0)!=1) && (((1023000/ADC) - 1000) < 300))
		{
			OCR2B = 85;

		}
		else if(((1023000/ADC) - 1000) >= 300)
		{
			OCR2B = 0;

		}

		//display_nokia(&flaq_200ms);

	}

}

/*************************************************************************************************************************/
void semaforo(){//Animação do semaforo

	const uint16_t vetor [] = {0x0f, 0x07, 0x03, 0x01, 0x100, 0xf0, 0x70, 0x30, 0x10};
	static int8_t i = 0;
	static uint32_t t_anterior_ms = 0;

	PORTB = vetor[i] & 0xff;//decodifica o valor e mostra no display, busca o valor na Tabela.
	PORTD = (vetor[i] & 0x100)>>1;//decodifica o valor e mostra no display, busca o valor na Tabela.

	if (Modo == 1)
	{
		if (i<=3){//LED verde

			if ((tempo_1ms - t_anterior_ms) >= (valorVerde_ms/4))
			{
				i++;
				t_anterior_ms += (valorVerde_ms/4);
			}
		}
		else
		{
			if (i<=4)
			{
				if ((tempo_1ms - t_anterior_ms) >= valorAmare_ms)
				{
					i++;
					t_anterior_ms += valorAmare_ms;
				}
			}
			else
			{
				if (i<=8)
				{
					if ((tempo_1ms - t_anterior_ms) >= (valorVerme_ms/4))
					{
						i++;
						t_anterior_ms += (valorVerme_ms/4);
					}
				}
				else
				{
					i=0;
					t_anterior_ms = tempo_1ms;
				}

			}
		}
	}
	else
	{
		valorVerde_ms = ((carros_min/60) + 1)*1000;//Calcula o tempo (sinal verde) para o modo manual
		valorVerme_ms = (-(carros_min/60) + 9)*1000;//Calcula o tempo (sinal vermelho) para o modo manual

		if (i<=3){//LED verde

			if ((tempo_1ms - t_anterior_ms) >= (valorVerde_ms/4))
			{
				i++;
				t_anterior_ms += (valorVerde_ms/4);
			}
		}
		else
		{
			if (i<=4)
			{
				if ((tempo_1ms - t_anterior_ms) >= valorAmare_ms)
				{
					i++;
					t_anterior_ms += valorAmare_ms;
				}
			}
			else
			{
				if (i<=8)
				{
					if ((tempo_1ms - t_anterior_ms) >= (valorVerme_ms/4))
					{
						i++;
						t_anterior_ms += (valorVerme_ms/4);
					}
				}
				else
				{
					i=0;
					t_anterior_ms = tempo_1ms;
				}

			}
		}
	}

}

/*************************************************************************************************************************/
ISR(INT0_vect)//Frequência de carros por hora 
{
	freq_carros++;
}

/*************************************************************************************************************************/
ISR(PCINT1_vect)//Captura o perído Ton do sinal PWM
{
	static uint32_t tempo_ = 0;
	if(tst_bit(PINC,6))
	{
		tempo_ = tempo_1ms;
	}
	if(!(tst_bit(PINC,6)))
	{
		T_on = tempo_1ms - tempo_;
		tempo_ = tempo_1ms;
	}
}

/*************************************************************************************************************************/
ISR(PCINT2_vect)//Menu 
{
	if (tst_bit(PIND,6))//Botão de seleção
	{
		while(tst_bit(PIND,6));
		a++;//Incrementa
		if (a > 4)
		{
			a = 1;
		}
		//display_nokia(&flaq_200ms);
	}

	else if (tst_bit(PIND,5))//Botão-
	{
		if(a == 1){
			Modo = 2;
		}//se o valor for maior igual a 2000s,  decrementa

		if((valorVerme_ms  >= 2000) && a == 2){
			valorVerme_ms  -= 1000;
		}//se o valor for maior igual a 2000s,  decrementa

		if((valorAmare_ms >= 2000) && a == 3){
			valorAmare_ms -= 1000;
		}//se o valor for maior igual a 2000s,  decrementa

		if((valorVerde_ms >= 2000) && a == 4){
			valorVerde_ms -= 1000;
		}//se o valor for maior igual a 2000s,  decrementa

		//display_nokia(&flaq_200ms);
	}
	else if (tst_bit(PIND,4))//Bota+
	{
		if(a==1){
			Modo = 1;
		}//se o valor for menor igual a 8000,  valorVerme é incrementado

		if((valorVerme_ms <= 8000) && a==2){
			valorVerme_ms +=1000;
		}//se o valor for menor igual a 8000,  valorVerme é incrementado

		if((valorAmare_ms <= 8000) && a==3){
			valorAmare_ms += 1000;
		}//se o valor for menor igual a 8000,  valorAmare é incrementado

		if((valorVerde_ms <= 8000) && a==4){
			valorVerde_ms += 1000;
		}//se o valor for menor igual a 8000,  valorVerde é incrementado
		//display_nokia(&flaq_200ms);
	}
}

/*************************************************************************************************************************/
ISR(TIMER0_COMPA_vect){//Base de tempo (1ms)
	tempo_1ms++;
	if ((tempo_1ms % 1000) == 0)
	{
		flaq_200ms = 1;
	}
	if((tempo_1ms % 5000) == 0){
		flag_5000ms = 1;
	}
}

/*************************************************************************************************************************/


int main(void)
{
	//GPIO
	DDRC = 0x3e;//PC1...PC6 como saída, os demais pinos como entrada
	DDRB = 0xff;//Todos os pinos da porta B como saídas
	DDRD = 0x88;//PD7, PD5 como saída, os demais pinos como entrada
	PORTC = 0x00;//Desabilita o pull-up
	PORTD = 0x00;//Desabilita o pull-up

	//Configurações das Interrupções
	EICRA = (1<<ISC11) | (ISC01);//Uma borda de decida em INT1 gera um pedido de interrupção
	EIMSK = (1<<INT0) | (1<<INT1);//habilita as interrupção INT0
	PCICR = (1<<PCIE2) | (1<<PCIE1); //habilita a interrupçõo PCINT2 e PCINT1
	PCMSK2 = (1<<PCINT20) | (1<<PCINT21) | (1<<PCINT22);//habilita a interrupçõo no PD4 e PD6
	PCMSK1 = (1<<PCINT14);//Habilita interrupt no pnchange14, PC6

	//Configurações dos Timers
	TCCR0A = (1<<WGM01); //habilita modo CTC do TC0
	TCCR0B = (1<<CS01) | (1<<CS00);  //liga TC0 com prescaler = 64
	OCR0A = 249; //ajusta o comparador para o TC0 contar até 249
	TIMSK0 = (1<<OCIE0A); //habilita a interrupção na igualdade de comparação com OCR0A. A
	//interrupção ocorre a cada 1ms = (64*(249+1))/16MHz

	//Configura ADC
	ADMUX = 0x40; //
	ADCSRA = 0xe7; //habilita o AD, habilita interrupção, modo de conversão contínua, prescaler = 128
	ADCSRB = 0x00; //modo de conversão contínua
	DIDR0 = 0x00;//0b00111110; //habilita pino PC0 como entrada do ADC0

	//Configura PWM
	TCCR2A = 0x23; //PWM não invertido no pino OC2B
	TCCR2B = 0x04; //liga TC0, prescaler = 256
	OCR2B = 128;

	sei(); //habilita interrupções globais, ativando o bit I do SREG

	GLCD_Setup();//Inicializa
	GLCD_SetFont(Font5x8, 5, 8, GLCD_Overwrite);//Nome da fonte; tamanho e comprimento

	while (1)
	{
		semaforo();
		display_nokia(&flaq_200ms);
		lampada(&flag_5000ms);
	}
}

