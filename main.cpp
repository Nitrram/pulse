/***
	This file is part of PulseAudio.
	PulseAudio is free software; you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published
	by the Free Software Foundation; either version 2.1 of the License,
	or (at your option) any later version.
	PulseAudio is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	General Public License for more details.
	You should have received a copy of the GNU Lesser General Public License
	along with PulseAudio; if not, see <http://www.gnu.org/licenses/>.
***/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#define BUFSIZE 1024


// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
#define WIDTH 1024
#define WINDOW_HEIGHT 768
#define HEIGHT 1 //768

static const int _num_bytes = WIDTH * HEIGHT *sizeof(char);
GLFWwindow* window;

#include "shader.h"

// use fft
#include <stdio.h>
#include <stdlib.h>
//#include "fft.h"

/*
// The sample type to use
static const pa_sample_spec ss = {
.format = PA_SAMPLE_S32LE , //PA_SAMPLE_S16BE, ??? Which one to us here ??? BE...Big Endian
.rate = 44100, // That are samples per second
.channels = 2
};

// Create the recording stream
// see: http://freedesktop.org/software/pulseaudio/doxygen/parec-simple_8c-example.html
if (!(s = pa_simple_new(NULL, "Record", PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error))) {
fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
pa_simple_free(s);
exit(EXIT_FAILURE);
}

int i = -1;

while (!exit_program) {
i = (i+1) % BUFNUMBER;

pthread_mutex_lock(&(buffer[i].write));
// Record data and save it to the buffer
if (pa_simple_read(s, buffer[i].buf, sizeof(buffer[i].buf), &error) < 0) {
fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
pa_simple_free(s);
exit(EXIT_FAILURE);
}

// unlock the reading mutex
pthread_mutex_unlock(&(buffer[i].read)); // open up for reading

}*/

//===========================================================
// #define BUFSIZE 44100  // Size of one element
// #define BUFNUMBER 16 // Number of elements
// #define AUDIO_BUFFER_FORMAT char

// // one element of the ringbuffer
// typedef struct ringbuf {
//	   AUDIO_BUFFER_FORMAT buf[BUFSIZE]; /* The buffer array */
//	   pthread_mutex_t read; /* indicates if block was read */
//	   pthread_mutex_t write; /* for locking writing */
// } ringbuffer_element;
//===========================================================

/*
// The sample type to use
static const pa_sample_spec ss = {
.format = PA_SAMPLE_S32LE , //PA_SAMPLE_S16BE,
.rate = 44100,
.channels = 2
};

if (stream == NULL) {
if (!(stream = pa_simple_new(NULL, "Stream", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
return false;
}
}

if (pa_simple_write(stream, buf, (size_t) size, &error) < 0) {
fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
pa_simple_free(stream);
return false;
}

// Make sure that every single sample was played
if (pa_simple_drain(stream, &error) < 0) {
fprintf(stderr, __FILE__": pa_simple_drain() failed: %s\n", pa_strerror(error));
pa_simple_free(stream);
return false;
}
*/

#ifdef __VISUAL__

#ifdef __OPEN_CL__
static cl_mem _pixel_buf;
#else
static GLubyte *_pixel_buf = 0;
#endif /*__OPEN_CL__*/


#define VIDEO_SURFACE_NUM_PBOS 2

static GLuint pbos[VIDEO_SURFACE_NUM_PBOS];
static unsigned int read_dx;
static unsigned int write_dx;

static GLuint texId;
static GLuint tex;
static GLuint vao;
static GLuint vbo[2];

static GLuint prog;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void setup_gl(char *zero_argv) {

	// relativize the path
	char * argv = strrchr(zero_argv, (int)'/');
	*(argv+1) = '\0';

	char vert_shader[1024];
	char frag_shader[1024];

	memset(vert_shader, 0 ,1024);
	memset(frag_shader, 0, 1024);

	strcpy(vert_shader, zero_argv);
	strcat(vert_shader, "TransformVertexShader.vertexshader");

	strcpy(frag_shader, zero_argv);
	strcat(frag_shader, "TextureFragmentShader.fragmentshader");

	prog = LoadShaders( vert_shader, frag_shader );

	texId  = glGetUniformLocation(prog, "myTextureSampler");

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	glUseProgram(0);


	glGenBuffers(VIDEO_SURFACE_NUM_PBOS, pbos);
	for(int i = 0; i < VIDEO_SURFACE_NUM_PBOS; ++i) {
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos[i]);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, _num_bytes, NULL, GL_STREAM_DRAW);
	}
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_ALPHA, GL_UNSIGNED_BYTE, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//float bc[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, bc);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLfloat vertices[] = {
		-1.0f,	1.0f,
		 1.0f,	1.0f,
		 1.0f, -1.0f,
		-1.0f, -1.0f
	};

	GLfloat tex_coords[] = {
		-1.0f,	1.0f,
		 1.0f,	1.0f,
		 1.0f, -1.0f,
		-1.0f, -1.0f
	};

	glGenBuffers(2, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER,
				 8*sizeof(GLfloat),
				 vertices,
				 GL_STATIC_DRAW);


	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER,
				 8*sizeof(GLfloat),
				 tex_coords,
				 GL_STATIC_DRAW);


	glEnableVertexAttribArray(0); // pos
	glEnableVertexAttribArray(1); // tex
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, 0, (GLvoid*)0);

	_pixel_buf = (GLubyte*)malloc(_num_bytes);
}

