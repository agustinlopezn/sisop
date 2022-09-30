TP1: Memoria virtual en JOS
===========================

Segmentación
--------------

### Tarea: simulación de traducciones

**1. Tomar los dos accesos a memoria y decidir si producen una dirección física o bien generan un segmentation fault**


```console
ARG seed 101826
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x0000006f (decimal 111)
  Segment 0 limit                  : 18

  Segment 1 base  (grows negative) : 0x00000069 (decimal 105)
  Segment 1 limit                  : 20

Virtual Address Trace
  VA  0: 0x00000032 (decimal:   50) --> PA or segmentation violation?
  VA  1: 0x00000003 (decimal:    3) - -> PA or segmentation violation?

0x00000032 --> 110010 --> Se traduce al segmento 1 --> offset = 10010 = 18 --> PA = 0x0000005b (decimal 91)
0x00000003 --> 000011 --> Se traduce al segmento 0 --> offset = 00011 = 3 --> PA = 0x00000072 (decimal 114)
```

```console
ARG seed 102671
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x00000059 (decimal 89)
  Segment 0 limit                  : 20

  Segment 1 base  (grows negative) : 0x000000b0 (decimal 176)
  Segment 1 limit                  : 17

Virtual Address Trace
  VA  0: 0x00000033 (decimal:   51) --> PA or segmentation violation?
  VA  1: 0x00000008 (decimal:    8) --> PA or segmentation violation?
0x00000033 --> 110011 --> Se traduce al segmento 1 --> offset = 10011 = 19 --> PA = 0x000000a3 (decimal:  163)
0x00000008 --> 001000 --> Se traduce al segmento 0 --> offset = 01000 = 8 --> PA = 0x00000061 (decimal:   97)
```

**2. Explicar el razonamiento del punto anterior, indicando las cuentas realizadas**

Como el address space es de 64 las direcciones van a ser de 6 bits (2^6=64), se nos dice que se usa el bit mas significativo de la virtual address para decidir que segmento se utiliza (si el bit más significativo es 0 se usa el seg-0 y seg-1 en caso contrario). A partir de eso se determinaron a que segmentos se traducirian las direcciones virtuales brindadas por el programa. 
Si se tiene el segmento 0 se utilizan los 5 bits restantes de offset, y eso se suman a la base del segmento. Si se pasa del limite será una violación de segmento, sino esa sera la PA final. En el caso del segmento 1, como este crece de manera negativa para calcular la traduccion de la PA la cuenta se realiza sumando a la base del segmento 1 la diferencia entre la VA y el tamaño del address space (PA = BASE + (VA - AS)).

**3. Validar los resultados volviendo a correr con el flag -c**
```console
ARG seed 101826
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x0000006f (decimal 111)
  Segment 0 limit                  : 18

  Segment 1 base  (grows negative) : 0x00000069 (decimal 105)
  Segment 1 limit                  : 20

Virtual Address Trace
  VA  0: 0x00000032 (decimal:   50)  --> VALID in SEG1: 0x0000005b (decimal:   91)
  VA  1: 0x00000003 (decimal:    3) --> VALID in SEG0: 0x00000072 (decimal:  114)
```

```console
ARG seed 102671
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x00000059 (decimal 89)
  Segment 0 limit                  : 20

  Segment 1 base  (grows negative) : 0x000000b0 (decimal 176)
  Segment 1 limit                  : 17

Virtual Address Trace
  VA  0: 0x00000033 (decimal:   51) --> VALID in SEG1: 0x000000a3 (decimal:  163)
  VA  1: 0x00000008 (decimal:    8) --> VALID in SEG0: 0x00000061 (decimal:   97)
```


### Tarea: traducciones inversas

* Modificaciones
  
Corrida 1
---
```terminal
VA  0: 0x00000032 (decimal:   50) --> VALID in SEG1: 0x000000a3 (decimal:  163)
VA  1: 0x00000003 (decimal:    3) --> VALID in SEG0: 0x00000061 (decimal:   97)
```

Corrida 2
---
```terminal
VA  0: 0x00000033 (decimal:   51) --> VALID in SEG1: 0x0000005b (decimal:   91)
VA  1: 0x00000008 (decimal:    8) --> VALID in SEG0: 0x00000072 (decimal:  114)
```


> 1. Para cada corrida modificada, determinar los valores de cada segmento, y especificarlos mediante los cuatro flags: -b, -l, -B y -L. Existe la posibilidad de que no se necesiten todos los valores para lograr el objetivo, o bien que el objetivo no sea posible. Si hay más de una solución; elegir la que tenga los límites de segmentos más pequeños. En caso de que no sea posible; explicar por qué.
   
