/************************  GPO Iluminacion  **********************************
ATG, 2020
******************************************************************************/

#include <GpO.h>
#define M_PI 3.14159265358979323846
// Fragment Shader


// TAMA�O y TITULO INICIAL de la VENTANA
int ANCHO = 800, ALTO = 600;  // Tama�o inicial ventana
const char* prac = "OpenGL(GpO) Iluminacion";   // Nombre de la practica (aparecera en el titulo de la ventana).
static int current_program = 0; // Esta variable lleva el seguimiento de qué programa está activo.
float az = 0.0f, el = 0.75f;  // Azimuth y elevación iniciales



///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////     CODIGO SHADERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GLSL(src) "#version 330 core\n" #src

//////////////////////////////////////////////////////////////////////////////////



/*
 *VERTICES
 */

 // VERTEX PROG 1, PARA EL FRAGMENT SKETCH
const char* vertex_prog1 = GLSL(
	layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv; //For texture coordinates
out vec3 n; // Normal transformada
out vec2 uv_frag;
out vec3 pos_escena;

uniform mat4 M;
uniform mat4 PV;

void main() {
	gl_Position = PV * M * vec4(pos, 1);
	mat3 M_adj = mat3(transpose(inverse(M)));
	n = M_adj * normalize(pos);
	pos_escena = vec3(M * vec4(pos, 1.0));
	uv_frag = uv;
}
);

// VERTEX PROG 2, PARA EL FRAMENT BLINN PHONG, GOOCH Y CELESTIAL
const char* vertex_prog2 = GLSL(
	layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;

out vec3 n;
out vec3 pos_escena;

uniform mat4 M;
uniform mat4 PV;
uniform vec3 campos;

void main() {
	gl_Position = PV * M * vec4(pos, 1);
	mat3 M_adj = mat3(transpose(inverse(M)));
	n = normalize(M_adj * normal);
	pos_escena = vec3(M * vec4(pos, 1.0));
}
);


/*
 *FRAGMENTOS
 */

 // FRAGMENTOS TOON SHADING, programa 0
const char* fragment_prog_blinn = GLSL(
	in vec3 n; // Normal transformada
	in vec3 pos_escena; //Posicion de la escena
	out vec3 col; // Color del fragmento

	uniform vec3 luz; // Dirección de la luz
	uniform vec3 campos;  // Posición de la camara
	uniform vec4 coef; // Coeficientes ambiente, difuso, especular y brillo

	void main() {
		vec3 nn = normalize(n); // Normaliza la normal
		vec3 v = normalize(campos - pos_escena); // Vector vista
		vec3 l = normalize(luz); // Normaliza la luz
		vec3 h = normalize(l + v); // Vector medio

		float difusa = max(dot(l, nn), 0.0);
		float especular = pow(max(dot(nn, h), 0.0), coef.w);

		float ilu = coef.x + coef.y * difusa + coef.z * especular; // Calculamos nuestra luz usando la difusa, especular y el coef
		col = vec3(1, 1, 1) * ilu;

		//Primer if para tener el resaltado de la silueta
		float dotProduct = abs(dot(nn, v)); // Valor absoluto para tener en cuenta ambas orientaciones
		if (dotProduct <= 0.2) {

			col = vec3(0.0, 0.0, 0.0) * ilu;
		}
		//Resto de else if para sombreado toon
		else if (dotProduct >= 0.2 && dotProduct <= 0.5) {

			col = vec3(0.2, 0.2, 0.2) * ilu;
		}
		else if (dotProduct >= 0.5 && dotProduct <= 0.7) {

			col = vec3(0.4, 0.4, 0.4) * ilu;
		}
		else if (dotProduct >= 0.7 && dotProduct <= 0.9) {

			col = vec3(0.6, 0.6, 0.6) * ilu;
		}
		else if (dotProduct >= 0.9) {

			col = vec3(0.8, 0.8, 0.8) * ilu;
		}
	}
);

