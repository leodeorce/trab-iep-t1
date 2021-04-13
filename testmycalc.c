#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define SEND_BUFFER_LENGTH     9
#define RECEIVE_BUFFER_LENGTH  4

unsigned char bytes[4];

char* converter_para_bytes(int32_t num)
{
   bytes[0] = (num >> 24) & 0xFF;
   bytes[1] = (num >> 16) & 0xFF;
   bytes[2] = (num >> 8 ) & 0xFF;
   bytes[3] =  num        & 0xFF;
}

void print_menu(){
   
   printf("\nComo utilizar esta calculadora:\n");
   printf("   1. Digite o operador correspondente a operacao que deseja de acordo com a tabela abaixo;\n");
   printf("   2. Digite o numero que corresponde ao lado esquerdo da operacao;\n");
   printf("   3. Digite o numero que corresponde ao lado direito da operacao;\n");

   printf("\n");
   printf("   +\tSoma\n");
   printf("   -\tSubtracao\n");
   printf("   *\tMultiplicacao\n");
   printf("   /\tDivisao\n");
   printf("\n");   
}

int verifica_operador(char operador){
   int sucesso = 1;
   if(operador != '+' && operador != '-' && operador != '*' && operador != '/'){
      sucesso = 0;
   }
   if(!sucesso){
      putchar('\n');
      system("clear -x");
      printf("Favor Inserir corretamente os dados, tente novamente:\n");
      getchar();
   } 
   return sucesso;
}

int verifica_operacao_valida(char operador, int primNumero, int segNumero){
   if(segNumero == 0 && operador == '/'){
      putchar('\n');
      system("clear -x");
      printf("Divisão por zero não é aceita.\n");
      printf("Favor Inserir corretamente os dados, tente novamente:\n");
      getchar();
      return 0;
   }
   return 1;
}

int main()
{
   int      write_return;
   int      read_return;
   int      open_return;
   char     operador;
   int      sucesso = 0;
   
   int32_t  primNumero = 0;
   int32_t  segNumero = 0;
   unsigned char stringEnviar[SEND_BUFFER_LENGTH];
   unsigned char stringReceber[RECEIVE_BUFFER_LENGTH];

   printf("\nIniciando calculadora...\n");

   open_return = open("/dev/mycalc", O_RDWR);       // Abre dispositivo com permissao de leitura e escrita
   if (open_return < 0) {
      perror("\nFalha ao abrir calculadora");
      return errno;
   }
   
   while(!sucesso){
      print_menu();
      printf("%*s", 17, "Operacao: ");
      if (scanf(" %c", &operador) != 1) {
         continue;
      } else {
         sucesso = verifica_operador(operador);
         if(!sucesso) continue;
      }
      printf("%*s", 17, "Primeiro numero: ");
      if (scanf("%d", &primNumero) != 1){
         sucesso = 0;
         continue;
      }
      printf("%*s", 17, "Segundo numero: ");
      if (scanf("%d", &segNumero) != 1)  {
         sucesso = 0;
         continue;
      }
      sucesso = verifica_operacao_valida(operador, primNumero, segNumero);      
   }

   // String de envio inicia com o operador escolhido
   stringEnviar[0] = operador;

   converter_para_bytes(primNumero);

   // Concatena string de envio com bytes do primeiro inteiro
   for(int i = 0; i < 4; i++)
      stringEnviar[i + 1] = bytes[i];

   converter_para_bytes(segNumero);

   // Concatena string de envio com bytes do segundo inteiro
   for(int i = 0; i < 4; i++)
      stringEnviar[i + 1 + 4] = bytes[i];

   printf("\nEnviando operacao para a calculadora...\n");

   write_return = write(open_return, stringEnviar, SEND_BUFFER_LENGTH);    // Envia a string para a LKM
   if (write_return < 0) {
      perror("Falha ao enviar operacao para a calculadora");
      return errno;
   }

   printf("Pressione ENTER para ler a resposta da calculadora...\n");
   char c = getchar();
   do { c = getchar(); } while(c != '\n' && c != 13);  // Sai apenas com \n ou \r

   printf("Lendo resposta da calculadora...\n");

   read_return = read(open_return, stringReceber, RECEIVE_BUFFER_LENGTH);  // Le a resposta da LKM
   if (read_return < 0) {
      perror("Falha ao ler resposta da calculadora");
      return errno;
   }

   int32_t resultado = 0x00000000;

   // Constroi novamente o valor do resultado a partir da string lida
   resultado = resultado | (((unsigned char) stringReceber[0] & 0xFFFFFFFF) << 24);
   resultado = resultado | (((unsigned char) stringReceber[1] & 0xFFFFFFFF) << 16);
   resultado = resultado | (((unsigned char) stringReceber[2] & 0xFFFFFFFF) << 8);
   resultado = resultado | (((unsigned char) stringReceber[3] & 0xFFFFFFFF));

   printf("\nResultado: %d\n", resultado);
   printf("\nFinalizando calculadora...\n\n");

   return 0;
}
