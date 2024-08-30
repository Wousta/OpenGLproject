

//####include <GpO.h>
#include "GpO.h"

#define STB_IMAGE_IMPLEMENTATION
//####include <stb\stb_image.h>
#include <stb_image.h>
#include <string>
#include <vector>

////////////////////   

extern int ANCHO, ALTO;


// Funciones inicialización librerias, ventanas, OpenGL
void init_GLFW(void)
{
	if (!glfwInit())
	{
		fprintf(stdout, "No se inicializo libreria GLFW\n");
		exit(EXIT_FAILURE);
	}
	int Major, Minor, Rev;
	glfwGetVersion(&Major, &Minor, &Rev);
	printf("Libreria GLFW (ver. %d.%d.%d) inicializada\n", Major, Minor, Rev);
}


GLFWwindow*  Init_Window(const char* nombre)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(ANCHO, ALTO, nombre, NULL, NULL);
	if (window == NULL)
	{
		fprintf(stdout, "Fallo al crear ventana GLFW con OpenGL context %d.%d\n", OPENGL_MAJOR, OPENGL_MINOR);
		glfwTerminate();
		exit(EXIT_FAILURE);
		return NULL;
	}
	
	fprintf(stdout, "Ventana GLFW creada con contexto OpenGL %d.%d\n", OPENGL_MAJOR, OPENGL_MINOR);
	glfwMakeContextCurrent(window);
		
	asigna_funciones_callback(window);

	return window;
}


void load_Opengl(void)
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		fprintf(stdout, "Fallo al cargar funciones de OpenGL con GLAD\n");
		exit(EXIT_FAILURE);
	}	
	fprintf(stdout, "OpenGL Version: %s\n",glGetString(GL_VERSION));
	glViewport(0, 0, ANCHO, ALTO);

	printf("---------------------------------------------\n");
	return;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CARGA, COMPILACION Y LINKADO DE LOS SHADERS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char* leer_codigo_de_fichero(const char* fich)
{
	FILE* fid;
	fopen_s(&fid, fich, "rb");  if (fid == NULL) return NULL;

	fseek(fid, 0, SEEK_END);  long nbytes = ftell(fid);
	fprintf(stdout, "Leyendo codigo de %s (%d bytes)\n", fich, nbytes);

	char* buf = new char[nbytes + 1];
	fseek(fid, 0, SEEK_SET);
	fread(buf, 1, nbytes, fid);
	buf[nbytes] = 0;
	fclose(fid);

//	for (int k = 0; k < nbytes; k++) fprintf(stdout,"%c", buf[k]);
//	fprintf(stdout, "\n ------------------------\n");

	return buf;
}

GLuint compilar_shader(const char* Shader_source, GLuint type)
{
	//printf("--------------------------------\n");
	switch (type)
	{
	case GL_VERTEX_SHADER: printf("Compilando Vertex Shader :: "); break;
	case GL_FRAGMENT_SHADER: printf("Compilando Fragment Shader :: "); break;
	case GL_GEOMETRY_SHADER: printf("Compilando Geometric Shader :: "); break;
	}
		
  GLuint ShaderID = glCreateShader(type);  // Create shader object

  glShaderSource(ShaderID, 1, &Shader_source , NULL);    // Compile Shader
  glCompileShader(ShaderID);

  GLint Result = GL_FALSE;
  int InfoLogLength; char error[512]; 

  glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &Result);
  if (Result==GL_TRUE) fprintf(stdout,"Sin errores\n");  
  else 
  {
   fprintf(stdout,"ERRORES\n");  
   glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
   if(InfoLogLength>512) InfoLogLength=512;
   glGetShaderInfoLog(ShaderID, InfoLogLength, NULL, error);
   fprintf(stdout,"\n%s\n", error);

  }
 printf("----------------------------------------------\n");
 return ShaderID;
}

