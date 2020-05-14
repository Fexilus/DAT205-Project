#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

uniform int object_id;

float hue2rgb(float p, float q, float t)
{
            if(t < 0) t += 1;
            if(t > 1) t -= 1;
            if(t < 1.0/6.0) return p + (q - p) * 6 * t;
            if(t < 1.0/2.0) return q;
            if(t < 2.0/3.0) return p + (q - p) * (2.0/3.0 - t) * 6;
            return p;
}

vec3 hslToRgb(float h, float s, float l)
{
    float r, g, b;

    if(s == 0){
        r = g = b = l; // achromatic
    }
	else{

        float q;
		if (l < 0.5) q = l * (1 + s);
		else q= l + s - l * s;

        float p = 2 * l - q;
        r = hue2rgb(p, q, h + 1.0/3.0);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1.0/3.0);
    }

    return vec3(r, g, b);
}

vec4 pick_test() 
{
    vec3 id_color = hslToRgb(((object_id * 11) % 255)/255.0, 175.0/255.0, 175.0/255.0);
	return vec4(id_color, 1.0);
}