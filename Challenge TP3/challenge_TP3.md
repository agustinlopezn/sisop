IPC bloqueante
========================

Parte 1: Implementación
---------

```c++
static int
sys_ipc_send(envid_t envid, uint32_t value, void *srcva, unsigned perm)
{
  if (env->env_ipc_from < 0)
    panic("the proccess already has a pending message"); // Tiene un mensaje pendiente de otro env

	if (!env->env_ipc_recving) {// No esta recibiendo, me bloqueo
		curenv->env_status = ENV_NOT_RUNNABLE;
       /* validar permisos */
    // Cargo los valores del mensaje en B, pero no modifico su env_ipc_recving ni su env_status
    env->env_ipc_from = curenv->env_id;
    env->env_ipc_value = value;
    env->env_ipc_perm = perm;
  } else {
    /* igual que antes, le paso los valores y le digo que se vuelva ENV_RUNNABLE */
    env->env_ipc_recving = 0;
    env->env_ipc_from = curenv->env_id;
    env->env_ipc_value = value;
    env->env_ipc_perm = perm;
    env->env_status = ENV_RUNNABLE;
  }
  return 0;
}
```

```c++
static int
sys_ipc_recv(void *dstva)
{
  if ((uintptr_t) dstva < UTOP && (uint32_t) dstva % PGSIZE != 0)
  return -E_INVAL;
	if (curenv->env_ipc_from > 0) {// Tengo un mensaje pendiente
    struct Env *env;
    curenv->env_ipc_from = -1; // marco que ya recibi un mensaje y lo lei

    if (envid2env(curenv->env_ipc_from, &env, 0) < 0) // el proceso que envió el mensaje no existe mas
      return 0; 
  
    if (env->env_status == ENV_NOT_RUNNABLE)
      env->env_status = ENV_RUNNABLE; // el proceso que envió el mensaje pasa a estado runnable

  } else {
    /* Codigo original, me bloqueo y espero un mensaje */
	curenv->env_status = ENV_NOT_RUNNABLE;
	curenv->env_ipc_recving = 1;
	curenv->env_ipc_dstva = dstva;
  }
	return 0;
}
```

Parte 2: Análisis
---------

Comparar ambas implementaciones respondiendo las preguntas anteriores y las siguientes con el mayor detalle posible. Pueden ser muy útiles diagramas de secuencia y de estados. Incluir este análisis en un archivo a parte ipc.md.

* ¿Qué implementación es más efeciente en cuanto al uso de los recursos del sistema? (por ejemplo, cantidad de context switch realizados)

La original es más eficiente en cuanto a la cantidad de context switch, ya que recv y send, en la nueva implementacion, cambian los estados de los procesos de RUNNABLE a NOT_RUNNABLE causando mas context switches.

* ¿Cuáles fueron las nuevas estructuras de datos utilizadas? ¿Por qué? ¿Qué otras opciones contemplaron?

Se utilizaron las estructuras de datos existentes en el TP3. Se contemplo tener una estructura extra para tener una cola de mensajes y que se pueda recibir varios mensajes (solo queda el primer mensaje sin leer, el resto no se envia).

* ¿Existe algún escenario o caso de uso en el cual se desee tener una implementación no bloqueante de alguna de las dos syscalls? Ejemplificar, y de ser posible implementar programas de usuario que lo muestren.

Una API seria un ejemplo en el que seria no deseable que el send sea bloqueante, ya que no se podria seguir enviando mensajes de respuesta hasta que no se lean. Para el receive tenemos otro ejemplo que podria ser el cliente de un juego, en el que se reciben mensajes del servidor y se tienen que renderizar los graficos, si fuese bloqueante y no se recibe un mensaje el juego se quedaria trabado.

