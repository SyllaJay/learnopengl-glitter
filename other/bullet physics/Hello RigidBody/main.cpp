// Preprocessor Directives
#define STB_IMAGE_IMPLEMENTATION

// Local Headers
#include "glitter.hpp"
#include "shader.h"
#include "camera.h"
#include "model.h"

// Console Color
#include "consoleColor.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

// Standard Headers
//#include <cstdio>
//#include <cstdlib>
#include <iostream>
#include <vector>

// ������������
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void do_movement();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void RenderCube();
void RenderQuad();

// ����ģ��
Model* sphereModel;

// ���
Camera camera(glm::vec3(0.0f, 3.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

// ʱ������Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

// �����������
btDynamicsWorld* world;
btDispatcher* dispatcher;
btCollisionConfiguration* collisionConfiguration;
btBroadphaseInterface* broadphase;
btConstraintSolver* solver;
std::vector<btRigidBody*> bodies;

void btInit();
void btExit();
btRigidBody* addSphere(float radius, float x, float y, float z, float mass);
void renderSphere(btRigidBody* sphere, Shader &shader);

int main(int argc, char * argv[]) {

	// glfw��ʼ��
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // ����������Mac OS Xϵͳ������
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// ��������,��ȡ������������
	GLFWwindow* window = glfwCreateWindow(mWidth, mHeight, "LearnOpenGL", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// ͨ��glfwע���¼��ص�
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Load OpenGL Functions
	gladLoadGL();
	//    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));
	std::cout << BLUE << "OpenGL " << glGetString(GL_VERSION) << RESET << std::endl;

	// ��ѯGPU���֧�ֶ������
	//    GLint nrAttributes;
	//    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	//    std::cout << GREEN << "Maximum nr of vertex attributes supported: " << nrAttributes << RESET << std::endl;

	// ��ȡ�ӿ�
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// ����OpenGL��ѡ��
	glEnable(GL_DEPTH_TEST); // ������Ȳ���
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// ������ɫ������
	Shader ourShader("vert.vert", "frag.frag");
	Shader redBallShader("redball.vert", "redball.frag");

	// ʮ����ͬ�������������
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	// ʹ���߿�ģʽ������Ⱦ
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// ��ȡ��������ͼ
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); // ֮���GL_TEXTURE_2D�����Բ�����������texture������
										   // ���������Ʒ�ʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// ����������˷�ʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// ��ȡͼƬ����������ӳ��
	//    int width, height;
	unsigned char* image = stbi_load("wall.jpg", &width, &height, 0, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(image);
	glBindTexture(GL_TEXTURE_2D, 0);

    // ������������
    btInit();

	// ��Ⱦ
	while (!glfwWindowShouldClose(window))
	{
		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// ����ģ��ʱ��
		world->stepSimulation(deltaTime);

		// ����¼�
		glfwPollEvents();
		do_movement();

		// ��Ⱦָ��
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		//        glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ��texture
		glBindTexture(GL_TEXTURE_2D, texture);

		// ��ͼ
		ourShader.Use();

		// �����������
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
		view = camera.GetViewMatrix();
		projection = glm::perspective(camera.Zoom, (GLfloat)mWidth / (GLfloat)mHeight, 0.1f, 1000.0f);
		// Get their uniform location
		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
		GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
		// Pass them to the shaders
//		for (GLuint i = 0; i < 10; i++)
//		{
//			model = glm::mat4();
//			model = glm::translate(model, cubePositions[i]);
//			model = glm::rotate(model, (GLfloat)glfwGetTime() * 1.0f, glm::vec3(0.5f, 1.0f, 0.0f));
//			GLfloat angle = 20.0f + i;
//			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
//
//			//glDrawArrays(GL_TRIANGLES, 0, 36);
//			RenderCube();
//		}
		glBindVertexArray(0);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// Note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(10.0f, 1.0f, 10.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		RenderQuad();

		glBindVertexArray(0);

		redBallShader.Use();
		glUniformMatrix4fv(glGetUniformLocation(redBallShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(redBallShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        for (int i = 0; i < bodies.size(); i++)
        {
            if (bodies[i]->getCollisionShape()->getShapeType() == SPHERE_SHAPE_PROXYTYPE) {
                renderSphere(bodies[i], redBallShader);
            }
        }

		// ��������
		glfwSwapBuffers(window);
	}

	// �ͷ�GLFW������ڴ�
	glfwTerminate();

    // �˳���������
    void btExit();

	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// ���û�����ESC��,��������window���ڵ�WindowShouldClose����Ϊtrue
	// �ر�Ӧ�ó���
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}

    // ���ո���С��
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
		std::cout << "camera.Front: " << camera.Front.x << "," << camera.Front.y << "," << camera.Front.z << std::endl;
        btRigidBody* sphere = addSphere(1.0, camera.Position.x, camera.Position.y, camera.Position.z, 1.0);
        glm::vec3 look = camera.Front;
        sphere->setLinearVelocity(btVector3(20 * look.x, 20 * look.y, 20 * look.z));
    }
}

void do_movement()
{
	// ���������
	GLfloat cameraSpeed = 5.0f * deltaTime;
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos; // ע���������෴�ģ���Ϊy�����Ǵӵײ����������������
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// RenderCube() Renders a 1x1 3D cube in NDC.
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;
void RenderCube()
{
	// Initialize (if necessary)
	if (cubeVAO == 0)
	{
		GLfloat vertices[] = {
			// Back face
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
			0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,  // top-right
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // bottom-left
			-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,// top-left

			// Front face
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
			0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
			0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // top-right
			 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
			-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom-left

			// Left face
			-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
			-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
			-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-left
			-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
			-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
			-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right

			// Right face
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
			0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
			0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right         
			0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-right
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // top-left
			0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left 

			// Bottom face
			-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
			0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
			0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,// bottom-left
			0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
			-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
			-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right

			// Top face
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
			0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
			0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right     
			0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
			-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// Fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// Link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// Render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

// RenderQuad() Renders a 1x1 quad in NDC, best used for framebuffer color targets
// and post-processing effects.
GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
	if (quadVAO == 0)
	{
		GLfloat quadVertices[] = {
			// Positions        // Texture Coords
			-1.0f, 0.0f, 1.0f,   0.0f, 1.0f,
			-1.0f, 0.0f,-1.0f,   0.0f, 0.0f,
			1.0f,  0.0f, 1.0f,   1.0f, 1.0f,
			1.0f,  0.0f,-1.0f,   1.0f, 0.0f,
		};
		// Setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

btRigidBody* addSphere(float radius, float x, float y, float z, float mass)
{
    btTransform t;
    t.setIdentity();
    t.setOrigin(btVector3(x, y, z));
    btSphereShape* sphere = new btSphereShape(radius);
    btVector3 inertia(0, 0, 0);
    if (mass != 0.0) {
        sphere->calculateLocalInertia(mass, inertia);
    }

    btMotionState* motion = new btDefaultMotionState(t);
    btRigidBody::btRigidBodyConstructionInfo info(mass, motion, sphere);
    btRigidBody* body = new btRigidBody(info);
	body->setRestitution(btScalar(0.5));
    world->addRigidBody(body);
    bodies.push_back(body);

    return body;
}

void renderSphere(btRigidBody* sphere, Shader &_shader)
{
    if (sphere->getCollisionShape()->getShapeType() != SPHERE_SHAPE_PROXYTYPE){
//        std::cout << RED << "sphere error" << RESET << std::endl;
        return;
    }
    float r = ((btSphereShape*)sphere->getCollisionShape())->getRadius();
    btTransform t;
    sphere->getMotionState()->getWorldTransform(t);

    float mat[16];
    t.getOpenGLMatrix(mat);

    glm::mat4 positionTrans = glm::make_mat4(mat);

    glm::mat4 model;
//    model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
    model = positionTrans * model;
    model = glm::scale(model, glm::vec3(r));
    glUniformMatrix4fv(glGetUniformLocation(_shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

    sphereModel->Draw(_shader);
}

void btInit()
{
	std::cout << "load model before" << std::endl;

	// ����ģ�ͳ�ʼ��
	sphereModel = new Model("sphere/sphere.obj");

	std::cout << "load model end" << std::endl;

	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	broadphase = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();
    world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    world->setGravity(btVector3(0, -10, 0));

    btTransform t;
    t.setIdentity();
    t.setOrigin(btVector3(0, 0, 0));
    btStaticPlaneShape* plane = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
    btMotionState* motion = new btDefaultMotionState(t);
    btRigidBody::btRigidBodyConstructionInfo info(0.0, motion, plane);
    btRigidBody* body = new btRigidBody(info);
	body->setRestitution(btScalar(0.5));
    world->addRigidBody(body);
    bodies.push_back(body);

    addSphere(1.0, 0, 20, 0, 1.0);

}

void btExit()
{
    for (int i = 0; i < bodies.size(); i++)
    {
        world->removeCollisionObject(bodies[i]);
        btMotionState* motionState = bodies[i]->getMotionState();
        btCollisionShape* shape = bodies[i]->getCollisionShape();
        delete bodies[i];
        delete shape;
        delete motionState;
    }
    delete dispatcher;
    delete collisionConfiguration;
    delete solver;
    delete world;
    delete broadphase;
}

