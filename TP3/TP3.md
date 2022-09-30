TP3: Multitarea con desalojo
========================

env_return
---------

* Al terminar un proceso su función umain() ¿dónde retoma la ejecución el kernel? Describir la secuencia de llamadas desde que termina umain() hasta que el kernel dispone del proceso.

Una vez que se termina de ejecutar `umain()` se llama a la funcion `exit()` que es un wrapper de la llamada a `sys_env_destroy(0)`, que tambien es un wrapper de `syscall(SYS_env_destroy, 1, 0, 0, 0, 0, 0)`, esto llamará a `env_destroy()` que finaliza con una llamada a `sys_yield()` que a su vez termina con la llamada a `sched_halt()`

* ¿En qué cambia la función env_destroy(). en este TP, respecto al TP anterior?

En el tp anterior simplemente se destruia el proceso y se quedaba ejecutando en el monitor. Ahora con soporte para multiples CPUs se chequea si el proceso a liberar esta corriendo en el CPU actual, si no es asi se marca como un proceso zombie el cual sera liberado posteriormente, caso contrario se libera el proceso y se llama al scheduler para que ejecute otro proceso.


sys_yield
---------

* Leer y estudiar el código del programa user/yield.c. Cambiar la función i386_init() para lanzar tres instancias de dicho programa, y mostrar y explicar la salida de make qemu-nox.


```console
[00000000] new env 00001000
[00000000] new env 00001001
[00000000] new env 00001002
Hello, I am environment 00001000.
Hello, I am environment 00001001.
Hello, I am environment 00001002.
Back in environment 00001000, iteration 0.
Back in environment 00001001, iteration 0.
Back in environment 00001002, iteration 0.
Back in environment 00001000, iteration 1.
Back in environment 00001001, iteration 1.
Back in environment 00001002, iteration 1.
Back in environment 00001000, iteration 2.
Back in environment 00001001, iteration 2.
Back in environment 00001002, iteration 2.
Back in environment 00001000, iteration 3.
Back in environment 00001001, iteration 3.
Back in environment 00001002, iteration 3.
Back in environment 00001000, iteration 4.
All done in environment 00001000.
[00001000] exiting gracefully
[00001000] free env 00001000
Back in environment 00001001, iteration 4.
All done in environment 00001001.
[00001001] exiting gracefully
[00001001] free env 00001001
Back in environment 00001002, iteration 4.
All done in environment 00001002.
[00001002] exiting gracefully
[00001002] free env 00001002
```

La salida trata de mostrar como se ejecutan los procesos segun el orden que decide el scheduler aplicando round robin. Se puede observar como hay 3 procesos y el scheduler ejecuta a todos los procesos en cada iteracion, es decir primero ejecuta al proceso 0 en la iteracion 0, luego al proceso 1 en la iteracion 0 y luego al proceso 2 en la iteracion 0 tambien; esto se repite para todas las iteraciones.



envid2env
---------

* ¿Qué ocurre en JOS si un proceso llama a sys_env_destroy(0)?

Si seguimos la ejecucion de la syscall env_destroy podemos ver que se usa `envid2env()` en donde si el id es 0 entonces se devuelve el proceso actual, por lo que se destruye el proceso actual.

dumbfork
---------

* Si una página no es modificable en el padre ¿lo es en el hijo? En otras palabras: ¿se preserva, en el hijo, el flag de solo-lectura en las páginas copiadas?

No, porque al usar `dumbfork()` estamos poniendo el permiso de escritura cuando se aloca y se mapea la pagina del hijo.

* Mostrar, con código en espacio de usuario, cómo podría dumbfork() verificar si una dirección en el padre es de solo lectura, de tal manera que pudiera pasar como tercer parámetro a duppage() un booleano llamado readonly que indicase si la página es modificable o no:
Ayuda: usar las variables globales uvpd y/o uvpt.

```c++
envid_t dumbfork(void) {
    // ...
    for (addr = UTEXT; addr < end; addr += PGSIZE) {
        bool readonly = true;
        if (uvpd[PDX(addr)] & PTE_P) {
			      pte_t pte = uvpt[PGNUM(addr)];
			      if !(pte & PTE_P)
              continue 
            if (pte & PTE_W)) // Si la pagina tiene permisos de escritura
                readonly = false;
            duppage(envid, addr, readonly);
        }
    }
    // ...
```

