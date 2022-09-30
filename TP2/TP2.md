TP2: Procesos de usuario
========================

env_alloc
---------

1. ¿Qué identificadores se asignan a los primeros 5 procesos creados? (Usar base hexadecimal.)

Los identificadores de los enviroments se setean con el siguiente calculo :

```console
generation | (env_free_list - envs)
donde generation = e->env_id + (1 << 12) & ~(1 << 10 - 1)
```


Calculamos generation (se obvia el env_id ya que son los primeros procesos creados por lo que estan inicializados en 0):
```c
printf("Generation\n");
printf("Decimal: %ld\n", (1 << 12) & ~(1 << 10 - 1));
printf("Hexadecimal: %x\n", (1 << 12) & ~(1 << 10 - 1));
```

```console
Generation
Decimal: 4096
Hexadecimal: 1000
```

Generation no va a variar, ya que los procesos están (en principio) todos inicializados con id 0.
Ahora calculamos (env_free_list - envs) para cada proceso.

Para el primer proceso, vemos que env_free_list y envs apuntan a la misma direccion por lo que la resta es 0.
Para el segundo proceso, env_free_list ahora apunta hacia el primer proceso libre de envs mientras que envs sigue siendo el inicio del vector, por lo que la resta da 1.
Ya que, en env_init() se seteo que env_free_list = envs, luego para los siguientes procesos en esta resta va a ir aumentando en 1, ya que en cada proceso nuevo se actualiza el puntero de env_free_list al proximo proceso en la lista envs (linkeados tambien en env_init()), quedando: 


1. 
```console
Generation | 0
Decimal: 4096
Hexadecimal: 0x1000
```
2. 
```console
Generation | 1
Decimal: 4097
Hexadecimal: 0x1001
```
3. 
```console
Generation | 2
Deicmal: 4098
Hexadecimal: 0x1002
```
4. 
```console
Generation | 3
Decimal: 4099
Hexadecimal: 0x1003
```
5. 
```console
Generation | 4
Decimal: 4100
Hexadecimal: 0x1004
```

2. Supongamos que al arrancar el kernel se lanzan NENV procesos a ejecución. A continuación se destruye el proceso asociado a envs[630] y se lanza un proceso que cada segundo muere y se vuelve a lanzar (se destruye, y se vuelve a crear). ¿Qué identificadores tendrán esos procesos en las primeras cinco ejecuciones?

Para la primera ejecucion del proceso 630 se tendra como id a 0x1276.

Esto se calculo con la formula (de nuevo con e->env_id = 0 por ser la primera vez):

`e->env_id + (1 << 12) & ~(1 << 10 - 1) | (env_free_list - envs) = 0x1000 | 630 = 0x1276`

Sabiendo que este id fue el inicial, podemos calcular cual sera el id del segundo proceso que se ejecute en envs[630]:


- Nuevo proceso 1era ejecucion:
`0x1276 + (1 << 12) & ~(1 << 10 - 1) | 630 = 0x2276`

- Nuevo proceso 2nda ejecucion:
`0x2276 + (1 << 12) & ~(1 << 10 - 1) | 630 = 0x3276`

- Nuevo proceso 3era ejecucion:
`0x3276 + (1 << 12) & ~(1 << 10 - 1) | 630 = 0x4276`

- Nuevo proceso 4ta ejecucion:
`0x4276 + (1 << 12) & ~(1 << 10 - 1) | 630 = 0x5276`

- Nuevo proceso 5ta ejecucion:
`0x5276 + (1 << 12) & ~(1 << 10 - 1) | 630 = 0x6276`



env_init_percpu
---------------
1. ¿Cuántos bytes escribe la función lgdt, y dónde?


Escribe 48 bits (6 bytes) en el registro gdtr.


2. ¿Qué representan esos bytes?


Se escribe la direccion de memoria donde se guarda la informacion de inicio y fin de la tabla gdt. Esta contiene metadata de los segmentos del kernel y del usuario, vital para los context switch.

```c
struct Pseudodesc gdt_pd = { sizeof(gdt) - 1, (unsigned long) gdt };

lgdt(&gdt_pd);

static inline void lgdt(void *p) {
	asm volatile("lgdt (%0)" : : "r" (p));
}
```


