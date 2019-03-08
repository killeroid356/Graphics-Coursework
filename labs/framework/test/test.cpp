#include "graphics_framework.h"
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
using namespace std;
using namespace graphics_framework;
using namespace glm;

geometry geom;
geometry geom2;
geometry geom3;
geometry geom4;
effect eff;
effect teff;
effect sbeff;
texture tpng;
texture tjpg;
texture tmipped;
target_camera cam;
cubemap cube_map;
shadow_map shadow;
float theta = 0.0f;

bool load_content() {
  // Create triangle data
  // Positions
  geom.set_type(GL_TRIANGLE_STRIP);
  vector<vec3> positions{vec3(1.0f, 0.0f, 1.0f), vec3(1.0f, 0.0f, -1.0f),
                         vec3(-1.0f, 0.0f, 1.0f), vec3(-1.0f, 0.0f, -1.0f)};
  // Texture coordinates
  vector<vec2> tex_coords{vec2(0.0f, 0.0f), vec2(40.0f, 0.0f),
                          vec2(0.0f, 40.0f), vec2(40.0f, 40.0f)};
  // Add to the geometry
  geom.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);

  geom.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);

  geom2 = geometry_builder::create_plane(10, 10);
  geom4 = geometry_builder::create_box();


  // Load in model
  auto src = "models/box.obj";
  geom3 = geometry(src);

  src = "textures/sahara_lf.jpg";
  tpng = texture(src, true, false);
  src = "textures/sahara_lf.jpg";
  tjpg = texture(src, false, false);

  array<string, 6> filenames = {
      "textures/sahara_ft.jpg", "textures/sahara_bk.jpg",
      "textures/sahara_up.jpg", "textures/sahara_dn.jpg",
      "textures/sahara_rt.jpg", "textures/sahara_lf.jpg"};
  cube_map = cubemap(filenames);

  vector<string> mipnames = {"textures/uv_32.png", "textures/uv_16.png",
                             "textures/uv_8.png",  "textures/uv_4.png",
                             "textures/uv_2.png",  "textures/uv_1.png"};

  tmipped = texture(mipnames, false);

  // Load in shaders
  eff.add_shader("shaders/basic.vert", GL_VERTEX_SHADER);
  eff.add_shader("shaders/basic.frag", GL_FRAGMENT_SHADER);
  eff.build();
  teff.add_shader("shaders/basic_textured.vert", GL_VERTEX_SHADER);
  teff.add_shader("shaders/basic_textured.frag", GL_FRAGMENT_SHADER);
  teff.build();
  sbeff.add_shader("shaders/skybox.vert", GL_VERTEX_SHADER);
  sbeff.add_shader("shaders/skybox.frag", GL_FRAGMENT_SHADER);
  sbeff.build();
  // Set camera properties
  cam.set_position(vec3(10.0f, 10.0f, 10.0f));
  cam.set_target(vec3(0.0f, 0.0f, 0.0f));
  auto aspect = static_cast<float>(renderer::get_screen_width()) /
                static_cast<float>(renderer::get_screen_height());
  cam.set_projection(1.0472f, aspect, 2.414f, 1000.0f);
  shadow = shadow_map(renderer::get_screen_width(), renderer::get_screen_height());
  return true;
}

