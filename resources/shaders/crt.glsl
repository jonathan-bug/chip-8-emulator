#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform float time;

out vec4 finalColor;

void main() {
    vec2 uv = fragTexCoord - 0.5;
    float radius = dot(uv, uv);
    uv *= (1.0 + radius * 0.2);
    uv += 0.5;

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        float offset = 0.003;
        float r = texture(texture0, uv + vec2(offset, 0.0)).r;
        float g = texture(texture0, uv).g;
        float b = texture(texture0, uv - vec2(offset, 0.0)).b;
        vec3 color = vec3(r, g, b);

        float scanline = sin(uv.y * 800.0) * 0.08;
        color -= scanline;
        color *= vec3(1.0, 0.98, 0.95);

        finalColor = vec4(color, 1.0) * fragColor;
    }
}