Corrida 1 
* El segmento 0 tendria que empezar en 0x0000005E (decimal 94) y limite  > 0x00000003 (decimal 3)
* El segmento 1 tendria que empezar en 0x000000B1 (decimal 177) y limite  >= 0x0000000F (decimal 14), esto sale de 177 + (50 - 64) = 163.
   
```console
python2 segmentation.py -a 64 -p 256 -s 101826 -n 2 -A 50,3 -b 94 -l 4 -B 177 -L 14 -c
```

Corrida 2
* El segmento 0 tendria que empezar en 0x0000006A (decimal 106) y limite  > 0x00000008 (decimal 8)
* El segmento 1 tendria que empezar en 0x000000AA (decimal 104) y limite  >= 0x0000000D (decimal 13), esto sale de 104 + (51 - 64) = 91.
  
```console
python2 segmentation.py -a 64 -p 256 -s 102671 -n 2 -A 51,8 -b 106 -l 9 -B 104 -L 13 -c
```

**2. En caso de que haya solución, mostrar además la corrida de la simulación que cumple con los requisitos.**


```console
ARG seed 101826
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x0000005e (decimal 94)
  Segment 0 limit                  : 4

  Segment 1 base  (grows negative) : 0x000000b1 (decimal 177)
  Segment 1 limit                  : 14

Virtual Address Trace
  VA  0: 0x00000032 (decimal:   50) --> VALID in SEG1: 0x000000a3 (decimal:  163)
  VA  1: 0x00000003 (decimal:    3) --> VALID in SEG0: 0x00000061 (decimal:   97)
```

```console
ARG seed 102671
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x0000006a (decimal 106)
  Segment 0 limit                  : 9

  Segment 1 base  (grows negative) : 0x00000068 (decimal 104)
  Segment 1 limit                  : 13

Virtual Address Trace
  VA  0: 0x00000033 (decimal:   51) --> VALID in SEG1: 0x0000005b (decimal:   91)
  VA  1: 0x00000008 (decimal:    8) --> VALID in SEG0: 0x00000072 (decimal:  114)
```

### Tarea: límites de segmentación

Utilizando un espacio de direcciones virtuales de 5-bits; y un espacio de direcciones físicas de 7-bits.

**1. ¿Cuál es el tamaño (en número de direcciones) de cada espacio (físico y virtual)?**

El address space seria de 32 direcciones y la fisica de 128 direcciones.

**2. ¿Es posible configurar la simulación de segmentación para que dos direcciones virtuales se traduzcan en la misma dirección física? Explicar, y de ser posible brindar un ejemplo de corrida de la simulación.**

Es posible si permitimos que se solapen los segmentos, En el siguiente caso se traducen las direcciones 30 (11110), que da un offset de -2 desde el inicio del segmento 1 y 0 (00000) que da 0 de offset desde la base del segmento 0, podemos comprobar que ambas se traducen a la posicion 14 de la memoria fisica, dado que el segmento 0 empieza en 14 y el segmento 1 en 16.

```terminal
python2 segmentation.py -a 32 -p 128 -s 128 -A 30,0 -c -b 14 -l 8 -B 16 -L 8
ARG seed 128
ARG address space size 32
ARG phys mem size 128

Segment register information:

  Segment 0 base  (grows positive) : 0x0000000e (decimal 14)
  Segment 0 limit                  : 8

  Segment 1 base  (grows negative) : 0x00000010 (decimal 16)
  Segment 1 limit                  : 8

Virtual Address Trace
  VA  0: 0x0000001e (decimal:   30) --> VALID in SEG1: 0x0000000e (decimal:   14)
  VA  1: 0x00000000 (decimal:    0) --> VALID in SEG0: 0x0000000e (decimal:   14)
```

**3. ¿Es posible que (aproximadamente) el 90% del espacio de direcciones virtuales esté mapeado de forma válida? Explicar, y de ser posible, dar una configuración de segmentos que de tal resultado.**
  
Es posible ya que se puede configurar que el segmento 0 empiece en 0 y tenga limite en 16 y el segmento 1 empiece en 32 y tengo el limite en 16, por lo que todas las direcciones virtuales se pueden mapear a memoria fisica sin problema (mitad irian al segmento 0 y mitad al segmento 1).

**4. ¿Es posible que (aproximadamente) el 90% del espacio de direcciones físicas esté mapeado de forma válida? Explicar, y de ser posible, dar una configuración de segmentos que de tal resultado.**

Esto no es posible, dado que, hay 128 direcciones fisicas a mapear en 32 direcciones, por lo que solo podria mapear un 25% de las direcciones de forma valida.