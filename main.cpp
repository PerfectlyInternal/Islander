#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <Windows.h>
#include <vector>
#include <ctime>
#include <math.h>
#include <numeric>
#include <thread>
#include <mutex>

#define map_size 1536

#include <internal/shader_loader.h>
#include <internal/terrain_generation.h>
#include <internal/loading_screen.h>
#include <internal/model.h>
#include <internal/texture_loader.h>

#define WINDOW_WIDTH 2048
#define WINDOW_HEIGHT 1024

typedef enum GAME_STATES
{
	MAIN_MENU,
	GENERATING_TERRAIN,
	IN_GAME,
	PAUSED
};

GLFWwindow* window;

bool init();
void quit();

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) // this needs to be replaced with something cross-platform, but Visual Studio won't complile anything without this
{
	#ifdef _DEBUG
		AllocConsole();
		FILE *dummy;
		freopen_s(&dummy, "CONOUT$", "w", stdout);
		freopen_s(&dummy, "CONOUT$", "w", stderr);
	#endif

	if (!init())
	{
		return 1;
	}

	// two different shader programs
	// one for solid colors, used to render the game world
	// another for textures, mostly text and GUIs
	GLuint solid_color_program_id = load_shaders("shaders/solid_color_vertex_shader.glsl", "shaders/solid_color_fragment_shader.glsl");
	GLuint texture_program_id = load_shaders("shaders/texture_vertex_shader.glsl", "shaders/texture_fragment_shader.glsl");

	GLuint main_menu = load_bmp("textures/test.bmp");
	GLuint texture_sampler_id = glGetUniformLocation(texture_program_id, "texture_sampler");

	GLuint vertex_array_id;
	glGenVertexArrays(1, &vertex_array_id);
	glBindVertexArray(vertex_array_id);
	
	std::vector<glm::vec3> vertices;
	std::vector<std::vector<glm::vec3>> map;
	std::vector<glm::vec3> colors;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;

	// buffers for position and color
	GLuint vertex_buffer;
	GLuint color_buffer;
	GLuint normal_buffer;
	GLuint uv_buffer;

	glGenBuffers(1, &vertex_buffer);
	glGenBuffers(1, &color_buffer);
	glGenBuffers(1, &normal_buffer);
	glGenBuffers(1, &uv_buffer);

	// movement variables
	glm::vec3 position = glm::vec3(0, 0, 0);
	double h_angle = 3.14; // radians
	double  v_angle = 0.0;
	const float initial_fov = 90.0f;

	const float speed = 100.0f;
	const float mouse_speed = 0.5f;

	// matricies
	glm::mat4 projection_matrix;
	glm::mat4 view_matrix;
	glm::mat4 model_matrix;
	glm::mat4 final_matrix;

	// handle for the matrices in the shaders
	GLuint matrix_id = glGetUniformLocation(solid_color_program_id, "matrix");
	GLuint view_id = glGetUniformLocation(solid_color_program_id, "view");
	GLuint model_id = glGetUniformLocation(solid_color_program_id, "model");

	GLuint texture_matrix_id = glGetUniformLocation(texture_program_id, "matrix");

	// ambient lighting
	glm::vec3 ambient_light_color = glm::vec3(0.2f, 0.2f, 0.2f);
	GLuint ambient_light_id = glGetUniformLocation(solid_color_program_id, "ambient_light_color");

	// sun variables
	float sun_angle = 45;
	const float sun_speed = 0.1;
	float sun_brightness = sin(glm::radians(sun_angle));
	glm::vec3 directional_light_color = glm::vec3(0.9f * sun_brightness/2.5, 0.9f * sun_brightness, 0.9f * sun_brightness);
	glm::vec3 directional_light_direction = glm::vec3(0.0f, 0.0f, 0.0f);
	GLuint directional_light_color_id = glGetUniformLocation(solid_color_program_id, "directional_light_color");
	GLuint directional_light_direction_id = glGetUniformLocation(solid_color_program_id, "directional_light_direction");
	glClearColor(sun_brightness * 0.529, sun_brightness * 0.808, sun_brightness * 0.922, 1);

	// fog variables
	float fog_start = 256;
	float fog_end = 512;
	glm::vec3 base_fog_color(0.529, 0.808, 0.922);
	glm::vec3 fog_color = sun_brightness * base_fog_color;

	// uniform variables to pass to shaders
	GLuint camera_pos_id = glGetUniformLocation(solid_color_program_id, "camera_pos");
	GLuint fog_start_id = glGetUniformLocation(solid_color_program_id, "fog_start");
	GLuint fog_end_id = glGetUniformLocation(solid_color_program_id, "fog_end");
	GLuint fog_color_id = glGetUniformLocation(solid_color_program_id, "fog_color");

	// delta_time calculation variables
	double current_time = glfwGetTime();
	double last_time = current_time;
	float delta_time;

	// physics variables
	const float gravity = 5.0f;
	const float max_fall_speed = 5.0f;
	float y_speed = 0;

	GAME_STATES game_state = MAIN_MENU;

	// main loop 
	while (!glfwWindowShouldClose(window))
	{	
		switch (game_state)
		{
		/*
		Player is currently looking at the main menu.
		This will later handle things like starting world generation and loading from file
		Right now it automatically starts world gen.
		*/
		case MAIN_MENU:
			glfwPollEvents();
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			{
				position = glm::vec3(0, map_size, 0);

				glEnable(GL_CULL_FACE);
				glDisable(GL_TEXTURE_2D);

				game_state = GENERATING_TERRAIN;
			}

			glDisable(GL_CULL_FACE);
			glEnable(GL_TEXTURE_2D);

			projection_matrix = glm::perspective(glm::radians(100.0f), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1024.0f);
			view_matrix = glm::lookAt(glm::vec3(1, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
			model_matrix = glm::mat4(1.0f);
			final_matrix = projection_matrix * view_matrix * model_matrix;

			vertices.resize(0);
			vertices.push_back(glm::vec3(0, -1, -1));
			vertices.push_back(glm::vec3(0, -1, 1));
			vertices.push_back(glm::vec3(0, 1, -1));

			vertices.push_back(glm::vec3(0, 1, 1));
			vertices.push_back(glm::vec3(0, 1, -1));
			vertices.push_back(glm::vec3(0, -1, 1));

			uvs.resize(0);
			uvs.push_back(glm::vec2(1, 0));
			uvs.push_back(glm::vec2(0, 0));
			uvs.push_back(glm::vec2(1, 1));

			uvs.push_back(glm::vec2(0, 1));
			uvs.push_back(glm::vec2(1, 1));
			uvs.push_back(glm::vec2(0, 0));

			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
			glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_DYNAMIC_DRAW);

			glClearColor(0.75, 0.75, 0.75, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(texture_program_id);

			glUniformMatrix4fv(texture_matrix_id, 1, GL_FALSE, &final_matrix[0][0]);

			glBindTexture(GL_TEXTURE_2D, main_menu);
			glActiveTexture(GL_TEXTURE0);
			glUniform1i(texture_sampler_id, 0);

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);

			glfwSwapBuffers(window);
			break;

		/*
		Terrain is being generated
		Unlike the rest of the cases, where the program constantly loops through them, this section only runs once at a time
		All updates happen inside the loading_screen() function
		*/
		case GENERATING_TERRAIN:
			{
				int terrain_completion = 0;
				std::thread terrain_thread = std::thread(generate_terrain, map_size, 0, 0.25, std::ref(map), std::ref(colors), std::ref(normals), std::ref(terrain_completion));
				loading_screen(window, terrain_completion, solid_color_program_id, glm::vec3(0.75, 0.75, 0.75), glm::vec3(0, 1, 0), glm::vec3(0.25, 0.25, 0.25));
				terrain_thread.join();
			}
			vertices.resize(0);
			for (int x = 0; x < map_size - 1; x++)
			{
				for (int z = 0; z < map_size - 1; z++)
				{
					vertices.push_back(map[x + 1][z]);
					vertices.push_back(map[x][z]);
					vertices.push_back(map[x + 1][z + 1]);

					vertices.push_back(map[x][z]);
					vertices.push_back(map[x][z + 1]);
					vertices.push_back(map[x + 1][z + 1]);
				}
			}

			for (unsigned int i = map_size; i < map.size(); i++)
				for (std::vector<glm::vec3>::iterator j = map[i].begin(); j != map[i].end(); j++)
					vertices.push_back(*j);

			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
			glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), &colors[0], GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
			glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_DYNAMIC_DRAW);

			game_state = IN_GAME;
			break;

		/*
		Player is currently in game
		This section handles all in-game events, such as input and interaction
		*/
		case IN_GAME:
			// calculate deltatime
			current_time = glfwGetTime();
			delta_time = current_time - last_time;
			last_time = current_time;

			// handle mouse movement
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

			h_angle += mouse_speed * delta_time * float(WINDOW_WIDTH / 2 - xpos);
			v_angle += mouse_speed * delta_time * float(WINDOW_HEIGHT / 2 - ypos);

			if (glm::degrees(v_angle) < -90) v_angle = glm::radians(-90.0f);
			if (glm::degrees(v_angle) > 90) v_angle = glm::radians(90.0f);

			// direction player is pointing in
			glm::vec3 direction(cos(v_angle) * sin(h_angle), sin(v_angle), cos(v_angle) * cos(h_angle));

			// forward movement 
			glm::vec3 forward(sin(h_angle), 0, cos(h_angle));

			// to the right of the player
			glm::vec3 right = glm::vec3(sin(h_angle - 3.14f / 2.0f), 0, cos(h_angle - 3.14f / 2.0f));

			glm::vec3 up = glm::cross(right, direction);

			sun_angle += delta_time * sun_speed;
			sun_brightness = sin(glm::radians(sun_angle));
			directional_light_direction = glm::vec3(cos(glm::radians(sun_angle)), sin(glm::radians(sun_angle)), 0);
			directional_light_color = glm::vec3(0.9f * sun_brightness, 0.9f * sun_brightness, 0.9f * sun_brightness);

			// handle keyboard input
			glfwPollEvents();
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) position += forward * delta_time * speed; // move forward
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) position -= forward * delta_time * speed; // move backward
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) position -= right * delta_time * speed;   // move left
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) position += right * delta_time * speed;   // move right

			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) game_state = PAUSED; // pause the game

			// gravity and physics
			float ground_pos;
			if (position.x > -map_size / 2 && position.x + 1 < map_size / 2 && position.z > -map_size / 2 && position.z + 1 < map_size / 2)
				ground_pos = (float)bilinear_interpolation(
					map[(int)position.x + map_size / 2][(int)position.z + map_size / 2].y,
					map[(int)position.x + map_size / 2][(int)position.z + 1 + map_size / 2].y,
					map[(int)position.x + 1 + map_size / 2][(int)position.z + map_size / 2].y,
					map[(int)position.x + 1 + map_size / 2][(int)position.z + 1 + map_size / 2].y,
					(int)position.x + map_size / 2,
					(int)position.x + 1 + map_size / 2,
					(int)position.z + map_size / 2,
					(int)position.z + 1 + map_size / 2,
					position.x + map_size / 2,
					position.z + map_size / 2
				);
			else
				ground_pos = 0;

			position.y += y_speed;
			if (abs(position.y - ground_pos - 5) < gravity * delta_time)
				if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
					y_speed = 2;
				else
					y_speed = 0;
			else if (position.y < ground_pos + 5)
			{
				position.y = ground_pos + 5;
				y_speed = 0;
			}
			else
			{
				y_speed -= gravity * delta_time;
				printf("%f, %f\n", position.x, position.z);
			}

			projection_matrix = glm::perspective(glm::radians(initial_fov), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1024.0f);
			view_matrix = glm::lookAt(position, position + direction, up);
			model_matrix = glm::mat4(1.0f);
			final_matrix = projection_matrix * view_matrix * model_matrix;

			fog_color = sun_brightness * base_fog_color;
			glClearColor(fog_color.r, fog_color.g, fog_color.b, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(solid_color_program_id);

			// send stuff to shaders
			glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &final_matrix[0][0]);
			glUniformMatrix4fv(view_id, 1, GL_FALSE, &view_matrix[0][0]);
			glUniformMatrix4fv(model_id, 1, GL_FALSE, &model_matrix[0][0]);

			glUniform3fv(ambient_light_id, 1, &ambient_light_color[0]);
			glUniform3fv(directional_light_color_id, 1, &directional_light_color[0]);
			glUniform3fv(directional_light_direction_id, 1, &directional_light_direction[0]);

			glUniform3fv(camera_pos_id, 1, &position[0]);
			glUniform3fv(fog_color_id, 1, &fog_color[0]);
			glUniform1f(fog_start_id, fog_start);
			glUniform1f(fog_end_id, fog_end);

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glDrawArrays(GL_TRIANGLES, 0, vertices.size());
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);

			glfwSwapBuffers(window);
			break;

		case PAUSED:
			glfwSetWindowShouldClose(window, true);
			break;

		// This shouldn't happen
		default:
			break;
		}
	}

	quit();
	return 0;
}	

// error callback function, prints out the error message to stderr
void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

bool init()
{
	// make GLFW call error_callback() whenever ther's an error
	glfwSetErrorCallback(error_callback);

	// attemt to initialize GLFW
	if (glfwInit())
	{
		// set the OpenGL version to 4.6
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

		glfwWindowHint(GLFW_SAMPLES, 4);

		// attempt to create the window
		window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Islander", NULL, NULL);
		if (window != NULL)
		{
			glfwMakeContextCurrent(window);

			// attemt to load advanced OpenGL functions using GLAD
			if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			{
				// enable Vsync
				glfwSwapInterval(1);

				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LESS);

				glEnable(GL_CULL_FACE);
				glFrontFace(GL_CCW);

				glEnable(GL_MULTISAMPLE);

				return true;
			}
		}
	}
	return false;
}

void quit()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}