env_pop_tf
----------

1. Dada la secuencia de instrucciones assembly en la función, describir qué contiene durante su ejecución:
* el tope de la pila justo antes popal


  Contiene a tf_regs, que es la estructura que contiene a todos los registros de propósito general.
* el tope de la pila justo antes iret


  Como previamente se popearon los valores de es y ds (junto con sus paddings de 16 bits), y luego se suman 8 bytes (2 registros de 32 bits que son los correspondientes a tf_trapno y a tf_errcode); entonces el tope de la pila tendrá tf_eip.
* el tercer elemento de la pila justo antes de iret

  Tiene a tf_eflags, ya que en el segundo elemento se tiene a tf_cs con su padding.

2. En la documentación de iret en [IA32-2A] se dice:

> If the return is to another privilege level, the IRET instruction also pops the stack pointer and SS from the stack, before resuming program execution.

¿Cómo determina la CPU (en x86) si hay un cambio de ring (nivel de privilegio)? Ayuda: Responder antes en qué lugar exacto guarda x86 el nivel de privilegio actual. ¿Cuántos bits almacenan ese privilegio?

El nivel del privilegio actual se guarda en los ultimos dos bits del campo tf_cs, para determinar el cambio de ring se compara el campo con el valor actual del registro (tf_cs y sus ultimos 2 bits).

gdb_hello
---------

1. Poner un breakpoint en env_pop_tf() y continuar la ejecución hasta allí.

```console
(gdb) b env_pop_tf
Breakpoint 1 at 0xf0102fa2: file kern/env.c, line 469.
(gdb) c
Continuing.
The target architecture is assumed to be i386
=> 0xf0102fa2 <env_pop_tf>:	endbr32 

Breakpoint 1, env_pop_tf (tf=0xf01c8000) at kern/env.c:469
469	{
```

2. En QEMU, entrar en modo monitor (Ctrl-a c), y mostrar las cinco primeras líneas del comando info registers.

```console
EAX=003bc000 EBX=f01c8000 ECX=f03bc000 EDX=00000204
ESI=00010094 EDI=00000000 EBP=f0119fd8 ESP=f0119fbc
EIP=f0103019 EFL=00000092 [--S-A--] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0010 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```

3. De vuelta a GDB, imprimir el valor del argumento tf:

```console
(gdb) p tf
$1 = (struct Trapframe *) 0xf01c8000
```

4. Imprimir, con x/Nx tf tantos enteros como haya en el struct Trapframe donde N = sizeof(Trapframe) / sizeof(int).Imprimir, con x/Nx tf tantos enteros como haya en el struct Trapframe donde N = sizeof(Trapframe) / sizeof(int).

```console
(gdb) print sizeof(struct Trapframe) / sizeof(int)
$2 = 17
(gdb) x/17x 0xf01c8000
0xf01c8000:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c8010:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c8020:	0x00000023	0x00000023	0x00000000	0x00000000
0xf01c8030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
0xf01c8040:	0x00000023
```

5. Avanzar hasta justo después del movl ...,%esp, usando si M para ejecutar tantas instrucciones como sea necesario en un solo paso:

```console
(gdb) disas
Dump of assembler code for function env_pop_tf:
=> 0xf0102fa2 <+0>:	endbr32 
   0xf0102fa6 <+4>:	push   %ebp
   0xf0102fa7 <+5>:	mov    %esp,%ebp
   0xf0102fa9 <+7>:	sub    $0xc,%esp
   0xf0102fac <+10>:	mov    0x8(%ebp),%esp
   0xf0102faf <+13>:	popa   
   0xf0102fb0 <+14>:	pop    %es
   0xf0102fb1 <+15>:	pop    %ds
   0xf0102fb2 <+16>:	add    $0x8,%esp
   0xf0102fb5 <+19>:	iret   
   0xf0102fb6 <+20>:	push   $0xf0105523
   0xf0102fbb <+25>:	push   $0x1df
   0xf0102fc0 <+30>:	push   $0xf01054c6
   0xf0102fc5 <+35>:	call   0xf01000ad <_panic>
End of assembler dump.
(gdb) si 5
=> 0xf0103026 <env_pop_tf+13>:	popa   
0xf0103026 in env_pop_tf (tf=0x0) at kern/env.c:470
470		asm volatile("\tmovl %0,%%esp\n"
```