// FRAGMENTOS SKETCH SHADING, programa 1
const char* fragment_prog_sketch = GLSL(
	in vec3 n;
	in vec2 uv_frag;
	in vec3 pos_escena;
	out vec4 col;

	uniform vec3 luz;
	uniform vec4 coef;
	uniform sampler2D textura; // Mapa de la textura

	void main() {
		vec3 nn = normalize(n);
		vec3 l = normalize(-luz); // Dirección de la luz
		vec3 v = normalize(-pos_escena); // Dirección de la posición de la escena
		vec3 h = normalize(l + v);

		// Componente ambiental
		vec3 ambient = coef.x * vec3(1.0, 1.0, 1.0);

		// Componente difusión
		float diff = max(dot(nn, l), 0.0);
		vec3 diffuse = coef.y * diff * vec3(1.0, 1.0, 1.0);

		// Componente especular
		float spec = pow(max(dot(nn, h), 0.0), coef.w);
		vec3 specular = coef.z * spec * vec3(1.0, 1.0, 1.0);

		//  color final
		vec3 finalColor = ambient + diffuse + specular;

		// Detección de los bordes
		vec2 tex_offset = 1.0 * vec2(1.0) / textureSize(textura, 0); // Pixel offset for texture
		float kernel[9] = float[9](
			-1.0, -1.0, -1.0,
			-1.0, 8.0, -1.0,
			-1.0, -1.0, -1.0
			);

		float edgeDetection = 0.0;
		for (int y = -1; y <= 1; ++y) {
			for (int x = -1; x <= 1; ++x) {
				vec2 offset = vec2(x, y) * tex_offset;
				vec4 current_point = texture(textura, uv_frag + offset);
				edgeDetection += kernel[(y + 1) * 3 + (x + 1)] * current_point.r; // Usamos solo el rojo
			}
		}
		edgeDetection = edgeDetection > 1.0 ? 1.0 : edgeDetection < 0.0 ? 0.0 : edgeDetection;

		// Combinamos el sombreado con los bordes detectados
		vec3 sketchColor = vec3(1.0 - edgeDetection) * finalColor;
		col = vec4(sketchColor, 1.0);
	}
);

// FRAGMENTOS GOOCH SHADING, programa 2
const char* fragment_prog_gooch = GLSL(
	in vec3 n; // Normal transformada
	in vec3 pos_escena; //Posicion de la escena
	out vec3 col; // Color del fragmento

	uniform vec3 luz; // Dirección de la luz
	uniform vec3 campos;  // Posición de la camara
	uniform vec4 coef; // Coeficientes ambiente, difuso, especular y brillo

	const vec3 coolColor = vec3(0.0, 0.0, 0.7);	//Constante de color Cool
	const vec3 warmColor = vec3(0.7, 0.7, 0.0); //Constante de color Calido

	void main() {
		vec3 nn = normalize(n);
		vec3 l = normalize(luz);
		vec3 v = normalize(campos - pos_escena);
		vec3 h = normalize(l + v);

		float difusa = max(dot(l, nn), 0.0);
		float especular = pow(max(dot(nn, h), 0.0), coef.w);

		vec3 kCool = coolColor + coef.x * difusa;
		vec3 kWarm = warmColor + coef.y * difusa;
		vec3 finalColor = mix(kCool, kWarm, difusa);

		col = finalColor + vec3(1, 1, 1) * especular * coef.z;
	}
);

// FRAGMENTOS CELESTIAL SHADING, programa 3
const char* fragment_prog_celestial = GLSL(
	in vec3 n;// Normal transformada
	in vec3 pos_escena; //Posicion de la escena
	out vec4 col;// Color del fragmento

	uniform vec3 luz; // Dirección de la luz
	uniform vec3 campos;  // Posición de la camara
	uniform vec4 coef; // Coeficientes ambiente, difuso, especular y brillo

	// Colores de celestial
	const vec3 topColor = vec3(0.7, 0.7, 1.0); // Light Sky Blue
	const vec3 bottomColor = vec3(0.2, 0.0, 0.3); // Dark Violet

	void main() {
		vec3 nn = normalize(n);
		vec3 l = normalize(luz);
		vec3 v = normalize(campos - pos_escena);

		// Gradiente de Celestial, la basamos en normal y componente
		float gradientFactor = (nn.y + 1.0) * 0.5; // Normalizamos del rango [-1, 1] al [0, 1]
		vec3 celestialColor = mix(bottomColor, topColor, gradientFactor);

		// Sombreado simple
		float dif = max(dot(nn, l), 0.0);
		vec3 diffuse = coef.y * dif * celestialColor;

		// Insertamos ambiente
		vec3 ambient = coef.x * celestialColor;

		// Color Celestial final
		col = vec4(ambient + diffuse, 1.0);
	}
);