void check_errores_programa(GLuint id)
{
 GLint Result = GL_FALSE;
 int InfoLogLength;
 char error[512]; 

 printf("Resultados del linker (GPU): ");
 glGetProgramiv(id, GL_LINK_STATUS, &Result);
 if (Result==GL_TRUE) fprintf(stdout,"Sin errores\n"); // Compiled OK
 else 
	{
     fprintf(stdout,"ERRORES\n");  
     glGetProgramiv(id, GL_INFO_LOG_LENGTH, &InfoLogLength);
	 if(InfoLogLength<1) InfoLogLength=1; if(InfoLogLength>512) InfoLogLength=512;
     glGetProgramInfoLog(id, InfoLogLength, NULL, error);
     fprintf(stdout, "\n%s\n",error);
	 glfwTerminate();
	}
 printf("---------------------------------------------\n");
}


GLuint Compile_Link_Shaders(const char* vertexShader_source,const char*fragmentShader_source)
{
	// Compile Shaders
	GLuint VertexShaderID = compilar_shader(vertexShader_source, GL_VERTEX_SHADER);
	GLuint FragmentShaderID = compilar_shader(fragmentShader_source,GL_FRAGMENT_SHADER);

    // Link the shaders in the final program
 

	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);

	glLinkProgram(ProgramID);
	check_errores_programa(ProgramID);

	// Limpieza final
	glDetachShader(ProgramID, VertexShaderID);  glDeleteShader(VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);  glDeleteShader(FragmentShaderID);

	

    return ProgramID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////  AUXILIARES 
/////////////////////////////////////////////////////////////////////////////////////////////////////////

GLuint cargar_textura(const char * imagepath, GLuint tex_unit)
{
  stbi_set_flip_vertically_on_load(true);

  int width, height,nrChannels;
  unsigned char* data = stbi_load(imagepath, &width, &height,&nrChannels,0);

  if (data == NULL)
  {
	  fprintf(stdout, "Error al cargar imagen: existe el fichero %s?\n",imagepath);
	  glfwTerminate();
	  return 0;
  }

  glActiveTexture(tex_unit);   //glBindTexture(GL_TEXTURE_2D, 0); 
	
	GLuint textureID;
	glGenTextures(1, &textureID);             // Crear objeto textura
	glBindTexture(GL_TEXTURE_2D, textureID);  // "Bind" la textura creada
	
	//printf("%d %d data %8X\n", width, height, data);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);  //Pasa datos a GPU

	glGenerateMipmap(GL_TEXTURE_2D);
	
	stbi_image_free(data); 

	// Opciones de muestreo, magnificación, coordenadas fuera del borde, etc.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	

	//glBindTexture(GL_TEXTURE_2D, 0); 
	// DEvolvemos ID de la textura creada y cargada con la imagen
	return textureID;
}



