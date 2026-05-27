# CG_OpenGL - Cornell Box Raytracing

Proyecto de Graficación por Computadora desarrollado en C++ y OpenGL.

El proyecto implementa un visualizador/raytracer para una escena tipo Cornell Box cargada desde un archivo OBJ. Incluye renderizado rasterizado tradicional y un modo de raytracing en shader, con materiales especiales, luces, cámaras y sombras duras.

## Características principales

- Carga de modelos OBJ usando Assimp.
- Escena Cornell Box cargada desde `models/CornellBox.obj`.
- Modo rasterizado con iluminación Phong / Blinn-Phong.
- Modo raytracing en fragment shader.
- Raytracing acelerado con BVH en GPU.
- Sombras duras mediante shadow rays.
- Material metálico tipo acero satinado.
- Material transparente tipo glass con refracción, Fresnel y highlights.
- Luz principal toggleable.
- Luz secundaria tenue siempre activa.
- Tres presets de cámara.
- Helpers visuales para luces y cámaras.
- Controles interactivos.

## Tecnologías utilizadas

- C++
- OpenGL 4.3
- GLSL
- GLFW
- GLEW
- GLM
- Assimp
- Visual Studio

## Controles

| Tecla | Acción |
|---|---|
| `ESC` | Cerrar programa |
| `R` | Alternar Rasterized / Raytracing |
| `1` | Cámara preset 1 |
| `2` | Cámara preset 2 |
| `3` | Cámara preset 3 |
| `L` | Encender / apagar luz principal |
| `M` | Encender / apagar material metálico en raytracing |
| `G` | Encender / apagar material glass en raytracing |
| `H` | Mostrar / ocultar helpers de debug |
| `T` | Encender / apagar texture mapping en rasterizer |
| `B` | Alternar Phong / Blinn-Phong en rasterizer |
| `W A S D` | Mover cámara libre |
| `SPACE` | Subir cámara |
| `CTRL` | Bajar cámara |
| `SHIFT` | Aumentar velocidad de cámara |
| Mouse izquierdo | Rotar cámara |

## Modos de render

### Rasterized

Renderizado tradicional con VAO, VBO y EBO.  
Incluye iluminación Phong / Blinn-Phong y texture mapping.

### Raytracing

Renderizado por raytracing en fragment shader.  
El shader genera un rayo por pixel, intersecta la escena cargada desde OBJ y calcula iluminación, sombras duras, reflexión metálica y refracción para glass.

El raytracer usa un BVH para acelerar las intersecciones contra los triángulos del OBJ.

## Estructura general

```txt
src/
├── buffers/       # VAO, VBO, EBO
├── camera/        # Cámara
├── debug/         # Helpers visuales
├── input/         # Controles
├── loaders/       # Carga de OBJ con Assimp
├── rendering/     # Shader, Mesh, RaytracingRenderer, BVH
├── scene/         # Scene, luces, materiales, configuración
├── shaders/       # GLSL raster/debug/raytracing
└── Application.cpp