* Supongamos que se desea actualizar el código de duppage() para tener en cuenta el argumento readonly: si este es verdadero, la página copiada no debe ser modificable en el hijo. Es fácil hacerlo realizando una última llamada a sys_page_map() para eliminar el flag PTE_W en el hijo, cuando corresponda:
```c++
void duppage(envid_t dstenv, void *addr, bool readonly) {
    // Código original (simplificado): tres llamadas al sistema.
    sys_page_alloc(dstenv, addr, PTE_P | PTE_U | PTE_W);
    sys_page_map(dstenv, addr, 0, UTEMP, PTE_P | PTE_U | PTE_W);

    memmove(UTEMP, addr, PGSIZE);
    sys_page_unmap(0, UTEMP);

    // Código nuevo: una llamada al sistema adicional para solo-lectura.
    if (readonly) {
        sys_page_map(dstenv, addr, dstenv, addr, PTE_P | PTE_U);
    }
}
```
Esta versión del código, no obstante, incrementa las llamadas al sistema que realiza duppage() de tres, a cuatro. Se pide mostrar una versión en el que se implemente la misma funcionalidad readonly, pero sin usar en ningún caso más de tres llamadas al sistema.

```c++
void duppage(envid_t dstenv, void *addr, bool readonly) {
    // Podemos agregar un if, para ver si es necesario agregar los permisos de escritura.
    int perm =  PTE_P | PTE_U;

    if (!readonly)
        perm |= PTE_W | UTEMP;

    sys_page_alloc(dstenv, addr, PTE_P | PTE_U | PTE_W);
    sys_page_map(dstenv, addr, 0, UTEMP, perm);

    memmove(UTEMP, addr, PGSIZE);
    sys_page_unmap(0, UTEMP);
}
```



multicore_init
---------


* ¿Qué código copia, y a dónde, la siguiente línea de la función boot_aps()?
```c
 memmove(code, mpentry_start, mpentry_end - mpentry_start);
```

Copia el codigo de `kern/mpentry.s` en la direccion de code.
En este caso code es: `MPENTRY_PADDR = 0x7000`

* ¿Para qué se usa la variable global mpentry_kstack? ¿Qué ocurriría si el espacio para este stack se reservara en el archivo kern/mpentry.S, de manera similar a bootstack en el archivo kern/entry.S?

Se utiliza para poder separar correctamente el stack de cada CPU. Esto es necesario para evitar la superposicon de stacks entre los diferentes CPUs del sistema.


* En el archivo kern/mpentry.S se puede leer:
```assembly
 # We cannot use kern_pgdir yet because we are still
 # running at a low EIP.
 movl $(RELOC(entry_pgdir)), %eax
 ```
- ¿Qué valor tendrá el registro %eip cuando se ejecute esa línea? Responder con redondeo a 12 bits, justificando desde qué región de memoria se está ejecutando este código.

El registro `%eip` tendra el valor de la direccion `MPENTRY_PADDR = 0x7000`, que fue mapeado previamente.



sys_ipc_recv
---------

- Un proceso podría intentar enviar el valor númerico -E_INVAL vía ipc_send(). ¿Cómo es posible distinguir si es un error, o no?

Para distinguir si hubo error no debemos ver si src es 0 o > 0.
Si es 0 significa que ocurrio un error al recbir el mensaje, y si es mayor a 0, significa que el mensaje fue recibido correctamente, y en src se encuentra el env_id del env que envió el mensaje.

```c++
int32_t
ipc_recv(envid_t *from_env_store, ...)
	if (from_env_store)
		*from_env_store = error ? 0 : thisenv->env_ipc_from;
```

```c++
envid_t src = -1;
int r = ipc_recv(&src, 0, NULL);

if (r < 0)
  if (src == 0)
    puts("Hubo error.");
  else
    puts("Valor negativo correcto.")
```



sys_ipc_try_send
---------

