# CG_OpenGL

Renderizador 3D en **OpenGL** para cargar y visualizar modelos `.obj` con iluminación dinámica, materiales procedurales y controles interactivos de cámara.

## Descripción

Este proyecto fue desarrollado como parte de la materia de **Graficación por Computadora**.  
El objetivo principal es renderizar un modelo 3D desde un archivo OBJ utilizando OpenGL, aplicando iluminación, materiales y controles de interacción en tiempo real.

El proyecto incluye:

- Carga directa de modelos `.obj`
- Normalización automática de escala y posición del modelo
- Iluminación con dos fuentes de luz
- Luz blanca principal
- Luz azul secundaria activable
- Materiales procedurales alternables
- Cámara libre con teclado y mouse
- Tres vistas/orientaciones del modelo
- Organización modular del código

## Tecnologías utilizadas

- C++
- OpenGL
- GLFW
- GLEW
- GLM
- Visual Studio

## Características principales

### Loader OBJ

El proyecto incluye un loader propio para archivos `.obj`, capaz de leer:

- Vértices
- Normales
- Coordenadas UV
- Caras triangulares
- Caras con más de tres vértices

Además, el modelo se centra y escala automáticamente para evitar problemas de tamaño o posición al cargar diferentes archivos OBJ.

### Iluminación

La escena cuenta con dos luces:

- **Luz blanca:** permanece encendida como iluminación principal.
- **Luz azul:** puede activarse o desactivarse con el teclado.

### Materiales

Se implementaron dos materiales visuales:

- **Mármol procedural**
- **Metal azul brillante**

Estos materiales se generan desde el fragment shader usando patrones procedurales.

### Cámara

La cámara permite moverse libremente por la escena y observar el modelo desde distintos ángulos.

## Controles

| Tecla / Input | Acción |
|---|---|
| `W` | Mover cámara hacia adelante |
| `S` | Mover cámara hacia atrás |
| `A` | Mover cámara a la izquierda |
| `D` | Mover cámara a la derecha |
| `SPACE` | Subir cámara |
| `CTRL` | Bajar cámara |
| `SHIFT` | Aumentar velocidad de movimiento |
| Mouse izquierdo | Rotar cámara |
| `M` | Cambiar material |
| `L` | Encender / apagar luz azul |
| `1` | Vista frontal |
| `2` | Vista lateral izquierda |
| `3` | Vista lateral derecha |
| `ESC` | Cerrar programa |

## Estructura del proyecto

```txt
src/
│
├── buffers/
├── camera/
├── input/
├── loaders/
├── rendering/
├── scene/
├── third_party/
└── Application.cpp