GLuint cargar_cube_map(const char * imagepath, GLuint tex_unit)
{
	GLuint textureID;
	char fich[128];
	//const char* suf[6];
	//suf[0] = "posx"; suf[1] = "negx";
	//suf[2] = "posy"; suf[3] = "negy";
	//suf[4] = "posz"; suf[5] = "negz";

	char suf[6][5] = { "posx", "negx", "posy", "negy", "posz", "negz" };


	glActiveTexture(tex_unit);	//glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &textureID); glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	
	unsigned char *data;  int width, height, nrChannels;

	//stbi_set_flip_vertically_on_load(true);  // AQUI PARECE QUE NO ES NECESARIO PERO PUEDE QUE OCASIONALMENTE SI

	printf("CARGANDO CUBE_MAP de %s_xxx.jpg\n", imagepath);
	for (int k = 0; k < 6; k++)
	{
	 sprintf_s(fich, 128, "%s_%s.jpg", imagepath, suf[k]); //printf("CUBE_MAP: %s\n", fich);
	 data = stbi_load(fich, &width, &height, &nrChannels, 0);
	 if (data == NULL)
	 {
		 fprintf(stdout, "Error al cargar imagen: existe el fichero %s?\n", fich);
		 stbi_image_free(data);
		 glfwTerminate();
		 return 0;
	 }
	 glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+k, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	 stbi_image_free(data);
	}

	/*sprintf_s(fich,128,"%s_%s.jpg", imagepath, "posx"); printf("CUBE_MAP: %s\n", fich);
	data = stbi_load(fich, &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	
	sprintf_s(fich, 128, "%s_%s.jpg", imagepath, "negx"); printf("CUBE_MAP: %s\n", fich);
	data = stbi_load(fich, &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	
	sprintf_s(fich, 128, "%s_%s.jpg", imagepath, "posy"); printf("CUBE_MAP: %s\n", fich);
	data = stbi_load(fich, &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	
	sprintf_s(fich, 128, "%s_%s.jpg", imagepath, "negy"); printf("CUBE_MAP: %s\n", fich);
	data = stbi_load(fich, &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	sprintf_s(fich, 128, "%s_%s.jpg", imagepath, "posz"); printf("CUBE_MAP: %s\n", fich);
	data = stbi_load(fich, &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	
	sprintf_s(fich, 128, "%s_%s.jpg", imagepath, "negz"); printf("CUBE_MAP: %s\n", fich);
	data = stbi_load(fich, &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);*/

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);



	return textureID;
}


objeto cargar_modelo(char* fichero)
{
	objeto obj;
	GLuint VAO;
	GLuint buffer,i_buffer;

	GLuint N_vertices, N_caras, N_indices; 

	unsigned char *vertex_data; 
	unsigned char *indices; 


	 FILE* fid;
     fopen_s(&fid,fichero,"rb");

	 if (fid==NULL) { 
		 printf("Error al leer datos. Existe el fichero %s?\n",fichero); 
		 obj.VAO=0; obj.Ni=0; obj.tipo_indice=0;
		 glfwTerminate();
	     return obj;
	 }

	 fread((void*)&N_caras,4,1,fid);    
	 fread((void*)&N_indices,4,1,fid); 
	 //N_indices=3*N_caras; 
	 fread((void*)&N_vertices,4,1,fid); 

	 fseek(fid,0,SEEK_END);
	 unsigned int s_fichero=ftell(fid);


	 GLuint tipo = GL_UNSIGNED_INT; unsigned char s_index = 4;
	 if (N_vertices <= 65536) { tipo = GL_UNSIGNED_SHORT; s_index = 2; }
	 if (N_vertices <= 256)   { tipo = GL_UNSIGNED_BYTE; s_index = 1; }

	 fseek(fid,12,SEEK_SET);


	 GLuint bytes_indices=N_indices*s_index;
	 GLuint bytes_data = s_fichero-12-bytes_indices;
	 GLuint datos_per_vertex=((bytes_data/4)/N_vertices);

	 printf("Leyendo modelo de %s: (%d bytes)\n",fichero,s_fichero);
	 printf("%d vertices, %d triangulos. Lista de %d indices\n",N_vertices,N_caras,N_indices);
	// printf("%d vertices, %d triangulos\n",N_vertices,N_caras);
	 printf("Indices guardados en enteros de %d bytes\n",s_index);
	 printf("%d datos por vertice\n",datos_per_vertex);

	 obj.Ni=N_indices;
	 obj.Nv=N_vertices;
	 obj.Nt=N_caras;
	 obj.tipo_indice=tipo; 

	 vertex_data=(unsigned char*)malloc(N_vertices*datos_per_vertex*4); 
	 if (vertex_data==NULL) printf("ptr NULL\n"); 
	 indices=(unsigned char*)malloc(N_indices*s_index); 
	 if (indices ==NULL) printf("ptrindices NULL\n"); 

     fread((void*)vertex_data,4,datos_per_vertex*N_vertices,fid);
     fread((void*)indices,s_index,N_indices,fid);

	 fclose(fid);


    glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, N_vertices*datos_per_vertex*4, vertex_data, GL_STATIC_DRAW);

	// Defino 1er argumento (atributo 0) del vertex shader (siempre XYZ)
	glEnableVertexAttribArray(0); 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, datos_per_vertex*sizeof(float), 0);

	switch (datos_per_vertex)
	{
	case 3:   break;
	case 5:   // 2º atributo = UV
       	glEnableVertexAttribArray(1);
	    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, datos_per_vertex*sizeof(float), (void*)(3*sizeof(float)));
		break;
	case 6:   // 2º atributo = (nx,ny,nz)
       	glEnableVertexAttribArray(1);
	    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, datos_per_vertex*sizeof(float), (void*)(3*sizeof(float)));
		break;
	case 8:   // 2º atributo = UV, 3º atributo = (nx,ny,nz)
       	glEnableVertexAttribArray(1);
	    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, datos_per_vertex*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(2);
	    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, datos_per_vertex*sizeof(float), (void*)(5*sizeof(float)));
		break;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);  // Asignados atributos, podemos desconectar BUFFER

	glGenBuffers(1, &i_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, N_indices*s_index,indices, GL_STATIC_DRAW);
	
		
	glBindVertexArray(0);  //Cerramos Vertex Array con todo lidto para ser pintado

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	obj.VAO=VAO; 
	

	//GLuint k;
	//for (k=0;k<12;k++) printf("%.1f ",vertex_data[k]);
	//for (k=0;k<12;k++) printf("%1d ",indices[k]);

	// Una vez transferido datos liberamos memoria en CPU
	free((void*)vertex_data);
	free((void*)indices);


	return obj;

}