/*set pixels*/
void setPixels(unsigned char* pixels) {
	if(!pixels) {
		printf("WARNING: VideoSurface::setPixels(), given pixels is NULL.\n");
		return;
	}
	if(!tex || WIDTH == 0 || HEIGHT == 0) {
		printf("WARNING: VideoSurface::setPixels(): cannot set, we're not initialized.\n");
		return;
	}

	read_dx = (read_dx + 1) % VIDEO_SURFACE_NUM_PBOS;
	write_dx = (read_dx + 1) % VIDEO_SURFACE_NUM_PBOS;

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos[read_dx]);

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_ALPHA, GL_UNSIGNED_BYTE, 0);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos[write_dx]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, _num_bytes, NULL, GL_STREAM_DRAW);
	GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
	if(ptr) {
		memcpy(ptr, pixels, _num_bytes);
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
	}

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}


void draw(/*int x, int	y*/) {
	glDepthMask(GL_FALSE);

	glBindVertexArray(vao);
	glUseProgram(prog);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDepthMask(GL_TRUE);
}

#endif /*__VISUAL*/

int main(int argc, char*argv[]) {
	/* The sample type to use */
	static const pa_sample_spec ss = {
		.format = PA_SAMPLE_U8,
		.rate = 44100,
		.channels = 2
	};
	pa_simple *s = NULL;
	int ret = 1;
	int error;

//	uint8_t *fft_buf = allocate(BUFSIZE);

//	printf("%s\n", *argv);

#ifdef __VISUAL__

	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( WIDTH, WINDOW_HEIGHT, "pulse", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetKeyCallback(window, key_callback);

	setup_gl(*argv);

#endif /*__VISUAL__*/


	/* Create the recording stream */
	if (!(s = pa_simple_new(NULL, argv[0], PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error))) {
		fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
		goto finish;
	}

	while (!glfwWindowShouldClose(window)) {
		uint8_t buf[BUFSIZE];
		/* Record some data ... */
		if (pa_simple_read(s, buf, sizeof(buf), &error) < 0) {
			fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
			goto finish;
		}

		//fft(buf, BUFSIZE, fft_buf);
		//four1(buf, BUFSIZE);
#ifdef __VISUAL__

//		int stride = WIDTH * 4;
//		int begin = HEIGHT/2;

		//	memset(_pixel_buf, 0, _num_bytes);

		memcpy(_pixel_buf, buf, BUFSIZE);

			/*
		for(int amp=0; amp < BUFSIZE; amp++) {
			size_t px = (buf[amp] * stride) + (amp*4);
			_pixel_buf[px] = 0x80;
			_pixel_buf[px+1] = 0x00;
			_pixel_buf[px+2] = 0xff;
			_pixel_buf[px+3] = 0x10;
		}
			*/


		/*
		for(int i=stride*begin;i<stride*(begin+10);i+=64) {
			_pixel_buf[i] = 0x80;
			_pixel_buf[i+1] = 0x00;
			_pixel_buf[i+2] = 0x00;
			_pixel_buf[i+3] = 0xff;
		}
		*/


		setPixels(_pixel_buf);
		draw();

		glfwSwapBuffers(window);

#endif /*__VISUAL__*/

		/*
		for(size_t i=0;i<20;++i) {
			printf("%02x ", buf[i]);
		}
		printf("\n");
		*/
		// /* And write it to STDOUT */
		// if (loop_write(STDOUT_FILENO, buf, sizeof(buf)) != sizeof(buf)) {
		//	  fprintf(stderr, __FILE__": write() failed: %s\n", strerror(errno));
		//	  goto finish;
		// }
	}
	ret = 0;
  finish:

#ifdef __VISUAL__

	free(_pixel_buf);
	_pixel_buf = 0;

	if(texId) {
		glDeleteTextures(1, &texId);
		texId = 0;
	}

	if(tex) {
		glDeleteTextures(1, &tex);
		tex = 0;
	}

	if(*vbo) {
		glDeleteBuffers(2, vbo);
	}

#endif /*__VISUAL__*/


	//free(fft_buf);
	if (s)
		pa_simple_free(s);
	return ret;
}