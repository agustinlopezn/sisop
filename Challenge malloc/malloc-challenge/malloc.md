* ¿Qué casos patológicos presenta su implementación? Si hubiera alguna secuencia de malloc() y free() que generara desperdicio, indicarlo con algún ejemplo.

Cuando se libera la memoria de un puntero, y esto causa que el bloque quede sin mapear (ya que no habia otra región ocupada); y luego se intenta liberar de nuevo ese puntero, esto debería causar un double free, en cambio en la implementación propia esto causa un invalid pointer ya que esa dirección no se encuentra mapeada en ningún bloque (el bloque al cual pertenecia se liberó por completo).

* ¿Qué mejoras se podría realizar a la implementación? Pueden comentar qué otro tipos de estrategias se podrían utilizar para implementar la librería.

Se podría utilizar una estrategia en la cual no haya subregiones en la regiones de cada bloque y se utilizara unicamente la memoria que se desea, es decir que no haya un padding asociado al tamaño de estas subregiones. Por ejemplo si quiero alocar 9 bytes y hay un bloque de 16KiB entonces se utilizaran 3 subregiones de 4 bytes, dandonos 3 bytes extra, ahora esto empeora si se utiliza un bloque grande (si solo hay disponible de ellos) ya que las subregiones son de 8KiB dandonos 8192-9 bytes extra. Una posible implementación que surgió fue la de alocar la memoria exacta que fue pedida en el primer espacio que hubiese disponible, cuando esa memoria se libera su fusiona con la memoria libre contigua (si la hay). 

* ¿En qué difiere la estrategia utilizada, con la estrategia de binning?

Binning utiliza n listas donde n-1 es la cantidad de tamaños predeterminados que se pueden alocar y en donde la ultima lista es para los tamaños mayores a ese rango. Estas listas contienen espacios libres de memoria, esto permite encontrar facilmente bloques del tamaño exacto que se desea en un tiempo constante (o ese es el objetivo). La estrategia que se utilizó realiza una busqueda lineal por sobre todas las regiones posibles hasta encontrar la cantidad de memoria contigua que se desea, haciendo mas lento el tiempo de ejecución . Por ultimo podemos aclarar que la estrategia de binning se considera de exact-fit es decir que encuentra la memoria justa, mientras que la implementacion propia encuentra el primer espacio que se encuentre contiguo y que se mayor o igual al tamaño requerido.

 