/*
 * Los cuatro siguientes son intentos fallidos de implementar la carda de modelos .obj
 */
objeto cargar_modelo1(char* fichero)
{
    objeto obj;
    GLuint VAO;
    GLuint buffer, i_buffer;

    std::vector<float> vertices;
    std::vector<float> texcoords;
    std::vector<float> normals;
    std::vector<unsigned int> indices;

    FILE* fid;
    fopen_s(&fid, fichero, "r");

    if (fid == NULL) {
        printf("Error al leer datos. Existe el fichero %s?\n", fichero);
        obj.VAO = 0;
        obj.Ni = 0;
        obj.tipo_indice = 0;
        glfwTerminate();
        return obj;
    }

    char line[128];
    while (fgets(line, sizeof(line), fid)) {
        if (strncmp(line, "v ", 2) == 0) {
            float x, y, z;
            sscanf(line + 2, "%f %f %f", &x, &y, &z);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        } else if (strncmp(line, "vt ", 3) == 0) {
            float u, v;
            sscanf(line + 3, "%f %f", &u, &v);
            texcoords.push_back(u);
            texcoords.push_back(v);
        } else if (strncmp(line, "vn ", 3) == 0) {
            float nx, ny, nz;
            sscanf(line + 3, "%f %f %f", &nx, &ny, &nz);
            normals.push_back(nx);
            normals.push_back(ny);
            normals.push_back(nz);
        } else if (strncmp(line, "f ", 2) == 0) {
            unsigned int v[3], vt[3], vn[3];
            int matches = sscanf(line + 2, "%u/%u/%u %u/%u/%u %u/%u/%u", &v[0], &vt[0], &vn[0], &v[1], &vt[1], &vn[1], &v[2], &vt[2], &vn[2]);
        	//int matches = fscanf(fid, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &v[0], &vt[0], &vn[0], &v[1], &vt[1], &vn[1], &v[2], &vt[2], &vn[2]);
            if (matches == 9) {
            	//printf("matches X.\n");
                for (int i = 0; i < 3; ++i) {
                    indices.push_back(v[i] - 1);
                }
            } else {
                matches = sscanf(line + 2, "%u/%u %u/%u %u/%u",
                    &v[0], &vt[0], &v[1], &vt[1], &v[2], &vt[2]);
                if (matches == 6) {
                    for (int i = 0; i < 3; ++i) {
                        indices.push_back(v[i] - 1);
                    }
                } else {
                    matches = sscanf(line + 2, "%u//%u %u//%u %u//%u",
                        &v[0], &vn[0], &v[1], &vn[1], &v[2], &vn[2]);
                    if (matches == 6) {
                        for (int i = 0; i < 3; ++i) {
                            indices.push_back(v[i] - 1);
                        }
                    } else {
                        matches = sscanf(line + 2, "%u %u %u",
                            &v[0], &v[1], &v[2]);
                        if (matches == 3) {
                            for (int i = 0; i < 3; ++i) {
                                indices.push_back(v[i] - 1);
                            }
                        } else {
                            printf("Formato de cara no soportado.\n");
                            fclose(fid);
                            obj.VAO = 0;
                            obj.Ni = 0;
                            obj.tipo_indice = 0;
                            glfwTerminate();
                            return obj;
                        }
                    }
                }
            }
        }
    }

    fclose(fid);

    obj.Ni = indices.size();
    obj.Nv = vertices.size() / 3;
    obj.Nt = obj.Ni / 3;
    obj.tipo_indice = GL_UNSIGNED_INT;

    std::vector<float> vertex_data;
    for (size_t i = 0; i < vertices.size() / 3; ++i) {
        vertex_data.push_back(vertices[3 * i]);
        vertex_data.push_back(vertices[3 * i + 1]);
        vertex_data.push_back(vertices[3 * i + 2]);
        if (!texcoords.empty()) {
            vertex_data.push_back(texcoords[2 * i]);
            vertex_data.push_back(texcoords[2 * i + 1]);
        }
        if (!normals.empty()) {
            vertex_data.push_back(normals[3 * i]);
            vertex_data.push_back(normals[3 * i + 1]);
            vertex_data.push_back(normals[3 * i + 2]);
        }
    }

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(float), vertex_data.data(), GL_STATIC_DRAW);

    int stride = 3 * sizeof(float);
    if (!texcoords.empty()) stride += 2 * sizeof(float);
    if (!normals.empty()) stride += 3 * sizeof(float);

    // Defino 1er argumento (atributo 0) del vertex shader (siempre XYZ)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);

    int offset = 3 * sizeof(float);
    if (!texcoords.empty()) {
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)offset);
        offset += 2 * sizeof(float);
    }
    if (!normals.empty()) {
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Asignados atributos, podemos desconectar BUFFER

    glGenBuffers(1, &i_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0); // Cerramos Vertex Array con todo listo para ser pintado

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    obj.VAO = VAO;

	printf("OBJ: %d vertices, %d triangles, %d indices, %d VAO, %d Tipo ind\n", obj.Nv, obj.Nt, obj.Ni, obj.VAO, obj.tipo_indice);
	return obj;
}
objeto cargar_modelo2(char* path) {

	objeto obj;
	GLuint VAO;
	std::vector<glm::vec3> out_vertices;
	std::vector<glm::vec2> out_uvs;
	std::vector<glm::vec3> out_normals;

	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if( file == NULL ){
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		obj.VAO = 0;
		obj.Ni = 0;
		obj.tipo_indice = 0;
		glfwTerminate();
	}

	while( 1 ){

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF) {
			break; // EOF = End Of File. Quit the loop.
		}

		// else : parse lineHeader

		if ( strcmp( lineHeader, "v" ) == 0 ){
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
			temp_vertices.push_back(vertex);
		}else if ( strcmp( lineHeader, "vt" ) == 0 ){
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y );
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}else if ( strcmp( lineHeader, "vn" ) == 0 ){
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			temp_normals.push_back(normal);
		}else if ( strcmp( lineHeader, "f" ) == 0 ){
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
			if (matches != 9){
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				obj.VAO = 0;
				obj.Ni = 0;
				obj.tipo_indice = 0;
				glfwTerminate();
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices    .push_back(uvIndex[0]);
			uvIndices    .push_back(uvIndex[1]);
			uvIndices    .push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}else{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for( unsigned int i=0; i<vertexIndices.size(); i++ ){

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
		glm::vec2 uv = temp_uvs[ uvIndex-1 ];
		glm::vec3 normal = temp_normals[ normalIndex-1 ];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs     .push_back(uv);
		out_normals .push_back(normal);

	}
	// Create a vertex array object
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Create a vertex buffer object and copy the vertex data to it
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, out_vertices.size() * sizeof(glm::vec3), &out_vertices[0], GL_STATIC_DRAW);

	// Specify the layout of the vertex data
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Unbind the VBO and VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Store the VAO in the objeto instance
	obj.VAO = VAO;
	obj.Nv = out_vertices.size();
	obj.Nt = obj.Nv / 3; // Assuming each triangle is made up of 3 vertices
	obj.Ni = obj.Nt * 3; // Assuming each triangle is made up of 3 indices
	obj.tipo_indice = GL_UNSIGNED_INT; // Assuming indices are stored as unsigned ints

	printf("OBJ: %d vertices, %d triangles, %d indices, %d VAO, %d Tipo ind\n", obj.Nv, obj.Nt, obj.Ni, obj.VAO, obj.tipo_indice);
	return obj;

}
objeto cargar_modelo3(char* fichero)
{
    objeto obj;
    GLuint VAO;
    GLuint buffer, i_buffer;

    std::vector<float> vertices;
    std::vector<float> texcoords;
    std::vector<float> normals;
    std::vector<unsigned int> indices;

    FILE* fid;
    fopen_s(&fid, fichero, "r");

    if (fid == NULL) {
        printf("Error al leer datos. Existe el fichero %s?\n", fichero);
        obj.VAO = 0;
        obj.Ni = 0;
        obj.tipo_indice = 0;
        glfwTerminate();
        return obj;
    }

    char line[128];
    while (fgets(line, sizeof(line), fid)) {
        if (strncmp(line, "v ", 2) == 0) {
            float x, y, z;
            sscanf(line + 2, "%f %f %f", &x, &y, &z);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        } else if (strncmp(line, "vt ", 3) == 0) {
            float u, v;
            sscanf(line + 3, "%f %f", &u, &v);
            texcoords.push_back(u);
            texcoords.push_back(v);
        } else if (strncmp(line, "vn ", 3) == 0) {
            float nx, ny, nz;
            sscanf(line + 3, "%f %f %f", &nx, &ny, &nz);
            normals.push_back(nx);
            normals.push_back(ny);
            normals.push_back(nz);
        } else if (strncmp(line, "f ", 2) == 0) {
            unsigned int v[3], vt[3], vn[3];
            int matches = sscanf(line + 2, "%u/%u/%u %u/%u/%u %u/%u/%u",
                &v[0], &vt[0], &vn[0], &v[1], &vt[1], &vn[1], &v[2], &vt[2], &vn[2]);
            if (matches == 9) {
                for (int i = 0; i < 3; ++i) {
                    indices.push_back(v[i] - 1);
                }
            } else {
                matches = sscanf(line + 2, "%u/%u %u/%u %u/%u",
                    &v[0], &vt[0], &v[1], &vt[1], &v[2], &vt[2]);
                if (matches == 6) {
                    for (int i = 0; i < 3; ++i) {
                        indices.push_back(v[i] - 1);
                    }
                } else {
                    matches = sscanf(line + 2, "%u//%u %u//%u %u//%u",
                        &v[0], &vn[0], &v[1], &vn[1], &v[2], &vn[2]);
                    if (matches == 6) {
                        for (int i = 0; i < 3; ++i) {
                            indices.push_back(v[i] - 1);
                        }
                    } else {
                        matches = sscanf(line + 2, "%u %u %u",
                            &v[0], &v[1], &v[2]);
                        if (matches == 3) {
                            for (int i = 0; i < 3; ++i) {
                                indices.push_back(v[i] - 1);
                            }
                        } else {
                            printf("Formato de cara no soportado.\n");
                            fclose(fid);
                            obj.VAO = 0;
                            obj.Ni = 0;
                            obj.tipo_indice = 0;
                            glfwTerminate();
                            return obj;
                        }
                    }
                }
            }
        }
    }

    fclose(fid);

    obj.Ni = indices.size();
    obj.Nv = vertices.size() / 3;
    obj.Nt = obj.Ni / 3;
    obj.tipo_indice = GL_UNSIGNED_INT;

    std::vector<float> vertex_data;
    for (size_t i = 0; i < vertices.size() / 3; ++i) {
        vertex_data.push_back(vertices[3 * i]);
        vertex_data.push_back(vertices[3 * i + 1]);
        vertex_data.push_back(vertices[3 * i + 2]);
        if (!texcoords.empty()) {
            vertex_data.push_back(texcoords[2 * i]);
            vertex_data.push_back(texcoords[2 * i + 1]);
        }
        if (!normals.empty()) {
            vertex_data.push_back(normals[3 * i]);
            vertex_data.push_back(normals[3 * i + 1]);
            vertex_data.push_back(normals[3 * i + 2]);
        }
    }

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(float), vertex_data.data(), GL_STATIC_DRAW);

    int stride = 3 * sizeof(float);
    if (!texcoords.empty()) stride += 2 * sizeof(float);
    if (!normals.empty()) stride += 3 * sizeof(float);

    // Defino 1er argumento (atributo 0) del vertex shader (siempre XYZ)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);

    int offset = 3 * sizeof(float);
    if (!texcoords.empty()) {
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)offset);
        offset += 2 * sizeof(float);
    }
    if (!normals.empty()) {
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Asignados atributos, podemos desconectar BUFFER

    glGenBuffers(1, &i_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0); // Cerramos Vertex Array con todo listo para ser pintado

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    obj.VAO = VAO;

    return obj;
}
objeto cargar_modelo4(char* fichero)
{
	std::vector<GLfloat> vertices;
	std::vector<GLuint> indices;

	FILE* file = fopen(fichero, "r");
	if (file == NULL) {
		printf("Error: no se puede abrir el archivo!\n");
		exit(1);
	}

	while (!feof(file)) {
		char line[128];
		fgets(line, 128, file);

		if (strncmp(line, "v ", 2) == 0) {
			GLfloat x, y, z;
			sscanf(line, "v %f %f %f", &x, &y, &z);
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
		} else if (strncmp(line, "f ", 2) == 0) {
			GLuint i1, i2, i3;
			sscanf(line, "f %d %d %d", &i1, &i2, &i3);
			// .obj indices are 1-based, adjust to 0-based
			indices.push_back(i1 - 1);
			indices.push_back(i2 - 1);
			indices.push_back(i3 - 1);
		}
	}

	fclose(file);

	// Create and bind the VAO
	GLuint VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	// Vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

	// Element buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	// Vertex attribute: position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // Unbind VAO

	objeto obj;
	obj.VAO = VAO;
	obj.Nv = vertices.size() / 3; // Number of vertices
	obj.Ni = indices.size(); // Number of indices
	obj.Nt = 0; // Number of texture coordinates (not used in this example)
	obj.tipo_indice = GL_UNSIGNED_INT;

	return obj;
}

