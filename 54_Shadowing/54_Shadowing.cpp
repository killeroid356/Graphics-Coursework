#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

map<string, mesh> meshes;
effect main_eff;
effect shadow_eff;
texture tex;
target_camera cam;
free_camera cam2;
double cursor_x = 0.0;
double cursor_y = 0.0;
spot_light spot;
shadow_map shadow;
bool enabled;

bool initialise() {
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwGetCursorPos(renderer::get_window(), &cursor_x, &cursor_y);
	enabled = false;
	return true;
}



bool load_content() {
  // *********************************
  // Create shadow map- use screen size
	shadow = shadow_map(renderer::get_screen_width(), renderer::get_screen_height());
  // Create plane mesh
	meshes["plane"] = mesh(geometry_builder::create_plane());
  // Create "teapot" mesh by loading in models/teapot.obj
	meshes["sphere"] = mesh(geometry_builder::create_sphere(20, 20));
	meshes["sphere"].get_transform().scale = vec3(2.5f, 2.5f, 2.5f);
	meshes["sphere"].get_transform().translate(vec3(0.0f, 13.8f, 0.0f));
	meshes["teapot"] = mesh(geometry("models/teapot.obj"));
  // Translate Teapot(0,4,0)
	meshes["teapot"].get_transform().translate(vec3(0.0f, 4.0f, 0.0f));
  // Scale the teapot - (0.1, 0.1, 0.1)
	meshes["teapot"].get_transform().scale = vec3(0.1f, 0.1f, 0.1f);
  // *********************************

  // Load texture
  tex = texture("textures/checked.gif");

  // ***********************
  // Set materials
  // - all emissive is black
  // - all specular is white
  // - all shininess is 25
  // ***********************
  // White plane
  meshes["plane"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  meshes["plane"].get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  meshes["plane"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  meshes["plane"].get_material().set_shininess(25.0f);
  // Red teapot
  meshes["teapot"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  meshes["teapot"].get_material().set_diffuse(vec4(1.0f, 0.0f, 0.0f, 1.0f));
  meshes["teapot"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  meshes["teapot"].get_material().set_shininess(25.0f);
  //Red sphere
  meshes["sphere"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  meshes["sphere"].get_material().set_diffuse(vec4(1.0f, 0.0f, 0.0f, 1.0f));
  meshes["sphere"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  meshes["sphere"].get_material().set_shininess(25.0f);

  // *******************
  // Set spot properties
  // *******************
  // Pos (20, 30, 0), White
  // Direction (-1, -1, 0) normalized
  // 50 range, 10 power
  spot.set_position(vec3(30.0f, 25.0f, 0.0f));
  spot.set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  spot.set_direction(normalize(-spot.get_position()));
  spot.set_range(500.0f);
  spot.set_power(10.0f);

  // Load in shaders
  main_eff.add_shader("54_Shadowing/shader.vert", GL_VERTEX_SHADER);
  vector<string> frag_shaders{"54_Shadowing/shader.frag", "shaders/part_spot.frag", "shaders/part_shadow.frag"};
  main_eff.add_shader(frag_shaders, GL_FRAGMENT_SHADER);

  shadow_eff.add_shader("50_Spot_Light/spot.vert", GL_VERTEX_SHADER);
  shadow_eff.add_shader("50_Spot_Light/spot.frag", GL_FRAGMENT_SHADER);

  // Build effects
  main_eff.build();
  shadow_eff.build();

  // Set camera properties
  cam.set_position(vec3(0.0f, 50.0f, -75.0f));
  cam.set_target(vec3(0.0f, 0.0f, 0.0f));
  cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
  cam2.set_position(vec3(0.0f, 50.0f, -75.0f));
  cam2.set_target(vec3(0.0f, 0.0f, 0.0f));
  cam2.set_yaw(135.0f);
  cam2.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
  return true;
}

bool update(float delta_time) {
  // Rotate the teapot
  meshes["teapot"].get_transform().rotate(vec3(0.0f, half_pi<float>(), 0.0f) * delta_time);
  meshes["sphere"].get_transform().rotate(vec3(0.0f, half_pi<float>(), 0.0f) * delta_time);

  if (glfwGetKey(renderer::get_window(), '1')) {
    cam.set_position(vec3(0.0f, 50.0f, -75.0f));
	enabled = false;
  }
  if (glfwGetKey(renderer::get_window(), '2')) {
    cam.set_position(spot.get_position());
	enabled = false;
  }
  if (glfwGetKey(renderer::get_window(), '3')) {
    cam.set_position(vec3(-25.0, 50.0, 0.0));
	enabled = false;
  }
  if (glfwGetKey(renderer::get_window(), '4')) {
    cam.set_position(vec3(-50, 2.0, 0));
	enabled = false;
  }


  if (glfwGetKey(renderer::get_window(), '5')) {
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

	  cam2.rotate(delta_x * 3, -delta_y * 3);
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
  

  // *********************************
  // Update the shadow map light_position from the spot light
  shadow.light_position = spot.get_position();

  // do the same for light_dir property

  shadow.light_dir = spot.get_direction();
  // *********************************

  // Press s to save
  if (glfwGetKey(renderer::get_window(), 'P') == GLFW_PRESS)
    shadow.buffer->save("test.png");

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

  // We could just use the Camera's projection, 
  // but that has a narrower FoV than the cone of the spot light, so we would get clipping.
  // so we have yo create a new Proj Mat with a field of view of 90.
  mat4 LightProjectionMat = perspective<float>(90.f, renderer::get_screen_aspect(), 0.1f, 1000.f);

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
  // *********************************
  // Set render target back to the screen
  renderer::set_render_target();
  // Set face cull mode to back
  glCullFace(GL_BACK);
  // *********************************

  // Bind shader
  renderer::bind(main_eff);

  // Render meshes
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
		glUniform3fv(main_eff.get_uniform_location("eye_pos"), 1, value_ptr(cam2.get_position()));
	}
	else {
		V = cam.get_view();
		P = cam.get_projection();
		// Set eye position - Get this from active camera
		glUniform3fv(main_eff.get_uniform_location("eye_pos"), 1, value_ptr(cam.get_position()));
	}
    auto MVP = P * V * M;
    // Set MVP matrix uniform
    glUniformMatrix4fv(main_eff.get_uniform_location("MVP"), // Location of uniform
                       1,                                    // Number of values - 1 mat4
                       GL_FALSE,                             // Transpose the matrix?
                       value_ptr(MVP));                      // Pointer to matrix data
    // Set M matrix uniform
    glUniformMatrix4fv(main_eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
    // Set N matrix uniform
    glUniformMatrix3fv(main_eff.get_uniform_location("N"), 1, GL_FALSE,
                       value_ptr(m.get_transform().get_normal_matrix()));
    // *********************************
    // Set lightMVP uniform, using:
	auto lM = m.get_transform().get_transform_matrix();
	// viewmatrix from the shadow map
	auto lV = shadow.get_view();
	// Multiply together with LightProjectionMat
	auto LMVP = LightProjectionMat * lV * lM;
	// Set uniform
	glUniformMatrix4fv(main_eff.get_uniform_location("lightMVP"), 1, GL_FALSE, value_ptr(LMVP));

	renderer::bind(m.get_material(), "mat");
	renderer::bind(spot, "spot");
	renderer::bind(tex, 0);
	renderer::bind(shadow.buffer->get_depth(), 1);

    // Set eye position
	glUniform1i(main_eff.get_uniform_location("tex"), 0);
    // Set the shadow_map uniform
	glUniform1i(main_eff.get_uniform_location("shadow_map"), 1);
    // Render mesh
	renderer::render(m);
    // *********************************
  }

  return true;
}

void main() {
  // Create application
  app application("54_Shadowing");
  // Set load content, update and render methods
  application.set_load_content(load_content);
  application.set_initialise(initialise);
  application.set_update(update);
  application.set_render(render);
  // Run application
  application.run();
}