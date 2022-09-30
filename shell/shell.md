# Lab: shell

### Búsqueda en $PATH
- ¿Cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?
 
execve(2) ejecuta el programa pasado por parametro como pathname, si este no esta definido entonces la syscall fallará en cambio la familia exec(3) usa el directorio actual definido en PATH en el caso en que no haya un caracter '/' en el string pasado como nombre del binario. Debido a esto la familia exec(3) es más adecuada para la implementación de bash. Tambien notar que en la familia exec(3) existen funciones que no reciben una array de strings con las variables de ambiente u otras variantes.

- ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?

Puede fallar, en tal caso se imprime el mensaje de error con el errno correspondiente, se mata el proceso hijo de sh y continua la ejecución a la espera de otro comando.
---

### Comandos built-in
¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)


pwd podría no ser built in ya que este solo nos dice el directorio actual de la shell, en cambio cd cambia el estado de la shell. pwd se hace de manera built-in por una cuestión de performance, para no tener que realizar forks ni los llamados a exec que son costosos.
---

### Variables de entorno temporarias

- ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?

Para evitar la posibilidad de que se pise el valor de una variable del proceso padre.
- En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un array y se lo coloca en el tercer argumento de una de las funciones de exec(3).


  - ¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.

  La diferencia es que cuando se utiliza el tercer parámetro éstas son las únicas variables definidas para el exec, es decir. las que se encuentran definidas previamente en el ambiente no serán utilizables. Esto se debe a que la variable `environ` se setea en cada proceso.

  - Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.

  Se podría ir haciando getenv de las variables de ambiente hasta tener todas en al array (incluyendo las nuevas). También existe la posibilidad de ir pasando por parámetro `char *envp[]` desde main pero esto no es estandar ya que no es soportado en algunos UNIX modernos (dejaria de ser portable).

---

### Procesos en segundo plano
- Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano.

Se realiza un llamado a fork en `runcmd()`, aqui si el comando es del tipo back se imprime el pid del proceso hijo se el wait utiliza el flag `WNOHANG` para que el proceso retorne inmediatamente si ningun hijo termino (todo esto en el proceso padre). En el proceso hijo se llama a `exec_cmd()` donde se chequea el tipo de comando que se esta ejecutando, si es del tipo de segundo plano se vuelve a llamar a `exec_cmd()` con el comando que se quiere ejecutar en segundo plano (llamado c en el struct de backcmd).
---

### Flujo estándar

- Investigar el significado de 2>&1, explicar cómo funciona su forma general y mostrar qué sucede con la salida de cat out.txt en el ejemplo. Luego repetirlo invertiendo el orden de las redirecciones. ¿Cambió algo?

La redirección 2>&1 es la forma correcta de redireccionar stderr hacia stdout, en principio un pensaría que 2>1 basta, pero esta redirección implica redireccionar la salida de stderr hacia el archivo '1', en cambio cuando agregamos el & estamos diciendo redireccioná hacia el file descriptor 1 (stdout).

 ```console
 $ ls -C /home /noexiste >out.txt 2>&1

$ cat out.txt
 ls: cannot access '/noexiste': No such file or directory
/home:
agustin  linuxbrew  lost+found

 ```
Si cambiamos el comando a 1>&2 estamos diciendo que redirecciones stdout a stderr, por lo que se esperaría que out.txt no tenga nada, ya que la salida es por stderr y el archivo esta recibiendo por stdout, y que lo que imprime el comando ls se muestre mediante stderr.
 ```console
 $ ls -C /home /noexiste >out.txt 1>&2
  ls: cannot access '/noexiste': No such file or directory
/home:
agustin/  linuxbrew/  lost+found/
$ cat out.txt
 ```

 Para comprobar esto podríamos redirigir stderr hacia out.txt.
 ```console
 $ ls -C /home /noexiste 2>out.txt 1>&2
$ cat out.txt
ls: cannot access '/noexiste': No such file or directory
/home:
agustin  linuxbrew  lost+found
 ```
---

### Tuberías simples (pipes)
- Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe ¿Cambia en algo? ¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con la implementación del este lab.

El exit code que setea el pipe es del proceso del ultimo proceso ejecutado (el de la derecha de todo). Aqui podemos ver como el status es sin error cuando el comando ls fallo, pero grep no fallo.

```console
$ ls /noexiste 2>&1 | grep noex
ls: cannot access '/noexiste': No such file or directory
$ echo $?
0
```

En cambio en este ejemplo el primer comando no falla y el segundo si y el exit code es de error (ya que grep no encuentra hola).

```console
$ echo hi | grep hola
$ echo $?
1
```
En cambio en la implementacion propia se obtiene lo siguiente

```console
$ $ echo hi | grep hola
$ echo $?
0
```
Si agregamos otro pipe en el medio sucede lo mismo
```console
echo hi | grep hola | grep hola
$ echo $?
0
```
Y finalmente

```console
$ ls /noexiste 2>&1 | grep noex
ls: cannot access '/noexiste': No such file or directory
$ echo $?
512
```

Por lo que la implementacion setea el exit code con el primer comando del pipe.

---

### Pseudo-variables
- Investigar al menos otras tres variables mágicas estándar, y describir su propósito. Incluir un ejemplo de su uso en bash (u otra terminal similar).


$_ expande el ultimo argumento.
```console
$echo sudo
sudo
$ $_
usage: sudo -h | -K | -k | -V
usage: sudo -v [-AknS] [-g group] [-h host] [-p prompt] [-u user]
usage: sudo -l [-AknS] [-g group] [-h host] [-p prompt] [-U user] [-u user] [command]
usage: sudo [-AbEHknPS] [-r role] [-t type] [-C num] [-g group] [-h host] [-p prompt] [-T timeout] [-u user] [VAR=value] [-i|-s] [<command>]
usage: sudo -e [-AknS] [-r role] [-t type] [-C num] [-g group] [-h host] [-p prompt] [-T timeout] [-u user] file ...
```
$1 $2 ... $n expande el argumento n del comando.
```console
set -- "arg  1" "arg  2" "arg  3"; echo $3
arg 3
```

$# expande la cantidad de argumentos pasados al comando.

```console
$ set -- "arg  1" "arg  2" "arg  3"; echo $#
3
```

---