- Se pide ahora explicar cómo se podría implementar una función sys_ipc_send() (con los mismos parámetros que sys_ipc_try_send()) que sea bloqueante, es decir, que si un proceso A la usa para enviar un mensaje a B, pero B no está esperando un mensaje, el proceso A sea puesto en estado ENV_NOT_RUNNABLE, y despertado una vez B llame a ipc_recv() (cuya firma no debe ser cambiada).

Es posible que surjan varias alternativas de implementación; para cada una, indicar:

* qué cambios se necesitan en struct Env para la implementación (campos nuevos, y su tipo; campos cambiados, o eliminados, si los hay)
* qué asignaciones de campos se harían en sys_ipc_send()
* qué código se añadiría en sys_ipc_recv()
Responder, para cada diseño propuesto:

* ¿existe posibilidad de deadlock?
* ¿funciona que varios procesos (A₁, A₂, …) puedan enviar a B, y quedar cada uno bloqueado mientras B no consuma su mensaje? ¿en qué orden despertarían?


La logica seria muy similar, la unica diferencia es que si el proceso B no esta esperando un mensaje, el proceso A pasa a ENV_NOT_RUNNABLE. Ademas de esto, debemos validar que ningun otro proceso previo le dejo un mensaje a B que todavia no escucho, esto lo podemos saber viendo el campo `env_ipc_from` de B, si tiene un valor negativo, sabemos que nadie le envio un mensaje por lo que podemos proceder a cargar los valores en los registros correspondientes.

```c++
static int
sys_ipc_send(envid_t envid, uint32_t value, void *srcva, unsigned perm)
{
  if (env->env_ipc_from < 0)
    panic("the proccess already has a pending message"); // Tiene un mensaje pendiente de otro env

	if (!env->env_ipc_recving) {// No esta recibieno, me bloqueo
		curenv->env_status = ENV_NOT_RUNNABLE;
       /* validar permisos */
    // Cargo los valores del mensaje en B, pero no modifico su env_ipc_recving ni su env_status
    env->env_ipc_from = curenv->env_id;
    env->env_ipc_value = value;
    env->env_ipc_perm = perm;
    return 0;
  } else {
    /* igual que antes, le paso los valores y le digo que se vuelva ENV_RUNNABLE */
  }
}
```

```c++
static int
sys_ipc_recv(void *dstva)
{
	if (curenv->env_ipc_from > 0) {// Tengo un mensaje pendiente
    struct Env *env;
    curenv->env_ipc_from = -1; // marco que ya recibi un mensaje y lo lei

    if (envid2env(curenv->env_ipc_from, &env, 0) < 0) // el proceso que envió el mensaje no existe mas
      return 0; 
  
    if (env->env_status == ENV_NOT_RUNNABLE)
      env->env_status = ENV_RUNNABLE; // el proceso que envió el mensaje pasa a estado runnable

  } else {
    /* igual que antes, me bloqueo y espero un mensaje */
  }
	return 0;
}
```


Entonces desde B, cuando se llame a `sys_ipc_recv()`, se puede recibir el mensaje enviado por A, y ademas como tenemos el `env_id` de quien nos pasa el mensaje en el campo `env_ipc_from` (en este caso A), podemos avisarle que pase a estado ENV_RUNNABLE.

- No se necesitan campos nuevos, utilizamos los existentes en la estructura `struct Env`.
- Hay que asignar ENV_NOT_RUNNABLE a `curenv->env_status` en el caso de que el env_ipc_recving sea 0.
- Un caso de deadlock se puede dar si:
A --> B (pero B no estaba esperando nada, entonces A queda bloqueado, esperando que B lea su mensaje))  
B --> A (pero A no estaba esperando nada, entonces B queda bloqueado, esperando que A lea su mensaje)
La unica forma de que se desbloqueen es que alguno lea el mensaje que le envio el otro, pero como ambos estan bloqueados no van a poder hacerlo, por lo que ambos quedaran asi.

- Con esta implementacion, no se permite que varios procesos (A1, A2, A3) envien a un mismo proceso (B), esto produciria un error. El uso correcto seria leer antes de que el proximo proceso envie un mensaje.