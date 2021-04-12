#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#define  DEVICE_NAME "mycalc"     // Nome do dispositivo na pasta /dev/
#define  CLASS_NAME  "char"       // Tipo de driver caractere
#define  RECEIVE_MSG_LENGTH 9     // Tamanho da mensagem enviada pelo usuario
#define  SEND_MSG_LENGTH    4     // Tamanho da mensagem enviada para o usuario

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joao Felipe, Leonardo Deorce, Philipe Aguiar");
MODULE_DESCRIPTION("Calculadora");
MODULE_VERSION("1.0");

static int    majorNumber;
static char   message[SEND_MSG_LENGTH] = {0};   // Armazena a mensagem a ser lida pelo usuario
static struct class*  mycalcClass      = NULL;
static struct device* mycalcDevice     = NULL;

static DEFINE_MUTEX(mycalc_mutex);   // Macro usada para declarar um novo mutex
                                     // cria um semaforo mycalc_mutex de valor 1 (desbloqueado)

// Funcoes "prototype" para um dispositivo de tipo caractere
static int     dev_open(struct inode*, struct file*);
static int     dev_release(struct inode*, struct file*);
static ssize_t dev_read(struct file*, char*, size_t, loff_t*);
static ssize_t dev_write(struct file*, const char*, size_t, loff_t*);

// Funcoes callback associadas as operacoes implementadas
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

/** Funcao de inicializacao da LKM
 */
static int __init mycalc_init(void)
{
   printk(KERN_INFO "MyCalc: Initializing the MyCalc driver as LKM\n");

   // Inicializa o mutex
   mutex_init(&mycalc_mutex);

   // Aloca o major number para o dispositivo
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber < 0) {
      printk(KERN_ALERT "MyCalc failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "MyCalc: registered correctly with major number %d\n", majorNumber);

   // Registra a classe do dispositivo
   mycalcClass = class_create(THIS_MODULE, CLASS_NAME);
   if ( IS_ERR(mycalcClass) ) {
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(mycalcClass);
   }
   printk(KERN_INFO "MyCalc: device class registered correctly\n");

   // Registra o driver do dispositivo
   mycalcDevice = device_create(mycalcClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if ( IS_ERR(mycalcDevice) ) {
      class_destroy(mycalcClass);
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(mycalcDevice);
   }
   printk(KERN_INFO "MyCalc: device class created correctly\n");
   return 0;
}


/** Funcao de limpeza e saida da LKM
 */
static void __exit mycalc_exit(void)
{
   mutex_destroy(&mycalc_mutex);                           // destroi o mutex
   device_destroy(mycalcClass, MKDEV(majorNumber, 0));     // remove o dispositivo
   class_unregister(mycalcClass);                          // remove registro da classe do dispositivo
   class_destroy(mycalcClass);                             // remove a classe do dispositivo
   unregister_chrdev(majorNumber, DEVICE_NAME);            // remove registro do major number
   printk(KERN_INFO "MyCalc: Goodbye!\n");
}

/** Funcao de abertura do dispositivo chamada sempre que o mesmo e' aberto
 */
static int dev_open(struct inode* inodep, struct file* filep)
{
   if(mutex_trylock(&mycalc_mutex) == 0) {    // Tentativa de down no mutex
                                              // retorna 1 caso sucesso e 0 caso fracasso
      printk(KERN_ALERT "MyCalc: Device in use by another process\n");
      return -EBUSY;
   }
   printk(KERN_INFO "MyCalc: Device has been opened\n");
   return 0;
}

/** Funcao de leitura que envia ao usuario a mensagem (string) armazenada em "message"
 */
static ssize_t dev_read(struct file* filep, char* buffer, size_t len, loff_t* offset)
{
   int error_count = 0;
   
   // copy_to_user e' uma forma segura de acessar o espaco do usuario
   error_count = copy_to_user(buffer, message, SEND_MSG_LENGTH);

   if (error_count == 0) {  // error_count armazena o numero de caracteres nao enviados
      printk(KERN_INFO "MyCalc: Sent %d characters to the user\n", SEND_MSG_LENGTH);
      return 0;
   }
   else {
      printk(KERN_INFO "MyCalc: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;       // Fracasso causado por erro no endereco de destino
   }
}

/** Funcao de escrita que recebe do usuario uma string formatada corretamente, manipula
 *  seu conteudo, realiza a operacao desejada e armazena o resultado em "message"
 */
static ssize_t dev_write(struct file* filep, const char* buffer, size_t len, loff_t* offset)
{
   char str[RECEIVE_MSG_LENGTH];
   int32_t  leftNumber  = 0x00000000;
   int32_t  rightNumber = 0x00000000;
   char     op;
   int32_t  result;

   copy_from_user(str, buffer, len);

   op = str[0];     // primeiro elemento da string e' numero ASCII do simbolo do operador

   // Constroi o primeiro numero em "leftNumber" usando operacoes bitwise e shift
   leftNumber = leftNumber | (((unsigned char) str[1] & 0xFFFFFFFF) << 24);
   leftNumber = leftNumber | (((unsigned char) str[2] & 0xFFFFFFFF) << 16);
   leftNumber = leftNumber | (((unsigned char) str[3] & 0xFFFFFFFF) << 8 );
   leftNumber = leftNumber | (((unsigned char) str[4] & 0xFFFFFFFF));

   // Constroi o segundo numero em "rightNumber" usando operacoes bitwise e shift
   rightNumber = rightNumber | (((unsigned char) str[5] & 0xFFFFFFFF) << 24);
   rightNumber = rightNumber | (((unsigned char) str[6] & 0xFFFFFFFF) << 16);   
   rightNumber = rightNumber | (((unsigned char) str[7] & 0xFFFFFFFF) << 8 );
   rightNumber = rightNumber | ( (unsigned char) str[8] & 0xFFFFFFFF);

   switch(op) {
      case '+': result = leftNumber + rightNumber; break;
      case '-': result = leftNumber - rightNumber; break;
      case '*': result = leftNumber * rightNumber; break;
      case '/': result = leftNumber / rightNumber; break;
      default:  result = 0;
   }

   // Transforma o resultado em uma representacao byte por byte na string "message"
   message[0] = (unsigned char) (result >> 24) & 0xFF;
   message[1] = (unsigned char) (result >> 16) & 0xFF;
   message[2] = (unsigned char) (result >> 8 ) & 0xFF;
   message[3] = (unsigned char)  result        & 0xFF;

   printk(KERN_INFO "MyCalc: Received %zu characters from the user\n", len);

   return len;
}


/** Funcao de fechamento chamada sempre que o dispositivo e' fechado
 */
static int dev_release(struct inode* inodep, struct file* filep)
{
   mutex_unlock(&mycalc_mutex);          // Up no mutex
   printk(KERN_INFO "MyCalc: Device successfully closed\n");
   return 0;
}

// Identifica as funcoes de inicializacao e limpeza
module_init(mycalc_init);
module_exit(mycalc_exit);
