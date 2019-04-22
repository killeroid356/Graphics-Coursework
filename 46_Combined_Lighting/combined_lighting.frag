#version 440

// This shader requires spot.frag, part_shadow.frag

// Spot light definition
struct spot_light {
  vec4 light_colour;
  vec3 position;
  vec3 direction;
  float constant;
  float linear;
  float quadratic;
  float power;
};

// A material structure
struct material {
  vec4 emissive;
  vec4 diffuse_reflection;
  vec4 specular_reflection;
  float shininess;
};



// Directional light for the scene
uniform spot_light light;

// Material of the object
uniform material mat;
// Position of the camera
uniform vec3 eye_pos;
// Texture
uniform sampler2D tex;
// Shadow map to sample from
uniform sampler2D shadow_map;


// Incoming position
layout(location = 0) in vec3 position;
// Incoming normal
layout(location = 1) in vec3 normal;
// Incoming texture coordinate
layout(location = 2) in vec2 tex_coord;
// Incoming light space position
layout(location = 3) in vec4 light_space_pos;


// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
  // *********************************
  float shade;
  vec3 proj_coords = light_space_pos.xyz / light_space_pos.w;
  // Use this to calculate texture coordinates for shadow map
  vec2 shadow_tex_coords;
  // This is a bias calculation to convert to texture space
  shadow_tex_coords.x = 0.5 * proj_coords.x + 0.5;
  shadow_tex_coords.y = 0.5 * proj_coords.y + 0.5;
  // Check shadow coord is in range
  if (shadow_tex_coords.x < 0 || shadow_tex_coords.x > 1 || shadow_tex_coords.y < 0 || shadow_tex_coords.y > 1) {
    shade = 1.0;
  }
  float z = 0.5 * proj_coords.z + 0.5;
  // *********************************
  // Now sample the shadow map, return only first component (.x/.r)
	float depth = texture(shadow_map, shadow_tex_coords).x;
  // *********************************
  //Check if depth is in range.  Add a slight epsilon for precision
  if (depth == 0.0) {
    shade = 1.0;
  } else if (depth < z + 0.001) {
    shade = 0.5;
  } else {
    shade = 1.0;
  }

  vec3 view_dir = normalize(eye_pos - position);
  vec4 tex_colour = texture(tex, tex_coord);

  // Calculate direction to the light
	vec3 light_dir = normalize(light.position - position);
	// Calculate distance to light
	float d = distance(light.position, position);
	// Calculate attenuation value :  (constant + (linear * d) + (quadratic * d * d)
	// Calculate spot light intensity :  (max( dot(light_dir, -direction), 0))^power
	// Calculate light colour:  (intensity / attenuation) * light_colour
	vec4 light_colour = light.light_colour * (pow(max(dot(-light.direction, light_dir),0),light.power)/light.constant + light.linear*d + light.quadratic*d*d);
	// *********************************
	// Now use standard phong shading but using calculated light colour and direction
	vec4 diffuse = (mat.diffuse_reflection * light_colour) * max(dot(normal, light_dir), 0.0);
	vec3 half_vector = normalize(light_dir + view_dir);
	vec4 specular = (mat.specular_reflection * light_colour) * pow(max(dot(normal, half_vector), 0.0), mat.shininess);
	
	colour = ((mat.emissive + diffuse) * tex_colour) + specular;
	colour.a = 1.0;

  colour = colour * shade;
  colour.w = 1;
  // *********************************
}