6. Comprobar, con x/Nx $sp que los contenidos son los mismos que tf (donde N es el tamaño de tf).

```console
(gdb) x/17x $sp
0xf01c8000:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c8010:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c8020:	0x00000023	0x00000023	0x00000000	0x00000000
0xf01c8030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
0xf01c8040:	0x00000023
```

7. Describir cada uno de los valores. Para los valores no nulos, se debe indicar dónde se configuró inicialmente el valor, y qué representa.

Los primeros 8 valores son los valores de los registros de PushRegs del Trapframe (reg_edi, reg_esi, etc). Los siguientes corresponden a los registros propios del trapframe (recordar que los registros de 16 bits se completan con sus respectivos padding, es decir 2 registros de 16 completan 1 de 32, para asi dar con los 17 registros que nos indica gdb). Aclarado esto podemos ver:
* tf_es = tf_ds = tf_ss =  0x00000023 el cual es el segmento de memoria.
* trapno nulo 
* tf_err nulo 
* tf_eip = 0x00800020 instruccion a ejecutar luego del retorno del programa
* tf_cs = 0x0000001b = 3 en decimal, indicando el privilegio de ejecucion.
* tf_eflags es nulo 
* tf_esp = 0xeebfe000 posicion a la que debe volver el registro esp.


8. Continuar hasta la instrucción iret, sin llegar a ejecutarla. Mostrar en este punto, de nuevo, las cinco primeras líneas de info registers en el monitor de QEMU. Explicar los cambios producidos.

```console
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=f01c8030
EIP=f010302c EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```

9.  Ejecutar la instrucción iret. En ese momento se ha realizado el cambio de contexto y los símbolos del kernel ya no son válidos.

* imprimir el valor del contador de programa con p $pc o p $eip

```console
(gdb) p $pc
$3 = (void (*)()) 0x800020
```

* cargar los símbolos de hello con el comando add-symbol-file, así:

```console
(gdb) add-symbol-file obj/user/hello 0x800020
add symbol table from file "obj/user/hello" at
        .text_addr = 0x800020
(y or n) y
Reading symbols from obj/user/hello...
```
* volver a imprimir el valor del contador de programa

```console
$4 = (void (*)()) 0x800020 <_start>
```

Mostrar una última vez la salida de info registers en QEMU, y explicar los cambios producidos.

```console
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=eebfe000
EIP=00800020 EFL=00000002 [-------] CPL=3 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =001b 00000000 ffffffff 00cffa00 DPL=3 CS32 [-R-]
```

10. Poner un breakpoint temporal (tbreak, se aplica una sola vez) en la función syscall() y explicar qué ocurre justo tras ejecutar la instrucción int $0x30. Usar, de ser necesario, el monitor de QEMU.
   
Antes de ejecutarla
```console
(gdb) p $pc
$5 = (void (*)()) 0x800a47 <syscall+29>
```

```console
EAX=00000000 EBX=00000000 ECX=0000000d EDX=eebfde88
ESI=00000000 EDI=00000000 EBP=eebfde40 ESP=eebfde18
EIP=00800a47 EFL=00000096 [--S-AP-] CPL=3 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =001b 00000000 ffffffff 00cffa00 DPL=3 CS32 [-R-]
```

Despues de ejecutarla:

```console
(gdb) p $pc
$6 = (void (*)()) 0xe05b
```
```console
EAX=00000000 EBX=00000000 ECX=0000000d EDX=eebfde88
ESI=00000000 EDI=00000000 EBP=eebfde40 ESP=efffffe8
EIP=f0103944 EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```

Podemos ver que se realiza un context switch de modo usuario a modo kernel.

kern_idt
---------

* **¿Cómo decidir si usar TRAPHANDLER o TRAPHANDLER_NOEC? ¿Qué pasaría si se usara solamente la primera?**

