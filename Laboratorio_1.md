# Laboratorio : Investigación sobre niveles de optimización

https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html

| Nivel | Qué hace | Relevancia |
| :--- | :--- | :--- |
| **-O0** | Sin optimización; el objetivo es reducir el costo de compilación y que la depuración produzca los resultados esperados | Peor tiempo esperado |
| **-O1** | Reduce tamaño de código y tiempo de ejecución sin optimizaciones que tomen mucho tiempo de compilación | Mejora moderada, sin vectorización |
| **-O2** | Realiza casi todas las optimizaciones que no implican un trade-off espacio-velocidad; activa vectorización| Salto fuerte de rendimiento |
| **-O3** | Activa todo lo de `-O2` más otras banderas | EL MEJOR PARA NUESTRO PROYECTO|
| **-Ofast** | Activa todo `-O3` más `-ffast-math`, `-fallow-store-data-races` y `-fno-protect-parens`; no es compatible con cumplimiento estricto de estándares | Poco útil aquí: los datos son `int`, no `float`/`double`, así que `-ffast-math` no aplica |
| **-Os / -Oz** | Optimiza para tamaño de código, desactivando cosas como `-falign-loops`, `-fprefetch-loop-arrays` | Contraproducente para un experimento de rendimiento |
| **-Og** | Pensado para el ciclo editar-compilar-depurar, no para medir desempeño | No usar |


**¿Por qué descartamos `-Ofast`, `-Os`, `-Oz` y `-Og` sin ejecutarlos?**
No se contemplan en el diseño experimental porque sus objetivos contradicen las condiciones de la prueba: **`-Ofast`** es para punto flotante (irrelevantes al procesar enteros); **`-Os`** y **`-Oz`** sacrifican velocidad a costa de reducir el tamaño del binario; y **`-Og`** prioriza el flujo de depuración desactivando optimizaciones clave, por lo que ninguna de estas banderas es válida para medir el máximo rendimiento en un *benchmark*.

Para evaluar el impacto de las optimizaciones del compilador, generamos los ejecutables correspondientes a los niveles **-O0, -O1, -O2 y -O3** mediante los siguientes comandos:

```bash
gcc -O0 -o mm_O0 multiplicacion_matrices.c
gcc -O1 -o mm_O1 multiplicacion_matrices.c
gcc -O2 -o mm_O2 multiplicacion_matrices.c
gcc -O3 -o mm_O3 multiplicacion_matrices.c
```
**Se evalua cada ejecutable bajo el mismo tamaño de matriz de 10X10**
| Nivel | Comando de compilación | Ejecutable | Verificación de Efectividad|
| :--- | :--- | :--- | :--- | 
| **-O0** | `gcc -O0 -o mm_O0 multiplicacion_matrices.c` | `mm_O0` | Ok |
| **-O1** | `gcc -O1 -o mm_O1 multiplicacion_matrices.c` | `mm_O1` | Ok | 
| **-O2** | `gcc -O2 -o mm_O2 multiplicacion_matrices.c` | `mm_O2` | Ok |
| **-O3** | `gcc -O3 -o mm_O3 multiplicacion_matrices.c` | `mm_O3` | Ok | 

**Se evalua cada ejecutable bajo el mismo tamaño de matriz de 600X600 en un PC ASUS**
| Nivel | Comando de compilación | Ejecutable | Verificación de Eficiencia (s)|
| :--- | :--- | :--- | :--- | 
| **-O0** | `gcc -O0 -o mm_O0 multiplicacion_matrices.c` | `mm_O0` | 0.424347413 |
| **-O1** | `gcc -O1 -o mm_O1 multiplicacion_matrices.c` | `mm_O1` | 0.138967614 |
| **-O2** | `gcc -O2 -o mm_O2 multiplicacion_matrices.c` | `mm_O2` | 0.124532745 |
| **-O3** | `gcc -O3 -o mm_O3 multiplicacion_matrices.c` | `mm_O3` | 0.143792297 |

**Se evalua cada ejecutable bajo el mismo tamaño de matriz de 600X600 en un PC LENOVO**
| Nivel | Comando de compilación | Ejecutable | Verificación de Eficiencia (s)|
| :--- | :--- | :--- | :--- | 
| **-O0** | `gcc -O0 -o mm_O0 multiplicacion_matrices.c` | `mm_O0` | 0.399778806 |
| **-O1** | `gcc -O1 -o mm_O1 multiplicacion_matrices.c` | `mm_O1` | 0.110900357 |
| **-O2** | `gcc -O2 -o mm_O2 multiplicacion_matrices.c` | `mm_O2` | 0.130804155 |
| **-O3** | `gcc -O3 -o mm_O3 multiplicacion_matrices.c` | `mm_O3` | 0.129852974 |

Según la documentación de GCC, -O3 incluye todas las optimizaciones de -O2 más otras orientadas a LOOPS, como el intercambio y el desenrollado, por lo que en teoría debería ser el nivel más rápido para nuestro kernel. Sin embargo, en las pruebas con matrices de 600×600 esto no se cumplió: en el equipo ASUS el mejor tiempo fue con -O2 (0,1245 s) y en el LENOVO con -O1 (0,1109 s), y -O3 no fue el más rápido en ninguno de los dos (0,1438 s en ASUS y 0,1299 s en LENOVO). Lo que sí se observó en los dos equipos fue el salto de -O0 a -O1: el tiempo se redujo entre 3 y 3,6 veces de 0,424 s a 0,139 s en ASUS y de 0,400 s a 0,111 s en LENOVO, mientras que entre -O1, -O2 y -O3 las diferencias son pequeñas y el orden cambia de un equipo a otro. Como cada tiempo proviene del menor valor entre 5 ejecuciones, no se puede afirmar que un nivel sea mejor que otro. Suponemos que esto se debe a que el kernel está limitado por el acceso a memoria (la matriz B se recorre por columnas lo que se trata en el laboratorio 2) y no por el cálculo, por lo que las optimizaciones adicionales de -O3 no se traducen en una mejora y pueden agregar costo. En conclusión, un nivel de optimización mayor no garantiza un menor tiempo: el resultado depende del programa y del equipo, y por eso conviene medirlo con una mayor nivel de muestras en lugar de asumirlo.