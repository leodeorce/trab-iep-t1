Integrantes: 
João Felipe Gobeti Calenzani
Leonardo Deorce Lima de Oliveira
Philipe Aguiar Marques

Para compilar o módulo mycalc, realize os seguintes comandos em seu terminal:

$ sudo make

verificando a existência do arquivo .ko:
$ ls -l *.ko

inserindo o módulo no kernel:
$ sudo insmod mycalc.ko

Caso queira verificar se o módulo foi inserido corretamente:
$ lsmod

e procure o módulo de nome "mycalc". Alternativamente, podemos usar:

$ lsmod | grep mycalc

Dessa forma o terminal deve mostrar o módulo inserido.

Para verificar a existência do arquivo do dispositivo na pasta /dev:

$ ls /dev/mycalc

O Terminal deve retornar o nome do módulo, comprovando a existência do arquivo como driver.

Para testar o funcionamento do driver, basta utilizar o testmycalc, que já foi compilado no make:

$ sudo ./testmycalc

O programa pedirá para que seja inserido um operador ('+', '-', '*' ou '/'), e dois números inteiros.
Após isso, será feita a comunicação com o driver que retornará o resultado da operação.
Caso a entrada esteja formatada de forma indesejável, ou seja, com um operador diferente dos possíveis
operadores mencionados ou números diferentes de inteiro, o programa imprime um aviso e,
em seguida, imprime um novo menu.
Exemplo de saída do programa:

"
Como utilizar esta calculadora:
   1. Digite o operador correspondente a operacao que deseja de acordo com a tabela abaixo;
   2. Digite o numero que corresponde ao lado esquerdo da operacao;
   3. Digite o numero que corresponde ao lado direito da operacao;

   +    Soma
   -    Subtracao
   *    Multiplicacao
   /    Divisao

       Operacao: *
Primeiro numero: 89
 Segundo numero: -1

Enviando operacao para a calculadora...
Pressione ENTER para ler a resposta da calculadora...

Lendo resposta da calculadora...

Resultado: -89

Finalizando calculadora...
"

Caso queira verificar as mensagens printadas pelo driver:

$ dmesg

Exemplo de saída do dmesg:
"
[15069.933976] MyCalc: Device has been opened
[15080.457739] MyCalc: Received 9 characters from the user
[15080.838123] MyCalc: Sent 4 characters to the user
[15080.838245] MyCalc: Device successfully closed
"

Lembrando que o dmesg exibe mensagens referentes a diversos drivers. Caso queira ver apenas mensagens do mycalc:

$ dmesg | grep MyCalc

Caso queira remover o driver do kernel:

$ sudo rmmod mycalc