Para decidir cual de las dos instrucciones usar, utilizamos el manual de  [Intel® 64 and IA-32 Architectures
Software Developer’s Manual (pagina 186)](https://pdos.csail.mit.edu/6.828/2017/readings/ia32/IA32-3A.pdf); donde para cada excepcion informa si setea o no codigo de error, donde segun la documentacion de las funciones TRAPHANDLER seteaba el codigo de error y TRAPHANDLER_NOEC no.

Si solo utilazaramos TRAPHANDLER, tendriamos inconvenientes en los casos en lo que no hay que pushear nada, generando asi un problema en los datos de nuestro Trapframe.

* **¿Qué cambia, en la invocación de handlers, el segundo parámetro (istrap) de la macro SETGATE? ¿Por qué se elegiría un comportamiento u otro durante un syscall?**


Si istrap esta activado, se trata de un trap/fault/abort, es decir que puedan ocurrir otras excepciones mientras el handler se esta ejecutando.
En caso que istrap este desactivado, no se permiten otras excepciones.

Es necesario que istrap este desactivado al tratarse de una syscall, puesto que no queremos que otras excepciones ocurran durante el llamado.

* **Leer user/softint.c y ejecutarlo con make run-softint-nox. ¿Qué interrupción trata de generar? ¿Qué interrupción se genera? Si son diferentes a la que invoca el programa… ¿cuál es el mecanismo por el que ocurrió esto, y por qué motivos? ¿Qué modificarían en JOS para cambiar este comportamiento?**

softint.c intenta generar una page-fault. Vemos que obtuvimos un "General Protection"

```console
Booting from Hard Disk..6828 decimal is 15254 octal!
Physical memory: 131072K available, base = 640K, extended = 130432K
check_page_alloc() succeeded!
check_page() succeeded!
check_kern_pgdir() succeeded!
check_page_installed_pgdir() succeeded!
[00000000] new env 00001000
Incoming TRAP frame at 0xefffffbc
TRAP frame at 0xf01c8000
  edi  0x00000000
  esi  0x00000000
  ebp  0xeebfdff0
  oesp 0xefffffdc
  ebx  0x00000000
  edx  0x00000000
  ecx  0x00000000
  eax  0x00000000
  es   0x----0023
  ds   0x----0023
  trap 0x0000000d General Protection
  err  0x00000072
  eip  0x00800037
  cs   0x----001b
  flag 0x00000082
  esp  0xeebfdfd4
  ss   0x----0023
[00001000] free env 00001000
Destroyed the only environment - nothing more to do!
Welcome to the JOS kernel monitor!
Type 'help' for a list of commands.
```

Esto ocurrio, por que Page Fault requiere permisos de kernel, al intentar ejecutarla desde el modo usuario obtenemos General Protection. Si en cambio, se intentara ejecutar Breakpoint veriamos que la salida concuerda con lo esperado, esto es porque Breakpoint requiere permisos de usuario.
Si se quisiera observar Page Fault, se deberia modificar el permiso al nivel del usuario (esto es en SETGATE poner el parametro de Descriptor Privilege Level = 3).

user_evilhello
---------

Version arriba: 

```c
sys_cputs(0xf010000c, 1);
```

Version programa:
```c
char *entry = (char *) 0xf010000c;
char first = *entry;
sys_cputs(&first, 1);
```

> ¿En qué se diferencia el código de la versión en evilhello.c mostrada arriba? ¿En qué cambia el comportamiento durante la ejecución? ¿Por qué? ¿Cuál es el mecanismo?

En el primer caso, utilizamos una system call directamente, esto se traduce en que se va a ejecutar en modo kernel.

En el segundo caso, vemos que se hace una des-referencia de una direccion del kernel (osea acceder al contenido del puntero al kernel).
Como esta des-referencia se hace en modo usuario, dado que no se utilizo una syscall, se obtiene una page fault.

En el caso uno, no esta protegido dado que se bypassea el control de privilegios acciendo con una syscall a la direccion del kernel, mientras que en el otro caso, al no utilizarla falla cuando se quiere ver el contenido del kernel desde el modo usario.

> Listar las direcciones de memoria que se acceden en ambos casos, y en qué ring se realizan. ¿Es esto un problema? ¿Por qué?

En ambos casos se quiere acceder al contenido de memoria de la direccion 0xf010000c.
En el caso uno, se accede directamente desde una syscall, es decir desde el ring del kernel (0).

En el segundo caso, se quiere acceder mediante una des-referencia, es decir desde el ring del usuario (3).