TP4: Sistema de archivos e intérprete de comandos
========================

DISKMAP
---------

* ¿Qué es super->s_nblocks?

Es la cantidad total de bloques en el disco.

* ¿Dónde y cómo se configura este bloque especial?

Se realiza en el archivo `fsformat.c` en la funcion `opendisk()`, alli se setean los atributos de la variable super (la estructura que contiene a todos los bloques), entre ellos la cantidad de bloques, el path del root y el famoso file system magic (es magico).