bool cargar_modelo(
	const char * path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals,
	std::vector<unsigned int> & out_indices
){
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if( file == NULL ){
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		return false;
	}

	while( 1 ){

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader

		if ( strcmp( lineHeader, "v" ) == 0 ){
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
			temp_vertices.push_back(vertex);
		}else if ( strcmp( lineHeader, "vt" ) == 0 ){
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y );
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}else if ( strcmp( lineHeader, "vn" ) == 0 ){
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			temp_normals.push_back(normal);
		}else if ( strcmp( lineHeader, "f" ) == 0 ){
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
			if (matches != 9){
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices    .push_back(uvIndex[0]);
			uvIndices    .push_back(uvIndex[1]);
			uvIndices    .push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}else{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for( unsigned int i=0; i<vertexIndices.size(); i++ ){

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
		glm::vec2 uv = temp_uvs[ uvIndex-1 ];
		glm::vec3 normal = temp_normals[ normalIndex-1 ];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs     .push_back(uv);
		out_normals .push_back(normal);
		out_indices .push_back(vertexIndex-1);
	}

	return true;
}

void transfer_mat4(const char* name, mat4 M)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);

 loc=glGetUniformLocation(prog,name); 
 if (loc == -1) {
	 printf("No existe variable llamada %s en el programa activo de la GPU (%d)\n", name, prog);
	 glfwTerminate(); //exit(EXIT_FAILURE);
	 //return NULL;
 }
 else glUniformMatrix4fv(loc, 1, GL_FALSE, &M[0][0]);
}

