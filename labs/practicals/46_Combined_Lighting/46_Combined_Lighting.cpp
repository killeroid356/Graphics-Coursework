#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

map<string, mesh> meshes;
effect eff;
texture tex;
target_camera cam;

bool load_content() {
  // Create plane mesh
  meshes["plane"] = mesh(geometry_builder::create_plane());
  meshes["box"] = mesh(geometry_builder::create_box(vec3(5.0f, 5.0f, 5.0f)));
  meshes["sphere"] = mesh(geometry_builder::create_sphere(20, 20));

  meshes["sphere"].get_transform().scale = vec3(2.5f, 2.5f, 2.5f);
  meshes["sphere"].get_transform().translate(vec3(-25.0f, 10.0f, -25.0f));
  meshes["box"].get_transform().translate(vec3(-10.0f, 2.5f, -30.0f));
  // Load in shaders
  eff.add_shader("46_Combined_Lighting/combined_lighting.vert", GL_VERTEX_SHADER);
  eff.add_shader("46_Combined_Lighting/combined_lighting.frag", GL_FRAGMENT_SHADER);
  // Build effect
  eff.build();

  // Set camera properties
  cam.set_position(vec3(-50.0f, 10.0f, -50.0f));
  cam.set_target(vec3(0.0f, 0.0f, 0.0f));
  cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
  return true;
}

bool update(float delta_time) {
  
  if (glfwGetKey(renderer::get_window(), '1')) {
    cam.set_position(vec3(-50, 10, -50));
  }
  


  cam.update(delta_time);

  return true;
}

bool render() {
  // Render meshes
int count = 0;
for (auto &e : meshes) {
	auto m = e.second;
	// Bind effect
	renderer::bind(eff);
	// Create MVP matrix
	auto M = m.get_transform().get_transform_matrix();
	auto V = cam.get_view();
	auto P = cam.get_projection();
	auto MVP = P * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// *********************************
	// Set M matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
	// Set N matrix uniform - remember - 3x3 matrix
	glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));
	// Set ambient intensity - (0.3, 0.3, 0.3, 1.0)
	glUniform4fv(eff.get_uniform_location("ambient_intensity"), 1, value_ptr(vec4(0.3f, 0.3f, 0.3f, 1.0f)));
	// Set light colour - (1.0, 1.0, 1.0, 1.0)
	glUniform4fv(eff.get_uniform_location("light_colour"), 1, value_ptr(vec4(1.0f, 1.0f, 1.0f, 1.0f)));
	// Set light direction - (1.0, 1.0, -1.0)
	glUniform3fv(eff.get_uniform_location("light_dir"), 1, value_ptr(vec3(0.5f, 1.0f, -0.5f)));
	// Set diffuse reflection - all objects red
	if (count == 1) {
		glUniform4fv(eff.get_uniform_location("diffuse_reflection"), 1, value_ptr(vec4(0.5f, 0.5f, 0.5f, 1.0f)));
	}
	else {
		glUniform4fv(eff.get_uniform_location("diffuse_reflection"), 1, value_ptr(vec4(0.7f, 0.05f, 0.05f, 1.0f)));
	}
    // Set specular reflection - white
	glUniform4fv(eff.get_uniform_location("specular_reflection"), 1, value_ptr(vec4(1.0f, 1.0f, 1.0f, 1.0f)));
    // Set shininess - Use 50.0f
	glUniform1f(eff.get_uniform_location("shininess"), 50.0f);
    // Set eye position - Get this from active camera
	glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(cam.get_position()));
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
  application.set_update(update);
  application.set_render(render);
  // Run application
  application.run();
}