////////////////////////////////  FIN PROGRAMAS GPU (SHADERS) //////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   RENDER CODE AND DATA
///////////////////////////////////////////////////////////////////////////////////////////////////

GLFWwindow* window;
GLuint prog[4];
objeto modelo_halo;
objeto modelo_buda;



// Dibuja objeto indexado
void dibujar_indexado(objeto obj)
{
	glBindVertexArray(obj.VAO);              // Activamos VAO asociado al objeto
	glDrawElements(GL_TRIANGLES, obj.Ni, obj.tipo_indice, (void*)0);  // Dibujar (indexado)
	glBindVertexArray(0);
}


// Variables globales
mat4 Proy, View, M;
vec3 campos = vec3(0.0f, 1.5f, 1.5f);
vec3 target = vec3(0.0f, 0.9f, 0.0f);
vec3 up = vec3(0, 1, 0);


// Compilaci�n programas a ejecutar en la tarjeta gr�fica:  vertex shader, fragment shaders
// Preparaci�n de los datos de los objetos a dibujar, envialarlos a la GPU
// Opciones generales de render de OpenGL
GLuint texID;
void init_scene()
{
	// Cargamos nuestra textura
	texID = cargar_textura("./data/canvas.jpg", GL_TEXTURE0);

	// Cargar los 2 modelos que usaremos.
	modelo_halo = cargar_modelo("./data/halo.bix");
	modelo_buda = cargar_modelo("./data/buda_n.bix");

	// Guardamos en el array de prog[] los diferentes compiladores
	prog[0] = Compile_Link_Shaders(vertex_prog2, fragment_prog_blinn); // Compile shaders BlinnPhong
	prog[1] = Compile_Link_Shaders(vertex_prog1, fragment_prog_sketch); // Compile Shaders de lápiz
	prog[2] = Compile_Link_Shaders(vertex_prog2, fragment_prog_gooch); // Compile shaders Gooch shading
	prog[3] = Compile_Link_Shaders(vertex_prog2, fragment_prog_celestial); // Compile shaders Celestial shading

	glUseProgram(prog[0]);
	glUniform1i(glGetUniformLocation(prog[1], "textura"), 0); // 0 corresponds to GL_TEXTURE0

	GLint camposLoc = glGetUniformLocation(prog[0], "campos");
	glUniform3fv(camposLoc, 1, &campos[0]);


	Proy = glm::perspective(glm::radians(75.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	View = glm::lookAt(campos, target, up);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}

void dibuja_indexado_vec4(float ambiente, const float difusa, float especular, float brillo) {
	transfer_mat4("PV", Proy * View); transfer_mat4("M", M);
	transfer_vec4("coef", vec4(ambiente, difusa, especular, brillo));

	// Ponemos el modelo Halo cuando usamos el NPR sketch
	if(current_program == 1){
		dibujar_indexado(modelo_halo);
	}
	else {
		dibujar_indexado(modelo_buda);
	}

}

// Actualizar escena: cambiar posici�n objetos, nuevos objetros, posici�n c�mara, luces, etc.
void render_scene() {
	vec3 luz = vec3(cos(el) * cos(az), sin(el), cos(el) * sin(az)); // Calcula la dirección de la luz
	glUniform3fv(glGetUniformLocation(prog[current_program], "luz"), 1, &luz[0]); // Envía la luz al shader

	vec4 coef;
	if (current_program == 1) {
		coef = vec4(0.1f, 0.6f, 0.3f, 16.0f); // Coeficientes para iluminación
		transfer_vec4("coef", coef);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texID);
	}
	if (current_program == 3) {
		vec4 coef = vec4(0.2f, 0.6f, 0.3f, 16.0f);
		transfer_vec4("coef", coef);
	}
	else {
		coef = vec4(0.1f, 0.6f, 0.3f, 16.0f); // Coeficientes para iluminación
		transfer_vec4("coef", coef);
	}

	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLint camposLoc = glGetUniformLocation(prog[current_program], "campos");
	glUniform3fv(camposLoc, 1, &campos[0]);

	float tt = (float)glfwGetTime();  // Contador de tiempo en segundos

	M = translate(vec3(0.0f, 0.0f, 0.0f)) * rotate(1 * tt, vec3(0.0f, 1.0f, 0.0f));   // Mov modelo
	if (current_program == 1) {
		//transfer_mat4("PV", Proy * View); transfer_mat4("M", M);
		//dibujar_indexado(modelo);
		dibuja_indexado_vec4(0.20f, 0.80f, 2.00f, 60.0f);
	}
	else dibuja_indexado_vec4(0.20f, 0.80f, 2.00f, 60.0f);

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PROGRAMA PRINCIPAL

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	init_GLFW();            // Inicializa lib GLFW
	window = Init_Window(prac);  // Crea ventana usando GLFW, asociada a un contexto OpenGL	X.Y
	load_Opengl();         // Carga funciones de OpenGL, comprueba versi�n.
	init_scene();          // Prepara escena

	while (!glfwWindowShouldClose(window))
	{
		render_scene();
		glfwSwapBuffers(window); glfwPollEvents();
		show_info();
	}

	glfwTerminate();

	exit(EXIT_SUCCESS);
}



/////////////////////  FUNCION PARA MOSTRAR INFO EN TITULO DE VENTANA  //////////////
void show_info()
{
	static int fps = 0;
	static double last_tt = 0;
	double elapsed, tt;
	char nombre_ventana[128];   // buffer para modificar titulo de la ventana

	fps++; tt = glfwGetTime();  // Contador de tiempo en segundos

	elapsed = (tt - last_tt);
	if (elapsed >= 0.5)  // Refrescar cada 0.5 segundo
	{
		sprintf_s(nombre_ventana, 128, "%s: %4.0f FPS @ %d x %d", prac, fps / elapsed, ANCHO, ALTO);
		glfwSetWindowTitle(window, nombre_ventana);
		last_tt = tt; fps = 0;
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////  INTERACCION  TECLADO RATON
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Callback de cambio tama�o
void ResizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	ALTO = height;
	ANCHO = width;
}

static void KeyCallback(GLFWwindow* window, int key, int code, int action, int mode) {
	switch (key) {
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, true);
		break;
	case GLFW_KEY_TAB:
		if (action == GLFW_PRESS) {
			current_program = (current_program + 1) % 4;  // Cicla entre 0, 1, 2 y 3
			glUseProgram(prog[current_program]);
			printf("Programa %d activo\n", current_program);
		}
		break;
	case GLFW_KEY_UP:
		el += 0.02;
		if (el > M_PI / 2) el = M_PI / 2;
		break;
	case GLFW_KEY_DOWN:
		el -= 0.02;
		if (el < -M_PI / 2) el = -M_PI / 2;
		break;
	case GLFW_KEY_RIGHT:
		az += 0.02;
		break;
	case GLFW_KEY_LEFT:
		az -= 0.02;
		break;
	case GLFW_KEY_KP_ADD:	// Zoom in
		campos -= vec3(0.0f, 0.0f, 0.1f);
		target -= vec3(0.0f, 0.0f, 0.1f);
		up -= vec3(0.0f, 0.0f, 0.1f);
		View = glm::lookAt(campos, target, up);
		break;
	case GLFW_KEY_KP_SUBTRACT:	// Zoom out
		campos += vec3(0.0f, 0.0f, 0.1f);
		target += vec3(0.0f, 0.0f, 0.1f);
		up += vec3(0.0f, 0.0f, 0.1f);
		View = glm::lookAt(campos, target, up);
		break;
	case GLFW_KEY_W:	// Desplazamiento hacia arriba
		target += vec3(0.0f, 0.1f, 0.0f);
		View = glm::lookAt(campos, target, up);
		break;
	case GLFW_KEY_S:	// Desplazamiento hacia derecha
		target -= vec3(0.0f, 0.1f, 0.0f);
		View = glm::lookAt(campos, target, up);
		break;
	case GLFW_KEY_A:	// Desplazamiento hacia izquierda
		campos -= vec3(0.1f, 0.0f, 0.0f);
		target -= vec3(0.1f, 0.0f, 0.0f);
		View = glm::lookAt(campos, target, up);
		break;
	case GLFW_KEY_D:	// Desplazamiento hacia derecha
		campos += vec3(0.1f, 0.0f, 0.0f);
		target += vec3(0.1f, 0.0f, 0.0f);
		View = glm::lookAt(campos, target, up);
		break;
	}
}


void asigna_funciones_callback(GLFWwindow* window)
{
	glfwSetWindowSizeCallback(window, ResizeCallback);
	glfwSetKeyCallback(window, KeyCallback);
}