void transfer_mat3(const char* name, mat3 M)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);

 loc=glGetUniformLocation(prog,name); 
 if (loc == -1) {
	 printf("No existe variable llamada %s en el programa activo de la GPU (%d)\n", name, prog);
	 glfwTerminate(); //exit(EXIT_FAILURE);
 }
 else glUniformMatrix3fv(loc, 1, GL_FALSE, &M[0][0]);
}


void transfer_vec4(const char* name, vec4 x)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);
 loc=glGetUniformLocation(prog,name);
 if (loc == -1) {
	 printf("No existe variable llamada %s en el programa activo de la GPU (%d)\n", name, prog);
	 glfwTerminate(); //exit(EXIT_FAILURE);
 }
 else glUniform4fv(loc, 1, &x[0]);
}

void transfer_vec3(const char* name, vec3 x)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);
 loc=glGetUniformLocation(prog,name);
 if (loc == -1) {
	 printf("No existe variable llamada %s en el programa activo de la GPU (%d)\n", name, prog);
	 glfwTerminate(); //exit(EXIT_FAILURE);
 }
 else glUniform3fv(loc, 1, &x[0]);
}

void transfer_vec2(const char* name, vec2 x)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);
 loc=glGetUniformLocation(prog,name);
 if (loc == -1) {
	 printf("No existe variable llamada %s en el programa activo de la GPU (%d)\n", name, prog);
	 glfwTerminate(); //exit(EXIT_FAILURE);
 }
 else glUniform2fv(loc, 1, &x[0]);
}

void transfer_int(const char* name, GLuint valor)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);
 loc=glGetUniformLocation(prog,name);
 if (loc == -1) {
	 printf("No existe variable llamada %s en el programa activo de la GPU (%d)\n", name, prog);
	 glfwTerminate(); //exit(EXIT_FAILURE);
 }
 else glUniform1i(loc,valor);
}


void transfer_float(const char* name, GLfloat valor)
{
 GLuint loc;
 GLuint prog;
 
 glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&prog);
 loc=glGetUniformLocation(prog,name);
 if (loc == -1) {
	 printf("No existe variable llamada %s en el programa activo de la GPU (%d)\n", name, prog);
	 glfwTerminate(); //exit(EXIT_FAILURE);
 }
 else glUniform1f(loc, valor);
}


void vuelca_mat4(glm::mat4 M)
{
	int j, k;
	printf("--------------------------------------\n");
	for (k = 0; k<4; k++) { for (j = 0; j<4; j++) printf("%6.3f ", M[j][k]); printf("\n"); }
	printf("--------------------------------------\n");
}

