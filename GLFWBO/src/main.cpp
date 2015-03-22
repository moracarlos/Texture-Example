#define GLFW_DLL
#include "../include/GL/glew.h"
#include "../include/GLFW/glfw3.h"
#include "../include/glm/glm.hpp"
#include "../include/glm/gtc/matrix_transform.hpp"
#include "../include/glm/gtc/type_ptr.hpp"
#include "GLSLProgram.h"
#include <stdlib.h>
#include <string>
#include <iostream>

#include "../include/IL/il.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;

///< Only wrapping the glfw functions
namespace glfwFunc
{
	GLFWwindow* glfwWindow;
	const unsigned int WINDOW_WIDTH = 512;
	const unsigned int WINDOW_HEIGHT = 512;
	const float NCP = 0.01f;
	const float FCP = 5.0f;
	const float fAngle = 45.f;
	string strNameWindow = "Hello GLFW";

	CGLSLProgram m_program;
	glm::mat4x4 mProjMatrix, mModelViewMatrix;
	GLuint m_idVAO;
	int imageWidth, imageHeight, imageFormat, id;
	unsigned char* data;
	GLuint vindex;
	GLuint index[] = {0,1,2, 2,1,3};

	GLuint texture;
	///< Function to build a simple triangle
	
	unsigned int LoadTexture(const char* filename) {
		ilInit();
		ILuint imageID;
		GLuint textureID;
		ILboolean success;
		ILenum error;
		ilGenImages(1, &imageID);
		ilBindImage(imageID);
		ilEnable(IL_ORIGIN_SET);
		ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

		success = ilLoadImage(filename);
		if (success)
		{
			success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
			if (!success){
				error = ilGetError();
				cout << "Image conversion fails" << endl;
			}
			glGenTextures(1, &textureID);
			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D,
				0,
				ilGetInteger(IL_IMAGE_FORMAT),
				ilGetInteger(IL_IMAGE_WIDTH),
				ilGetInteger(IL_IMAGE_HEIGHT),
				0,
				ilGetInteger(IL_IMAGE_FORMAT),
				GL_UNSIGNED_BYTE,
				ilGetData()
				);
			imageWidth = ilGetInteger(IL_IMAGE_WIDTH);
			imageHeight = ilGetInteger(IL_IMAGE_HEIGHT);
			imageFormat = ilGetInteger(IL_IMAGE_FORMAT);
		}