bool update(float delta_time) {
  // Update the angle - half rotation per second
  theta += pi<float>() * delta_time;
  // Update the camera
  cam.set_position(
      rotate(vec3(15.0f, 12.0f, 15.0f), theta * 0.05f, vec3(0, 1.0f, 0)));
  cam.update(delta_time);

  if (glfwGetKey(renderer::get_window(), GLFW_KEY_F)) {
    renderer::set_screen_dimensions(1280, 720);
    auto aspect = static_cast<float>(renderer::get_screen_width()) /
                  static_cast<float>(renderer::get_screen_height());
    cam.set_projection(1.0472f, aspect, 2.414f, 1000.0f);
  }

  if (glfwGetKey(renderer::get_window(), GLFW_KEY_G)) {
    renderer::set_screen_dimensions(800, 600);
    auto aspect = static_cast<float>(renderer::get_screen_width()) /
                  static_cast<float>(renderer::get_screen_height());
    cam.set_projection(quarter_pi<float>(), aspect, 2.414f, 1000.0f);
  }

  if (glfwGetKey(renderer::get_window(), GLFW_KEY_H)) {
    renderer::set_screen_mode(renderer::borderless);
    auto aspect = static_cast<float>(renderer::get_screen_width()) /
                  static_cast<float>(renderer::get_screen_height());
    cam.set_projection(1.0472f, aspect, 2.414f, 1000.0f);
  }
  if (glfwGetKey(renderer::get_window(), GLFW_KEY_J)) {
    renderer::set_screen_mode(renderer::fullscreen);
    auto aspect = static_cast<float>(renderer::get_screen_width()) /
                  static_cast<float>(renderer::get_screen_height());
    cam.set_projection(1.0472f, aspect, 2.414f, 1000.0f);
  }

  // Press s to save
  if (glfwGetKey(renderer::get_window(), 'S') == GLFW_PRESS) {
    shadow.buffer->save("tests.png");
  }
  if (glfwGetKey(renderer::get_window(), 'L') == GLFW_PRESS) {
    shadow.buffer->save("testl.png",false);
  }

  return true;
}


bool do_render() {
  renderer::bind(eff);
  mat4 R;
  R = rotate(mat4(1.0f), theta, vec3(0.0f, 0.0f, 1.0f));
  mat4 M = R;
  auto V = cam.get_view();
  auto P = cam.get_projection();
  auto MVP = P * V * M;
  // Set MVP matrix uniform
  glUniformMatrix4fv(eff.get_uniform_location("MVP"), // Location of uniform
    1,               // Number of values - 1 mat4
    GL_FALSE,        // Transpose the matrix?
    value_ptr(MVP)); // Pointer to matrix data
                     // Render geometry
  renderer::render(geom3);

  renderer::bind(teff);
  renderer::bind(tjpg, 0);
  renderer::bind(tpng, 1);
  renderer::bind(tmipped, 2);
  glUniform1i(teff.get_uniform_location("tex"), 0);
  glUniformMatrix4fv(
    teff.get_uniform_location("MVP"), 1, GL_FALSE,
    value_ptr(P * V * translate(mat4(1.0), vec3(3.0f, -5.0f, -8.0f))));
  renderer::render(geom2);
  glUniform1i(teff.get_uniform_location("tex"), 1);
  glUniformMatrix4fv(
    teff.get_uniform_location("MVP"), 1, GL_FALSE,
    value_ptr(P * V * translate(mat4(1.0), vec3(-8.0f, -5.0f, -8.0f))));
  renderer::render(geom2);

  glUniform1i(teff.get_uniform_location("tex"), 2);
  glUniformMatrix4fv(teff.get_uniform_location("MVP"), 1, GL_FALSE,
    value_ptr(P * V *
      translate(mat4(1.0), vec3(0, -10.0f, 0)) *
      scale(mat4(1.0), vec3(20.0f))));
  renderer::render(geom);

  glDepthMask(GL_FALSE);
  glDisable(GL_CULL_FACE);

  renderer::bind(sbeff);
  MVP = P * V * scale(mat4(1.0), vec3(100.0f));
  glUniformMatrix4fv(sbeff.get_uniform_location("MVP"), 1, GL_FALSE,
    value_ptr(MVP));
  renderer::bind(cube_map, 0);
  glUniform1i(sbeff.get_uniform_location("cubemap"), 0);
  renderer::render(geom4);

  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  return true;
}

bool render() {
  renderer::set_render_target(shadow);
  // Clear depth buffer bit
  glClear(GL_DEPTH_BUFFER_BIT);
  // Set face cull mode to front
  glCullFace(GL_FRONT);

  do_render();

  // Set render target back to the screen
  renderer::set_render_target();
  // Set face cull mode to back
  glCullFace(GL_BACK);

  do_render();
  return true;
}

int main() {
  // Create application
  app application("Framework test", renderer::windowed);
  // Set load content, update and render methods
  application.set_load_content(load_content);
  application.set_update(update);
  application.set_render(render);
  // Run application
  application.run();
  return 0;
}
