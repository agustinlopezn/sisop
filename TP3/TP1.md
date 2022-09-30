TP1: Memoria virtual en JOS
===========================

boot_alloc_pos
--------------

**a. Un cálculo manual de la primera dirección de memoria que devolverá boot_alloc() tras el arranque. Se puede calcular a partir del binario compilado (obj/kern/kernel), usando los comandos readelf y/o nm y operaciones matemáticas.**

La salida de los programas nos indica:

```console
 111: f0118970     0 NOTYPE  GLOBAL DEFAULT    6 end
```

Si pasamos la direccion `0xf0118970` a decimal obtenemos `4027681136`.

Al ser el tamaño de pagina 4096 y mantenerse la alineacion la cuenta para la proxima direccion libre seria 

`4027681136 + 1680 = 4027682816`

Donde `1680 = 4096 - (4027681136 % 4096)`

Por lo que nos quedaria la direccion `0xf0119000`.


**b. Una sesión de GDB en la que, poniendo un breakpoint en la función boot_alloc(), se muestre el valor devuelto en esa primera llamada, usando el comando GDB finish.**

```console
gdb -q -s obj/kern/kernel -ex 'target remote 127.0.0.1:26000' -n -x .gdbinit
Reading symbols from obj/kern/kernel...
Remote debugging using 127.0.0.1:26000
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0x0000fff0 in ?? ()
(gdb) b boot_alloc
Breakpoint 1 at 0xf0100b34: file kern/pmap.c, line 86.
(gdb) continue
Continuing.
The target architecture is assumed to be i386
=> 0xf0100b34 <boot_alloc>:	push   %ebp

Breakpoint 1, boot_alloc (n=4096) at kern/pmap.c:86
86	{
(gdb) finish
Run till exit from #0  boot_alloc (n=4096) at kern/pmap.c:86
=> 0xf0102684 <mem_init+26>:	mov    %eax,0xf0118968
mem_init () at kern/pmap.c:135
135		kern_pgdir = (pde_t *) boot_alloc(PGSIZE);
Value returned is $1 = (void *) 0xf0119000
```

Se comprueba que la direccion es la correcta.


page_alloc
----------

**Responder: ¿en qué se diferencia page2pa() de page2kva()?**

Ambas reciben un puntero a un struct PageInfo, `page2pa()` devuelve la direccion fisica de la pagina mientras que `page2kva()` devuelve una direccion virtual del kernel. Tambien en su implementacion `page2pa()` simplemente realiza un cuenta matematica y la devuelve, mientras que `page2kva()` chequea que la direccion pasada sea valido, en caso contrario lanza un kernel panic.


map_region_large
----------------

Responder las siguientes dos preguntas, específicamente en el contexto de JOS:

**¿cuánta memoria se ahorró de este modo? (en KiB) ¿es una cantidad fija, o depende de la memoria física de la computadora?**


Cada page table a la que apunta una entrada de la Page directory pesa 4096 bytes, como con large pages nos ahorramos todas las page tables (son 1024, un por cada entrada de la page directory) nos ahorramos 1024 entradas * 4096 bytes/entrada = 4MiB. Esto solo en caso de que todos los mapeos solicitados esten alineados a 4Mib. Esto no depende de la memoria fisica de las computadores en particular.