		ilDeleteImages(1, &imageID);
		std::cout << "Textura creada exitosamente" << endl;
		return textureID;
	}
	
	void initiateTriangle()
	{
		//Figure
		float vfVertexT[] = { -0.6f, -0.4f, 0.f,
							   0.6f, -0.4f, 0.f,
							   -0.6f, 0.4f, 0.f,
							   0.6f, 0.4f, 0.f };
		float vfColorT[] = { 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f };
		//index = { 0, 1, 2, 0, 1, 3 };
		float textureCoord[] = {
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f, 1.0f,
			1.f, 1.f,
		};


		GLuint idVBO;
		//
		//Generate the Vertex Array
		glGenVertexArrays(1, &m_idVAO);
		glBindVertexArray(m_idVAO);		

			//Generate the Buffer Object
			glGenBuffers(1, &idVBO);
			glBindBuffer(GL_ARRAY_BUFFER, idVBO);

				//Load the data
			glBufferData(GL_ARRAY_BUFFER, sizeof(vfVertexT)+sizeof(vfColorT)+sizeof(textureCoord), NULL, GL_STATIC_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vfVertexT), vfVertexT);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(vfVertexT), sizeof(vfColorT), vfColorT);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(vfVertexT)+sizeof(vfColorT), sizeof(textureCoord), textureCoord);
		
				//Map the atribute array to an atibute location in the shader
				glEnableVertexAttribArray(m_program.getLocation("vVertex"));
				glVertexAttribPointer(m_program.getLocation("vVertex"), 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0)); //Vertex
				glEnableVertexAttribArray(m_program.getLocation("vColor"));
				glVertexAttribPointer(m_program.getLocation("vColor"), 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vfVertexT))); //Colors
				glEnableVertexAttribArray(m_program.getLocation("vTexture"));
				glVertexAttribPointer(m_program.getLocation("vTexture"), 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vfVertexT)+sizeof(vfColorT)));

				glGenBuffers(1, &vindex);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vindex);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

			//Unbind Buffer Object
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		//Unbind Vertex Array
		glBindVertexArray(0);
	}
	
	bool initialize()
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		//Glew INIT
		glewExperimental = GL_TRUE;
		if(glewInit() != GLEW_OK) 
		{
			cout << "- glew Init failed :(" << endl;
			return false;
		}

		std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
		std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
		std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
		std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

		//Load the shaders
		m_program.loadShader("shaders/basic.vert", CGLSLProgram::VERTEX);
		m_program.loadShader("shaders/basic.frag", CGLSLProgram::FRAGMENT);
		//Link the shaders in a program
		m_program.create_link();
		//Enable the program
		m_program.enable();
				//Link the attributes and the uniforms
				m_program.addAttribute("vVertex");
				m_program.addAttribute("vColor");
				m_program.addAttribute("vTexture");
				m_program.addUniform("mProjection");
				m_program.addUniform("mModelView");
				//No estoy seguro si haya que hacerlo con myTextureSampler
		//Disable the program
		m_program.disable();

		const char* file = "fb.png";
		texture = LoadTexture(file);
		if (texture == 0)
			return(2);
		//Function to initiate a triangle
		initiateTriangle();

		return true;
	}
	
	///< Callback function used by GLFW to capture some possible error.
	void errorCB(int error, const char* description)
	{
		cout << description << endl;
	}

	///
	/// The keyboard function call back
	/// @param window id of the window that received the event
	/// @param iKey the key pressed or released
	/// @param iScancode the system-specific scancode of the key.
	/// @param iAction can be GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT
	/// @param iMods Bit field describing which modifier keys were held down (Shift, Alt, & so on)
	///
	void keyboardCB(GLFWwindow* window, int iKey, int iScancode, int iAction, int iMods)
	{
		if (iAction == GLFW_PRESS)
		{
			switch (iKey)
			{
				case GLFW_KEY_ESCAPE:
				case GLFW_KEY_Q:
					glfwSetWindowShouldClose(window, GL_TRUE);
					break;
			}
		}
	}
	
	///< The resizing function
	void resizeCB(GLFWwindow* window, int iWidth, int iHeight)
	{
		if(iHeight == 0) iHeight = 1;
		float ratio = iWidth / float(iHeight);
		glViewport(0, 0, iWidth, iHeight);
		mProjMatrix = glm::perspective(fAngle, ratio, NCP, FCP);
	}

	///< The main rendering function.
	void draw()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.15f, 0.15f, 0.15f, 1.f);
		
		mModelViewMatrix = glm::translate(glm::mat4(), glm::vec3(0,0,-2.f)); 
		
		//Display the triangle
		m_program.enable();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vindex);
			glUniformMatrix4fv(m_program.getLocation("mModelView"), 1, GL_FALSE, glm::value_ptr(mModelViewMatrix));
			glUniformMatrix4fv(m_program.getLocation("mProjection"), 1, GL_FALSE, glm::value_ptr(mProjMatrix));
			glBindVertexArray(m_idVAO);
				//glDrawArrays(GL_TRIANGLES, 0, 3);
			glDrawElements(GL_TRIANGLES, sizeof(index), GL_UNSIGNED_INT, NULL);
			glBindVertexArray(0);
			glBindTexture(GL_TEXTURE_2D, 0);
		m_program.disable();

		glfwSwapBuffers(glfwFunc::glfwWindow);
	}
	
	/// Here all data must be destroyed + glfwTerminate
	void destroy()
	{
		if(glIsVertexArray(m_idVAO)) glDeleteVertexArrays(1, &m_idVAO);
		glfwDestroyWindow(glfwFunc::glfwWindow);
		glfwTerminate();
	}
};

int main(int argc, char** argv)
{
	glfwSetErrorCallback(glfwFunc::errorCB);
	if (!glfwInit())	exit(EXIT_FAILURE);
	glfwFunc::glfwWindow = glfwCreateWindow(glfwFunc::WINDOW_WIDTH, glfwFunc::WINDOW_HEIGHT, glfwFunc::strNameWindow.c_str(), NULL, NULL);
	if (!glfwFunc::glfwWindow)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(glfwFunc::glfwWindow);
	if(!glfwFunc::initialize()) exit(EXIT_FAILURE);
	glfwFunc::resizeCB(glfwFunc::glfwWindow, glfwFunc::WINDOW_WIDTH, glfwFunc::WINDOW_HEIGHT);	//just the 1st time
	glfwSetKeyCallback(glfwFunc::glfwWindow, glfwFunc::keyboardCB);
	glfwSetWindowSizeCallback(glfwFunc::glfwWindow, glfwFunc::resizeCB);
	// main loop!
	while (!glfwWindowShouldClose(glfwFunc::glfwWindow))
	{
		glfwFunc::draw();
		glfwPollEvents();	//or glfwWaitEvents()
	}

	glfwFunc::destroy();
	return EXIT_SUCCESS;
}