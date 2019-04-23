#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

map<string, mesh> meshes;
std::array<mesh, 2> hierarchymeshes;
effect eff;
effect shadow_eff;
texture tex;
target_camera cam;
free_camera cam2;
double cursor_x = 0.0;
double cursor_y = 0.0;
spot_light light;
shadow_map shadow;
bool enabled;

bool initialise(){
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwGetCursorPos(renderer::get_window(), &cursor_x, &cursor_y);
	enabled = false;
	return true;
}
   

bool load_content() {
  
	shadow = shadow_map(renderer::get_screen_width(), renderer::get_screen_height());
	// Create plane mesh
  meshes["plane"] = mesh(geometry_builder::create_plane());
  hierarchymeshes[0] = mesh(geometry_builder::create_box(vec3(5.0f, 5.0f, 5.0f)));
  hierarchymeshes[1] = mesh(geometry_builder::create_box(vec3(5.0f, 5.0f, 5.0f)));
  meshes["sphere"] = mesh(geometry_builder::create_sphere(20, 20));
  meshes["teapot"] = mesh(geometry("models/teapot.obj"));

  meshes["sphere"].get_transform().scale = vec3(2.5f, 2.5f, 2.5f);
  meshes["sphere"].get_transform().translate(vec3(-25.0f, 9.8f, -25.0f));
  hierarchymeshes[0].get_transform().translate(vec3(0.0f, 2.5f, 0.0f));
  hierarchymeshes[1].get_transform().translate(vec3(-5.0f, 0.0f, 0.0f));
  meshes["teapot"].get_transform().translate(vec3(-25.0f, 0.0f, -25.0f));
  meshes["teapot"].get_transform().scale = vec3(0.1f, 0.1f, 0.1f);

  //all diffuse is red
  hierarchymeshes[0].get_material().set_diffuse(vec4(0.9f, 0.0f, 0.0f, 1.0f));
  hierarchymeshes[1].get_material().set_diffuse(vec4(0.5f, 0.0f, 0.9f, 1.0f));
  meshes["sphere"].get_material().set_diffuse(vec4(0.9f, 0.0f, 0.0f, 1.0f));
  meshes["teapot"].get_material().set_diffuse(vec4(0.9f, 0.0f, 0.0f, 1.0f));
  //except the plane
  meshes["plane"].get_material().set_diffuse(vec4(0.0f, 0.0f, 0.9f, 1.0f));

  //all emmissive is black
  hierarchymeshes[0].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  hierarchymeshes[1].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  meshes["sphere"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  meshes["teapot"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  meshes["plane"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));

  //all specular is white
  hierarchymeshes[0].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  hierarchymeshes[1].get_material().set_specular(vec4(1.0f, 1.0f, 0.5f, 1.0f));
  meshes["sphere"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  meshes["teapot"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  meshes["plane"].get_material().set_specular(vec4(0.0f, 0.0f, 0.0f, 1.0f));

  //all shininess is 25
  hierarchymeshes[0].get_material().set_shininess(25.0f);
  hierarchymeshes[1].get_material().set_shininess(50.0f);
  meshes["sphere"].get_material().set_shininess(25.0f);
  meshes["teapot"].get_material().set_shininess(25.0f);
  meshes["plane"].get_material().set_shininess(25.0f);

  tex = texture("textures/checker.png");
  //settup the lighting
  light.set_position(vec3(30.0f, 20.0f, 0.0f));
  light.set_direction(vec3(-1.0f, -1.0f, -1.0f));
  light.set_constant_attenuation(1.0f);
  light.set_linear_attenuation(1.0f);
  light.set_quadratic_attenuation(1.0f);
  light.set_range(5000.0f);
  light.set_light_colour(vec4(0.9f, 0.9f, 0.9f, 1.0f));
  light.set_power(2.0f);
  

  // Load in shaders
  eff.add_shader("46_Combined_Lighting/combined_lighting.vert", GL_VERTEX_SHADER);
  eff.add_shader("46_Combined_Lighting/combined_lighting.frag", GL_FRAGMENT_SHADER);
  shadow_eff.add_shader("50_Spot_Light/spot.vert", GL_VERTEX_SHADER);
  shadow_eff.add_shader("50_Spot_Light/spot.frag", GL_FRAGMENT_SHADER);

  // Build effect
  eff.build();
  shadow_eff.build();

  // Set camera properties
  cam.set_position(vec3(-50.0f, 10.0f, -50.0f));
  cam.set_target(vec3(0.0f, 0.0f, 0.0f));
  cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
  cam2.set_position(vec3(50.0f, 10.0f, 50.0f));
  cam2.set_target(vec3(0.0f, 0.0f, 0.0f));
  cam2.set_yaw(45.0f);
  cam2.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
  return true;
}

bool update(float delta_time) {
	

	hierarchymeshes[0].get_transform().rotate(vec3(0.0f, 1.0f, 0.0f)*delta_time);
	hierarchymeshes[1].get_transform().rotate(vec3(1.0f, 0.0f, 0.0f)*delta_time);
	if (glfwGetKey(renderer::get_window(), 'Y')) {
		light.rotate(vec3(0.0f,0.05f,0.0f));
	}
	if (glfwGetKey(renderer::get_window(), 'T')) {
		light.rotate(vec3(0.0f, -0.05f, 0.0f));
	}
	//enable fixed cam
	if (glfwGetKey(renderer::get_window(), '1')) {
		enabled = false;
	}
	//enable freecam
	if (glfwGetKey(renderer::get_window(), '2')) {
		enabled = true;
	}
	//if movement is enabled then give control of camera to user
	if (enabled) {
		vec3 transform = vec3(0.0f);
		//free cam implementation
		static double ratio_width = quarter_pi<float>() / static_cast<float>(renderer::get_screen_width());
		static double ratio_height =
			(quarter_pi<float>() *
			(static_cast<float>(renderer::get_screen_height()) / static_cast<float>(renderer::get_screen_width()))) /
			static_cast<float>(renderer::get_screen_height());

		double current_x = 0.0;
		double current_y = 0.0;

		glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
		double delta_x = current_x - cursor_x;
		double delta_y = current_y - cursor_y;
		delta_x = delta_x * ratio_width;
		delta_y = delta_y * ratio_height;

		cam2.rotate(delta_x*3, -delta_y*3);
		//keyboard controls
		if (glfwGetKey(renderer::get_window(), 'W')) {
			transform = transform + vec3(0.0f, 0.0f, 1.0f);
		}
		if (glfwGetKey(renderer::get_window(), 'A')) {
			transform = transform + vec3(-1.0f, 0.0f, 0.0f);
		}
		if (glfwGetKey(renderer::get_window(), 'S')) {
			transform = transform + vec3(0.0f, 0.0f, -1.0f);
		}
		if (glfwGetKey(renderer::get_window(), 'D')) {
			transform = transform + vec3(1.0f, 0.0f, 0.0f);
		}
		cam2.move(transform);
		
		cam2.update(delta_time);
		cursor_x = current_x;
		cursor_y = current_y;
	}
	else {
		cam.update(delta_time);
	}
	shadow.light_position = light.get_position();
	
	shadow.light_dir = light.get_direction();

	if (glfwGetKey(renderer::get_window(), 'P') == GLFW_PRESS) {
		shadow.buffer->save("test.png");
	}

	return true;
}

bool render() {

	// *********************************
 // Set render target to shadow map
	renderer::set_render_target(shadow);
	// Clear depth buffer bit
	glClear(GL_DEPTH_BUFFER_BIT);
	// Set face cull mode to front
	glCullFace(GL_FRONT);
	// *********************************
	   //glEnable(GL_BLEND);
	// We could just use the Camera's projection, 
	// but that has a narrower FoV than the cone of the spot light, so we would get clipping.
	// so we have yo create a new Proj Mat with a field of view of 90.
	mat4 LightProjectionMat = perspective<float>(90.0f, renderer::get_screen_aspect(), 0.1f, 1000.f);

	// Bind shader
	renderer::bind(shadow_eff);
	// Render meshes
	
	for (auto &e : meshes) {
		auto m = e.second;
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		// *********************************
		// View matrix taken from shadow map
		auto V = shadow.get_view();
		// *********************************
		auto MVP = LightProjectionMat * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(shadow_eff.get_uniform_location("MVP"), // Location of uniform
			1,                                      // Number of values - 1 mat4
			GL_FALSE,                               // Transpose the matrix?
			value_ptr(MVP));                        // Pointer to matrix data
// Render mesh
		renderer::render(m);
	}
	
	for (size_t i = 0; i < hierarchymeshes.size(); i++) {
		auto m = hierarchymeshes[i];
		auto M = m.get_transform().get_transform_matrix();
		for (size_t j = i; j > 0; j--) {
			M = hierarchymeshes[j - 1].get_transform().get_transform_matrix() * M;
		}
		// *********************************
		// View matrix taken from shadow map
		auto V = shadow.get_view();
		// *********************************
		auto MVP = LightProjectionMat * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(shadow_eff.get_uniform_location("MVP"), // Location of uniform
			1,                                      // Number of values - 1 mat4
			GL_FALSE,                               // Transpose the matrix?
			value_ptr(MVP));                        // Pointer to matrix data
// Render mesh
		renderer::render(m);
	}
	
	// *********************************
  // Set render target back to the screen
	renderer::set_render_target();
	// Set face cull mode to back
	glCullFace(GL_BACK);
	// *********************************

  // Render meshes
int count = 0;
renderer::bind(eff);

for (auto &e : meshes) {
	auto m = e.second;
	// Create MVP matrix
	auto M = m.get_transform().get_transform_matrix();
	mat4 V;
	mat4 P;
	
	if (enabled) {
		V = cam2.get_view();
		P = cam2.get_projection();
		// Set eye position - Get this from active camera
		glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(cam2.get_position()));
	}
	else{
		V = cam.get_view();
		P = cam.get_projection();
		// Set eye position - Get this from active camera
		glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(cam.get_position()));
	}
	auto MVP = P * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// *********************************
	// Set M matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
	// Set N matrix uniform - remember - 3x3 matrix
	glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));

	// Set lightMVP uniform, using:
	//Model matrix from m
	auto lM = m.get_transform().get_transform_matrix();
	// viewmatrix from the shadow map
	auto lV = shadow.get_view();
	// Multiply together with LightProjectionMat
	auto LMVP = LightProjectionMat * lV * lM;
	// Set uniform
	glUniformMatrix4fv(eff.get_uniform_location("lightMVP"), 1, GL_FALSE, value_ptr(LMVP));

	//bind some stuff and hope it works
	renderer::bind(m.get_material(), "mat");
	renderer::bind(light, "light");
	renderer::bind(tex, 0);
	renderer::bind(shadow.buffer->get_depth(), 1);

	//set tex uniform
	glUniform1i(eff.get_uniform_location("tex"), 0);
	glUniform1i(eff.get_uniform_location("shadow_map"), 1);
    // *********************************
    // Render mesh
    renderer::render(m);
	count = count + 1;
  }
  count = 0;
  for (size_t i = 0; i < hierarchymeshes.size(); i++) {
	  auto m = hierarchymeshes[i];
	  // Create MVP matrix
	  auto M = m.get_transform().get_transform_matrix();
	  mat4 V;
	  mat4 P;
	  for (size_t j = i; j > 0; j--) {
		  M = hierarchymeshes[j - 1].get_transform().get_transform_matrix() * M;
	  }
	  if (enabled) {
		  V = cam2.get_view();
		  P = cam2.get_projection();
		  // Set eye position - Get this from active camera
		  glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(cam2.get_position()));
	  }
	  else {
		  V = cam.get_view();
		  P = cam.get_projection();
		  // Set eye position - Get this from active camera
		  glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(cam.get_position()));
	  }
	  auto MVP = P * V * M;
	  // Set MVP matrix uniform
	  glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	  // *********************************
	  // Set M matrix uniform
	  glUniformMatrix4fv(eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
	  // Set N matrix uniform - remember - 3x3 matrix
	  glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));
	  // Set lightMVP uniform, using:
	//Model matrix from m
	  auto lM = m.get_transform().get_transform_matrix();
   // viewmatrix from the shadow map
	  auto lV = shadow.get_view();
   // Multiply together with LightProjectionMat
	  auto LMVP = LightProjectionMat * lV * lM;
   // Set uniform
	  glUniformMatrix4fv(eff.get_uniform_location("lightMVP"), 1, GL_FALSE, value_ptr(LMVP));




	  //bind some stuff and hope it works
	  renderer::bind(m.get_material(), "mat");
	  renderer::bind(light, "light");
	  renderer::bind(tex, 0);
	  renderer::bind(shadow.buffer->get_depth(), 1);
	  //set tex uniform
	  glUniform1i(eff.get_uniform_location("tex"), 0);
	  glUniform1i(eff.get_uniform_location("shadow_map"), 1);
	  
	  // *********************************
	  // Render mesh
	  renderer::render(m);
	  count = count + 1;
  }
  count = 0;
  return true;
}

void main() {
  // Create application
  app application("46_Combined_Lighting");
  // Set load content, update and render methods
  application.set_load_content(load_content);
  application.set_initialise(initialise);
  application.set_update(update);
  application.set_render(render);
  // Run application
  application.run();
}